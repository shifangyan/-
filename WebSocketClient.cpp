#include "WebSocketClient.h"
#include "HttpRequest.h"
#include "muduo/base/Logging.h"

muduo::net::WebSocketClient::WebSocketClient(EventLoop* loop, const InetAddress& addr, std::string path) :
	loop_(loop),
	addr_(addr),
	path_(path),
	connected_(false)
{
}

void muduo::net::WebSocketClient::connect()
{
	tcpClient_.reset(new TcpClient(loop_, addr_, "webSocketClient"));
	tcpClient_->setConnectionCallback(std::bind(&WebSocketClient::onConnection, this, std::placeholders::_1));
	tcpClient_->setMessageCallback(std::bind(&WebSocketClient::onResponce, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	tcpClient_->connect();
}

void muduo::net::WebSocketClient::onConnection(const muduo::net::TcpConnectionPtr& conn)
{
	if (conn->connected())
	{
		conn_ = conn;
		HttpRequest request;
		request.setHttpMethod(HttpRequest::HttpMethod::kGet);
		request.setPath(path_);
		request.setHttpVersion(HttpRequest::HttpVersion::kHttp1_1);
		request.addHeader("Connection", "Upgrade");
		request.addHeader("Upgrade", "websocket");
		request.addHeader("Host", addr_.toIpPort());
		request.addHeader("Origin", "http://test.com");
		request.addHeader("Sec-WebSocket-Key", "lr0tgfvYt4sSWaoQ/HkHfg==");
		request.addHeader("Sec-WebSocket-Version", "13");
		//请求行
		std::string send_str;
		send_str += request.getHttpMethod();
		send_str += " ";
		send_str += request.getPath();
		send_str += " ";
		send_str += request.getHttpVersion();
		send_str += "\r\n";

		//请求头
		auto& headers = request.getHeaders();
		for (auto item : headers)
		{
			send_str += item.first;
			send_str += ": ";
			send_str += item.second;
			send_str += "\r\n";
		}
		send_str += "\r\n";
		//主体
		send_str += request.getBody();
		LOG_INFO << send_str;
		conn_->send(send_str);
	}
}

void muduo::net::WebSocketClient::onResponce(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time)
{
	LOG_INFO << buf->peek();
	const char* crlf = buf->findCRLF(); //找回应行
	if (crlf)
	{
		const char* start = buf->peek();
		const char* end = crlf;
		const char* space = std::find(start, end, ' ');
		if (space != end)
		{
			std::string version(start, space);
			if (version == std::string("HTTP/1.1"))
			{
				start = space + 1;
				space = std::find(start, end, ' ');
				if (space != end)
				{
					int state_code = std::stoi(std::string(start, space));
					if (state_code == 101)
					{
						LOG_INFO << state_code;
					//	std::string send_str(10, 'c');
					//	send(send_str);
						LOG_INFO << "connect success";
						connected();
					}
					else
					{
						LOG_ERROR << "unsupport state code: " << state_code;
					}
				}
				else
				{
					LOG_ERROR << "can not find space in responce line";
					return;
				}

			}
			else
			{
				LOG_ERROR << "unsupport HTTP version: " << version;
			}
		}
		else
		{
			LOG_ERROR << "can not find space in responce line";
			return;
		}
	}
	else
	{
		LOG_ERROR << "can not find responce line";
		return;
	}
}

void muduo::net::WebSocketClient::send(const std::string& send_str)
{
	if (getConnected())
	{
		static const uint8_t masking_key[4] = { 0x12,0x34,0x56,0x78 };
		int payload_size = send_str.size();
		int buf_length = 2 + (payload_size >= 126 ? 2 : 0) + (payload_size >= 65536 ? 6 : 0) + 4 + payload_size;
		uint8_t* buf = new uint8_t[buf_length];
		uint8_t* begin;
		//OpcodeType code = OpcodeType::kText;
		buf[0] = 0x81;
		if (payload_size < 126)
		{
			buf[1] = (payload_size & 0xff) | 0x80;
			buf[2] = masking_key[0];
			buf[3] = masking_key[1];
			buf[4] = masking_key[2];
			buf[5] = masking_key[3];
			begin = buf + 6;
		}
		else if (payload_size < 65536)
		{
			buf[1] = 126 | 0x80;
			//buf[2] = (payload_size >> 8)& 0xff;
			//buf[3] = (payload_size >> 0) & 0xff;
			int16_t be16 = muduo::net::sockets::hostToNetwork16(payload_size);
			memcpy(&buf[2], &be16, sizeof(be16));
			buf[4] = masking_key[0];
			buf[5] = masking_key[1];
			buf[6] = masking_key[2];
			buf[7] = masking_key[3];
			begin = buf + 8;
		}
		else
		{
			buf[1] = 127 | 0x80;
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
			buf[10] = masking_key[0];
			buf[11] = masking_key[1];
			buf[12] = masking_key[2];
			buf[13] = masking_key[3];
			begin = buf + 14;
		}
		memcpy((void*)begin, send_str.c_str(), payload_size);
		for (int i = 0; i < payload_size; i++)
		{
			*(begin + i) ^= masking_key[i & 0x03];
		}
		conn_->send((void*)buf, buf_length);
	}
}
//void muduo::net::WebSocketClient::onConnection(const muduo::net::TcpConnectionPtr& conn)
//{
//	if (conn->connected())
//	{
//		upgradeProtocol(conn);
//	}
//}

//void muduo::net::WebSocketClient::upgradeProtocol(const muduo::net::TcpConnectionPtr& conn)
//{
//	HttpRequest request;
//	request.setHttpMethod(HttpRequest::HttpMethod::kGet);
//	request.setPath("/");
//	request.setHttpVersion(HttpRequest::HttpVersion::kHttp1_1);
//	request.addHeader("Connection", "Upgrade");
//	request.addHeader("Upgrade", "websocket");
//	request.addHeader("Host", addr_.toIpPort());
//	request.addHeader("Sec-WebSocket-Key", "lr0tgfvYt4sSWaoQ/HkHfg==");
//	request.addHeader("Sec-WebSocket-Version", "13");
//
//	conn->send();
//}
