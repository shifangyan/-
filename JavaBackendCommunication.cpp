#include "JavaBackendCommunication.h"
#include "muduo/net/http/HttpRequest.h"
#include "muduo/net/http/HttpResponse.h"
#include "muduo/base/Logging.h"
#include "AlgorithmServerCommunication.h"
#include "Task.h"

JavaBackendCommunication::JavaBackendCommunication(Configuration* config,muduo::net::EventLoop* loop, AlgorithmServerCommunication* algo):
	config_(config),
	http_server_(loop,config_->getJavaBackendAddr(),std::string("java_server")),
	algo_(algo),
	loop_(loop)
{
	muduo::net::HttpServer::HttpCallback func = std::bind(&JavaBackendCommunication::httpCallBack, this,std::placeholders::_1,std::placeholders::_2, std::placeholders::_3);
	http_server_.setHttpCallback(func);

	//注册任务
	PathFunc path_func = std::bind(&JavaBackendCommunication::updateCameraConf, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	path_funcs_.insert({ "/updateCameraConf",path_func });

	path_func = std::bind(&JavaBackendCommunication::updateAlgorithmConf, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	path_funcs_.insert({ "/updateAlgorithmConf",path_func });

//	path_func = std::bind(&JavaBackendCommunication::detectionTask, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
//	path_funcs_.insert({ "/detectionTask",path_func });

//	path_func = std::bind(&JavaBackendCommunication::trackTask, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
//	path_funcs_.insert({ "/trackTask",path_func });

	http_server_.start();
}

void JavaBackendCommunication::httpCallBack(const muduo::net::TcpConnectionPtr & conn, const muduo::net::HttpRequest & request, muduo::net::HttpResponse * response)
{
		LOG_DEBUG << "httpCallBack path:" << request.path();
		auto iter = path_funcs_.find(request.path());
		if (iter == path_funcs_.end())
		{
			LOG_ERROR << "can not find the path task";
			return;
		}

		bool ok = iter->second(conn, request, response); //执行对应的任务函数

		//设置responce
		if (!ok)
		{
			response->setStatusCode(muduo::net::HttpResponse::HttpStatusCode::k400BadRequest);
			response->setStatusMessage("Bad Request");
			//responce->setCloseConnection(true);
			return;
		}
		response->setStatusCode(muduo::net::HttpResponse::HttpStatusCode::k200Ok);
		response->setStatusMessage("OK");
		//responce->setCloseConnection(true);
		return;
}

bool JavaBackendCommunication::updateCameraConf(const muduo::net::TcpConnectionPtr& conn, const muduo::net::HttpRequest& request, muduo::net::HttpResponse* responce)
{
	responce->setCloseConnection(true); 
	bool ok = true;
	auto& body = request.body();
	if (body.empty())
	{
		LOG_ERROR << "updateCameraConf::body is empty";
		ok = false;
		return ok;
	}
	neb::CJsonObject json(body);
	LOG_DEBUG << request.getHeader("Content-Length");
	//LOG_DEBUG << body;
	LOG_DEBUG << json.ToString();
	//获取cameraConf
	for (int i = 0; i < json["cameraConf"].GetArraySize(); i++)
	{
		neb::CJsonObject son_json;
		json["cameraConf"].Get(i, son_json);
		int64_t id;
		std::string rtsp;
		son_json.Get("id", id);
		son_json.Get("cameraRtsp", rtsp);
		config_->addCamera(id, rtsp);
	}

	//获取cameraRelationConf
	for (int i = 0; i < json["cameraRelationConf"].GetArraySize(); i++)
	{
		neb::CJsonObject son_json;
		json["cameraRelationConf"].Get(i, son_json);
		int64_t id1,id2;
		int dis;
		son_json.Get("cameraId1", id1);
		son_json.Get("cameraId2", id2);
		son_json.Get("distance", dis);
		config_->addCameraDistance(id1, id2, dis);
	}
	LOG_INFO << "receive the camera info";
	return ok;
}

bool JavaBackendCommunication::updateAlgorithmConf(const muduo::net::TcpConnectionPtr& conn, const muduo::net::HttpRequest& request, muduo::net::HttpResponse* responce)
{
	responce->setCloseConnection(true);
	bool ok = true;
	auto& body = request.body();
	if (body.empty())
	{
		LOG_ERROR << "updateCameraConf::body is empty";
		ok = false;
		return ok;
	}
	neb::CJsonObject json(body);

	//获取AlgorithmConf
	for (int i = 0; i < json.GetArraySize(); i++)
	{
		neb::CJsonObject son_json;
		json.Get(i, son_json);
		std::string server_name,ip,portStr;
		int type,port;
		son_json.Get("serviceName", server_name);
		son_json.Get("serviceIp", ip);
		son_json.Get("servicePort", portStr);
		port = std::stoi(portStr);
		son_json.Get("serviceType", type);
		muduo::net::InetAddress addr(ip, port);
		AlgorithmServerInfo info(addr, type);
		config_->addAlgorithmServer(server_name, info);
	}

	algo_->ConnectAlgorithmServer();
	LOG_INFO << "receive the algorithm server info";
	return ok;
}