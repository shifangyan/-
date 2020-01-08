#ifndef WEBSOCKET_H
#define WEBSOCKET_H
#include "HttpRequest.h"
#include "muduo/net/InetAddress.h"
#include "muduo/net/TcpClient.h"
#include "string"

namespace muduo
{
namespace net
{
	class WebSocketClient
	{
	public:
		enum OpcodeType
		{
			kContinue,
			kText,
			kBinary,
			kClose,
			kPing,
			kPong
		};

		typedef std::unique_ptr<TcpClient> TcpClientPtr;
		typedef std::function<void(const muduo::net::TcpConnectionPtr & conn)> OnConnectionCallBack;
		WebSocketClient(EventLoop* loop, const InetAddress& addr,std::string path);
		void connect();
		void send(const std::string& send_str);
	//	void upgradeProtocol(const muduo::net::TcpConnectionPtr& conn);
	//	void upgradeResponce()
		void connected()
		{
			connected_ = true;
		}
		void disconnected()
		{
			connected_ = false;
		}
		bool getConnected() const
		{
			return connected_;
		}
	private:
		void onConnection(const muduo::net::TcpConnectionPtr& conn);
		::HttpRequest request_;
		muduo::net::InetAddress addr_;
		TcpClientPtr tcpClient_;
		EventLoop* loop_;
		OnConnectionCallBack onConnectionCallBack_;
		muduo::net::TcpConnectionPtr conn_;
		std::string path_;
		bool connected_;
		void onResponce(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time);
};
}
}

#endif // !WEBSOCKET_H



