#ifndef WEBSOCKETSERVER
#define WEBSOCKETSERVER
#include "muduo/net/TcpServer.h"
#include <map>
#include "muduo/net/http/HttpRequest.h"
#include "muduo/net/http/HttpResponse.h"
#include "WebSocketContent.h"

namespace muduo
{
namespace net
{
	class WebsocketServer
	{
	public:
		typedef std::function<void(const TcpConnectionPtr & conn,std::string& payloadData)> ResponceFunc;
		WebsocketServer(EventLoop* loop, const InetAddress& listenAddr);
		bool connected() const
		{
			return websocketConnected_;
		}
		void setResponceFunc(const ResponceFunc& func)
		{
			responceFunc_ = func;
		}
		void start();
		void send(const std::string& send_str);
	private:
		void onConnection(const TcpConnectionPtr& conn);
		void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime);
		bool praseHttpRequest(Buffer* buf, HttpRequest& req);
		bool parseRequestLine(Buffer* buf, HttpRequest& req);
		bool parseRequestHeader(Buffer* buf, HttpRequest& req);
		bool parseRequestBody(Buffer* buf, HttpRequest& req);
		bool buildHttpResponce(const HttpRequest& req, HttpResponse& rep);
		std::string SHA1(const std::string& src);
		bool sendHttpResponce(const TcpConnectionPtr& conn, const HttpResponse& rep);
		bool parseWebSocketContent(Buffer* buf, WebSocketContent& content);
		//void onRequest(std::string pay_load);
		void echo(const TcpConnectionPtr& conn, std::string& payloadData);
		
	//	std::map<std::string, ResponceFunc> request2Responce_;
		TcpServer tcpServer_;
		TcpConnectionPtr tcpConnection_;
		bool websocketConnected_;
		ResponceFunc responceFunc_;
	};
}
}

#endif // !WEBSOCKETSERVER