#include "Configuration.h"
#include "Util.h"
#include "CJsonObject.hpp"
#include <iostream>

Configuration::Configuration(const std::string &path)
{
	std::string json_content = ReadConfigFile(path);
	neb::CJsonObject json(json_content);
	
	std::string ip;
	int port;
	//获取与java后台通信的本地监听地址
	json["java_back-end"].Get("listen_ip", ip);
	json["java_back-end"].Get("listen_port", port);
	backendAddr_ = std::move(muduo::net::InetAddress(ip, port));

	//获取与算法服务通信的本地监听地址
	json["algorithm_server"].Get("listen_ip", ip);
	json["algorithm_server"].Get("listen_port", port);
	algorithmServerAddr_ = std::move(muduo::net::InetAddress(ip, port));

	//获取前端IP地址和端口
	std::string requestPath;
	json["front-end_addr"].Get("listen_ip", ip);
	json["front-end_addr"].Get("listen_port", port);
	frontendAddr_ = std::move(muduo::net::InetAddress(ip, port));
	//frontend_addr_.path_ = requestPath;

	//获取后端相关信息
	json["back-end_addr"].Get("ip", ip);
	json["back-end_addr"].Get("port", port);
	json["back-end_addr"].Get("path", requestPath);
	peekBackendAddr_.addr_ = std::move(muduo::net::InetAddress(ip, port));
	peekBackendAddr_.path_ = requestPath;
}

const muduo::net::InetAddress& Configuration::getJavaBackendAddr() const
{
	// TODO: 在此处插入 return 语句
	return backendAddr_;
}

const muduo::net::InetAddress& Configuration::getAlgorithmServerAddr() const
{
	// TODO: 在此处插入 return 语句
	return algorithmServerAddr_;
}

void Configuration::addCamera(int64_t id, const std::string& rtsp)
{
	camera_rtsp_[id] = rtsp;
}

const std::string Configuration::getCamera(int64_t id) const
{
	// TODO: 在此处插入 return 语句
	std::string result;
	auto iter = camera_rtsp_.find(id);
	if (iter == camera_rtsp_.end())
		return result;
	else
	{
		result = iter->second;
		return result;
	}
}

const std::map<int64_t, std::string>& Configuration::cameras() const
{
	// TODO: 在此处插入 return 语句
	return camera_rtsp_;
}

void Configuration::addCameraDistance(int id1, int id2, int distance)
{
	camera_distance_[id1].insert({ id2,distance });
}

int Configuration::getCameraDistance(int id1, int id2) const
{
	int dis = 0;
	auto iter = camera_distance_.find(id1);
	if (iter == camera_distance_.end())
	{
		return dis;
	}
	auto iter1 = iter->second.find(id2);
	if (iter1 == iter->second.end())
	{
		return dis;
	}
	dis = iter1->second;
	return dis;
}

const std::map<int, std::map<int, int>>& Configuration::cameraDistance() const
{
	// TODO: 在此处插入 return 语句
	return camera_distance_;
}

void Configuration::addAlgorithmServer(const std::string& server_name, const AlgorithmServerInfo& algorithm_server_info)
{
	services_managed_[server_name] = algorithm_server_info;
}

const AlgorithmServerInfo Configuration::getAlgorithmServerInfo(const std::string& server_name) const
{
	AlgorithmServerInfo info;
	auto iter = services_managed_.find(server_name);
	if (iter == services_managed_.end())
	{
		return info;
	}
	info = iter->second;
	return info;
}

const std::map<std::string, AlgorithmServerInfo>& Configuration::algorithmServerInfo() const
{
	// TODO: 在此处插入 return 语句
	return services_managed_;
}




