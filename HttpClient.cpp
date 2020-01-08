#include "HttpClient.h"

muduo::net::HttpClient::HttpClient(EventLoop* loop, const InetAddress& addr):loop_(loop),addr_(addr)
{
}

//void muduo::net::HttpClient::setRequest(const ::HttpRequest& req)
//{
//	request_ = req;
//}

void muduo::net::HttpClient::sendRequest(const ::HttpRequest& req)
{
	request_ = req;
	tcpClient_.reset(new TcpClient(loop_, addr_, "http_request"));
	tcpClient_->setConnectionCallback(std::bind(&HttpClient::OnConnection,this,std::placeholders::_1));
	tcpClient_->setMessageCallback(std::bind(&HttpClient::OnResponce, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	tcpClient_->connect();
}

void muduo::net::HttpClient::OnConnection(const muduo::net::TcpConnectionPtr& conn)
{
	if (conn->connected())
	{
		//请求行
		std::string send_str;
		send_str += request_.getHttpMethod();
		send_str += " ";
		send_str += request_.getPath();
		send_str += " ";
		send_str += request_.getHttpVersion();
		send_str += "\r\n";

		//请求头
		auto& headers = request_.getHeaders();
		for (auto item : headers)
		{
			send_str += item.first;
			send_str += ": ";
			send_str += item.second;
			send_str += "\r\n";
		}
		send_str += "\r\n";
		//主体
		send_str += request_.getBody();
		LOG_INFO << send_str;
		conn->send(send_str);
	}
}

void muduo::net::HttpClient::OnResponce(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time)
{
	//暂时只确认回应行
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
					if (state_code == 200 || state_code == 400)
					{
						LOG_INFO << state_code;
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
	conn->disconnected();
}
