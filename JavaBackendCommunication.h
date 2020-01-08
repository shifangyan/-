#ifndef JAVABACKENDCOMMUNICATION
#define JAVABACKENDCOMMUNICATION
#include "muduo/net/http/HttpServer.h"
#include "muduo/net/EventLoop.h"
#include "Configuration.h"
#include <map>


class AlgorithmServerCommunication;
class JavaBackendCommunication
{
public:
	JavaBackendCommunication(Configuration* config,muduo::net::EventLoop* loop,AlgorithmServerCommunication *algo);
private:
	typedef std::function<bool(const muduo::net::TcpConnectionPtr &,const muduo::net::HttpRequest&, muduo::net::HttpResponse *)> PathFunc;
	void httpCallBack(const muduo::net::TcpConnectionPtr& conn,const muduo::net::HttpRequest& request,muduo::net::HttpResponse *response);
	bool updateCameraConf(const muduo::net::TcpConnectionPtr& conn,const muduo::net::HttpRequest& request, muduo::net::HttpResponse* responce);
	bool updateAlgorithmConf(const muduo::net::TcpConnectionPtr& conn,const muduo::net::HttpRequest& request, muduo::net::HttpResponse* responce);
	Configuration* config_;
	muduo::net::HttpServer http_server_;
	typedef muduo::net::HttpServer::HttpCallback HttpCallback;
	std::map<std::string, PathFunc> path_funcs_;
	AlgorithmServerCommunication* algo_;
	muduo::net::EventLoop *loop_;
};

#endif // !JAVABACKENDCOMMUNICATION



