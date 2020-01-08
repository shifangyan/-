#ifndef ALGORITHMSERVERCOMMUNICATION_H
#define ALGORITHMSERVERCOMMUNICATION_H
#include <set>
#include <map>
#include "muduo/net/TcpConnection.h"
#include "muduo/net/TcpClient.h"
#include "muduo/net/TcpServer.h"
#include "CJsonObject.hpp"
#include "Task.h"

class Configuration;
//class Task;
class EventLoop;
class AlgorithmServerCommunication
{
public:
	enum TaskState {
		kRunning,
		kStop
	};
	typedef std::unique_ptr<Task> TaskPtr;
	AlgorithmServerCommunication(Configuration* config, muduo::net::EventLoop* loop);
	void ServerOnConnection(const muduo::net::TcpConnectionPtr& conn);
	void OnMessage(const muduo::net::TcpConnectionPtr& conn, muduo::net::Buffer* buf, muduo::Timestamp receive_time);
	void MessageProcess(const std::string& message);
	void ConnectAlgorithmServer();
	void ClientOnConnection(const muduo::net::TcpConnectionPtr& conn);
	void setTask(TaskPtr& task) 
	{
		task_ = std::move(task);
	}
	void Send(const std::string& channel, neb::CJsonObject& json);
	bool taskRunning()
	{
		return (task_state_ == TaskState::kRunning);
	}
	void setTaskState(TaskState s)
	{
		task_state_ = s;
	}
private:
	typedef std::set<muduo::net::TcpConnectionPtr> ServerConnectionList;
	typedef std::map<std::string, muduo::net::TcpConnectionPtr> ClientConnectMap;
	muduo::net::TcpServer server_;
	std::set<std::unique_ptr<muduo::net::TcpClient>> client_managed_;
	ServerConnectionList server_connections_;
	ClientConnectMap client_connections_;
	TaskPtr task_;
	Configuration *config_;
	muduo::net::EventLoop *loop_;
	TaskState task_state_;
};
#endif
