#ifndef FRONTENDSERVER_H
#define FRONTENDSERVER_H
#include "WebsocketServer.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/InetAddress.h"
#include "muduo/net/EventLoop.h"

class Configuration;
class AlgorithmServerCommunication;
class FrontendServer
{
public:
	FrontendServer(muduo::net::EventLoop* loop, Configuration* config, AlgorithmServerCommunication* algo);
	void send(const std::string& sendStr);
private:
	void FrontendCallback(const muduo::net::TcpConnectionPtr& conn, std::string& payloadData);
	muduo::net::WebsocketServer server_;
	Configuration* config_;
	AlgorithmServerCommunication* algo_;
	muduo::net::EventLoop* loop_;
};
#endif // !FRONTENDSERVER_H



