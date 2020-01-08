#include "WebsocketServer.h"
#include "muduo/base/Logging.h"
#include "openssl/sha.h"
#include "muduo/net/http/HttpContext.h"
#include "Base64.h"

muduo::net::WebsocketServer::WebsocketServer(EventLoop* loop, const InetAddress& listenAddr) :
	tcpServer_(loop, listenAddr, "WebsocketServer"),
	websocketConnected_(false)
{
	tcpServer_.setConnectionCallback(std::bind(&WebsocketServer::onConnection, this, std::placeholders::_1));
	tcpServer_.setMessageCallback(std::bind(&WebsocketServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	setResponceFunc(std::bind(&WebsocketServer::echo, this, std::placeholders::_1, std::placeholders::_2));
	
}

void muduo::net::WebsocketServer::start()
{
	tcpServer_.start();
}

void muduo::net::WebsocketServer::onConnection(const TcpConnectionPtr& conn)
{
	if (!conn->connected())
	{
		tcpConnection_ .reset();
		websocketConnected_ = false;
	}
	//else if (conn->disconnected())
	//{
	//	tcpConnection_.reset();
	//}
}

void muduo::net::WebsocketServer::onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime)
{
	if (connected()) //已经连接成功
	{
		LOG_INFO << "go there";
		WebSocketContent content;
		if (!parseWebSocketContent(buf, content))
		{
			LOG_ERROR << "parseWebSocketContent error";
			return;
		}
		if (content.getOpcode() == WebSocketContent::OpcodeType::kDisconnect)
		{
			LOG_DEBUG << "the websocket disconnect";
			tcpConnection_->shutdown();
			tcpConnection_.reset();
			websocketConnected_ = false;
			return;
		}
		std::string payloadData = content.getPayloadData();
		responceFunc_(conn, payloadData);
	}
	else //未连接，应当进行连接
	{
		LOG_DEBUG << buf->peek();
		HttpRequest req;
		if (!praseHttpRequest(buf, req))
		{
			LOG_ERROR << "praseHttpRequest error";
			return;
		}
		HttpResponse rep(false);
		if (!buildHttpResponce(req, rep))
		{
			LOG_ERROR << "buildHttpResponce error";
			return;
		}
		if (!sendHttpResponce(conn, rep))
		{
			LOG_ERROR << "sendHttpResponce error";
			return;
		}
		websocketConnected_ = true; //连接成功
		tcpConnection_ = conn;
	}
}

bool muduo::net::WebsocketServer::praseHttpRequest(Buffer* buf, HttpRequest& req)
{
	bool ok = true;
	ok = parseRequestLine(buf, req);
	if (!ok)
	{
		LOG_ERROR << "praseHttpRequest error";
		return ok;
	}
	else
	{
		const char* crlf = buf->findCRLF();
		buf->retrieveUntil(crlf + 2);
		ok = parseRequestHeader(buf,req);
		if (!ok)
		{
			LOG_ERROR << "praseHttpRequest error";
			return ok;
		}
		else
		{
			ok = parseRequestBody(buf,req);
			if (!ok)
			{
				LOG_ERROR << "parseRequestBody error";
				return ok;
			}
		}
	}
	return ok;
}

bool muduo::net::WebsocketServer::parseRequestLine(Buffer* buf, HttpRequest& req)
{
	bool ok = true;
	const char* crlf = buf->findCRLF(); //find request line
	if (!crlf)
	{
		ok = false;
		LOG_ERROR << "can not find request line";
		return ok;
	}
	else
	{
		const char* start = buf->peek();
		const char* end = crlf;
		const char* space = std::find(start, end, ' '); // find method
		if (space == end || !req.setMethod(start, space))
		{
			ok = false;
			LOG_ERROR << "method set error";
			return ok;
		}
		else
		{
			start = space + 1;
			space = std::find(start, end, ' ');
			if (space == end)
			{
				ok = false;
				LOG_ERROR << "can not find URL";
				return ok;
			}
			else
			{
				const char* question = std::find(start, space, '?');
				if (question != space) //存在参数
				{
					req.setPath(start, question);
					req.setQuery(question, space);
				}
				else
				{
					req.setPath(start, space);
				}
			}
			start = space + 1;
			if (std::string(start, end) != std::string("HTTP/1.1")) //目前只支持HTTP/1.1
			{
				ok = false;
				LOG_ERROR << "unsupport HTTP version";
				return ok;
			}
			else
				req.setVersion(HttpRequest::kHttp11);
		}
	}

	return ok;
}

bool muduo::net::WebsocketServer::parseRequestHeader(Buffer* buf, HttpRequest& req)
{
	bool ok = true;
	const char* crlf;
	while ((crlf = buf->findCRLF()))
	{
		const char* colon = std::find(buf->peek(), crlf, ':');
		if (colon == crlf)
		{
			break;
		}
		else
		{
			req.addHeader(buf->peek(), colon,crlf);
			buf->retrieveUntil(crlf + 2);
		}
	}
	buf->retrieveUntil(crlf + 2);
	return ok;
}

bool muduo::net::WebsocketServer::parseRequestBody(Buffer* buf, HttpRequest& req)
{
	bool ok = true;
	auto length = req.getHeader("Content-Length");
	if (length.empty())
	{
		LOG_INFO << "the request not have body";
	}
	else
	{
		req.setBody(buf->peek(), buf->peek() + std::stoi(length));
	}
	return ok;
}

bool muduo::net::WebsocketServer::buildHttpResponce(const HttpRequest& req, HttpResponse& rep)
{
	bool ok = true;
	auto connection = req.getHeader("Connection");
	auto upgrade = req.getHeader("Upgrade");
	auto key = req.getHeader("Sec-WebSocket-Key");
	auto version = req.getHeader("Sec-WebSocket-Version");
	if (connection != "Upgrade" || upgrade != "websocket" || key.empty() || version.empty())
	{
		LOG_ERROR << "bad request";
		rep.setStatusCode(HttpResponse::HttpStatusCode::k400BadRequest);
		rep.setStatusMessage("Bad Request");
		rep.setCloseConnection(true);
		return ok;
	}

	rep.setStatusCode(HttpResponse::HttpStatusCode::k101SwitchingProtocols);
	rep.setStatusMessage("Switching Protocols");
	rep.addHeader("Connection", connection);
	rep.addHeader("Upgrade", upgrade);
	auto sha1 = SHA1(key + std::string("258EAFA5-E914-47DA-95CA-C5AB0DC85B11"));
	//auto sha1 = SHA1("w4v7O6xFTi36lq3RNcgctw==" + std::string("258EAFA5-E914-47DA-95CA-C5AB0DC85B11"));
	std::string base64_sha1;
	base64_encode(sha1.c_str(), sha1.size(), base64_sha1);
	rep.addHeader("Sec-WebSocket-Accept", base64_sha1);
	rep.addHeader("Sec-WebSocket-Version", version);
	return ok;
}

std::string muduo::net::WebsocketServer::SHA1(const std::string& src)
{
	
	unsigned char obuf[20];
	::SHA1((const unsigned char*)src.c_str(), src.size(), obuf);
	return std::string((char*)obuf);
}

bool muduo::net::WebsocketServer::sendHttpResponce(const TcpConnectionPtr& conn, const HttpResponse& rep)
{
	bool ok = true;
	Buffer buf;
	rep.appendToBuffer(&buf);
	LOG_DEBUG << buf.peek();
	conn->send(&buf);
	if (rep.closeConnection())
		conn->shutdown();
	return ok;
}

bool muduo::net::WebsocketServer::parseWebSocketContent(Buffer* buf, WebSocketContent& content)
{
	bool ok = true;
	//第一个字节
	const char* msg = buf->peek();
	//int pos = 0;
	bool fin = (*msg >> 7);
	content.setFin(fin);
	int opcode = (*msg & 0x0f);
	content.setOpcode((WebSocketContent::OpcodeType)opcode);
	buf->retrieveInt8();

	//第二个字节
	//pos++;
	msg = buf->peek();
	bool mask = (*msg >> 7);
	content.setMask(mask);
	uint8_t payloadLength = *msg & 0x7f;
	//pos++;
	buf->retrieveInt8();
	if (payloadLength == 126)
	{
		uint16_t length;
		msg = buf->peek();
		memcpy(&length, msg, 2);
		length = muduo::net::sockets::networkToHost16(length);
		content.setPayloadLength(length);
		//pos += 2;
		buf->retrieveInt16();
	}
	else if (payloadLength == 127)
	{
		uint64_t length;
		msg = buf->peek();
		memcpy(&length, msg, 8);
		length = muduo::net::sockets::networkToHost64(length);
		content.setPayloadLength(length);
		buf->retrieveInt64();
	}
	else
	{
		content.setPayloadLength(payloadLength);
	}

	uint8_t maskKey[4];
	for (int i = 0; i < 4; i++)
	{
		msg = buf->peek();
		maskKey[i] = *msg;
		buf->retrieveInt8();
	}
	content.setMaskKey(maskKey);

	
	uint64_t length = content.getPayloadLength();
	std::string data = buf->retrieveAsString(length);
	
	//uint8_t* data = new uint8_t[length];
	//memcpy(data, &msg[pos], length);
	//std::string data(&msg[pos], length);
	if (content.getMask())
	{
		const uint8_t* maskKey = content.getMaskKey();
		for (int i = 0; i < length; i++)
		{
			data[i] = data[i] ^ maskKey[i % 4];
		}
	}
	//std::string str((char *)data, length);
	LOG_DEBUG << data;
	content.setPayloadData(data);
	//buf->retrieveUntil(&msg[pos + length]);
	return ok;
}

void muduo::net::WebsocketServer::echo(const TcpConnectionPtr& conn, std::string& payloadData)
{
	send(payloadData);
}

void muduo::net::WebsocketServer::send(const std::string& send_str)
{
	if (websocketConnected_)
	{
		//static const uint8_t masking_key[4] = { 0x12,0x34,0x56,0x78 };
		int payload_size = send_str.size();
		int buf_length = 2 + (payload_size >= 126 ? 2 : 0) + (payload_size >= 65536 ? 6 : 0)  + payload_size;
		uint8_t* buf = new uint8_t[buf_length];
		uint8_t* begin;
		//OpcodeType code = OpcodeType::kText;
		buf[0] = 0x81;
		if (payload_size < 126)
		{
			buf[1] = (payload_size & 0xff) | 0x00;
			//buf[2] = masking_key[0];
			//buf[3] = masking_key[1];
			//buf[4] = masking_key[2];
			//buf[5] = masking_key[3];
			begin = buf + 2;
		}
		else if (payload_size < 65536)
		{
			buf[1] = 126 | 0x00;
			//buf[2] = (payload_size >> 8)& 0xff;
			//buf[3] = (payload_size >> 0) & 0xff;
			int16_t be16 = muduo::net::sockets::hostToNetwork16(payload_size);
			memcpy(&buf[2], &be16, sizeof(be16));
			//buf[4] = masking_key[0];
			//buf[5] = masking_key[1];
			//buf[6] = masking_key[2];
			//buf[7] = masking_key[3];
			begin = buf + 4;
		}
		else
		{
			buf[1] = 127 | 0x00;
			//buf[2] = (payload_size >> 56) & 0xff;
			//buf[3] = (payload_size >> 48) & 0xff;
			//buf[4] = (payload_size >> 40) & 0xff;
			//buf[5] = (payload_size >> 32) & 0xff;
			//buf[6] = (payload_size >> 24) & 0xff;
			//buf[7] = (payload_size >> 16) & 0xff;
			//buf[8] = (payload_size >> 8) & 0xff;
			//buf[9] = (payload_size >> 0) & 0xff;
			int64_t be64 = muduo::net::sockets::hostToNetwork64(payload_size);
			memcpy(&buf[2], &be64, sizeof(be64));
			//buf[10] = masking_key[0];
			//buf[11] = masking_key[1];
			//buf[12] = masking_key[2];
			//buf[13] = masking_key[3];
			begin = buf + 10;
		}
		memcpy((void*)begin, send_str.c_str(), payload_size);
		/*for (int i = 0; i < payload_size; i++)
		{
			*(begin + i) ^= masking_key[i & 0x03];
		}*/
		tcpConnection_->send((void*)buf, buf_length);
	}
}