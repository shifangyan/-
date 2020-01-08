#include "FrontendServer.h"
#include "muduo/base/Logging.h"
#include "CJsonObject.hpp"
#include "Configuration.h"
#include "AlgorithmServerCommunication.h"

FrontendServer::FrontendServer(muduo::net::EventLoop* loop, Configuration* config, AlgorithmServerCommunication* algo):
	server_(loop,config->getFrontendAddr()),
	config_(config),
	algo_(algo),
	loop_(loop)
{
	server_.setResponceFunc(std::bind(&FrontendServer::FrontendCallback, this, std::placeholders::_1, std::placeholders::_2));
	server_.start();
}

void FrontendServer::send(const std::string& sendStr)
{
	LOG_DEBUG << "send websocket message";
	server_.send(sendStr);
}

void FrontendServer::FrontendCallback(const muduo::net::TcpConnectionPtr& conn, std::string& payloadData)
{
	neb::CJsonObject json(payloadData);
	LOG_DEBUG << json.ToString();
	std::string taskType;
	json.Get("taskType", taskType);
	if (taskType.empty())
	{
		LOG_ERROR << "can not find taskType";
		return;
	}
	if (taskType != "detection")
	{
		LOG_ERROR << "can not find taskType";
		return;
	}
	std::string cameraIDStr;
	json.Get("id", cameraIDStr);
	int64_t cameraID = std::stol(cameraIDStr);
	auto cameraRtsp = config_->getCamera(cameraID);
	if ( cameraRtsp.empty())
	{
		LOG_ERROR << "can not find the camera";
		return;
	}

	static int task_id = 0;
	AlgorithmServerCommunication::TaskPtr task(new Task(task_id++, TaskType::Detection, loop_, algo_, config_));
	task->setVideoStream(cameraID, cameraRtsp);
	task->setFrontServer(this);
	task->start();
	algo_->setTask(task);
	algo_->setTaskState(AlgorithmServerCommunication::TaskState::kRunning);
}
