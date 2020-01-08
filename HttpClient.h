#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H
#include "muduo/base/noncopyable.h"
//#include "muduo/net/http/HttpRequest.h"
#include "muduo/net/InetAddress.h"
#include "muduo/net/TcpClient.h"
#include <memory>
#include "muduo/base/Logging.h"
#include <string>
#include "HttpRequest.h"
//目前只支持短连接
namespace muduo
{
namespace net
{
	class HttpClient:noncopyable
	{
	public:
		typedef std::unique_ptr<TcpClient> TcpClientPtr;
		typedef std::function<void(const muduo::net::TcpConnectionPtr & conn)> OnConnectionCallBack;
		HttpClient(EventLoop* loop,const InetAddress& addr);
		//void setRequest(const ::HttpRequest& req);
		void sendRequest(const ::HttpRequest& req);
		
	private:
		void OnConnection(const muduo::net::TcpConnectionPtr& conn);
		void OnResponce(const TcpConnectionPtr& conn,Buffer* buf,Timestamp time);
		::HttpRequest request_;
		InetAddress addr_;
		TcpClientPtr tcpClient_;
		EventLoop* loop_;
		OnConnectionCallBack onConnectionCallBack_;
	};
}
}

#endif // !HTTPCLIENT_H



