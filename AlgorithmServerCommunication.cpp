#include "AlgorithmServerCommunication.h"
#include "Configuration.h"
#include "muduo/base/Logging.h"
#include "Task.h"

AlgorithmServerCommunication::AlgorithmServerCommunication(Configuration* config, muduo::net::EventLoop* loop):
	server_(loop,config->getAlgorithmServerAddr(),"algorithm_server"),
	config_(config),
	loop_(loop),
	task_state_(TaskState::kStop)
{
	//�㷨���������
	server_.setConnectionCallback(std::bind(&AlgorithmServerCommunication::ServerOnConnection, this, std::placeholders::_1));
	server_.setMessageCallback(std::bind(&AlgorithmServerCommunication::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	server_.start(); //��ʼ����
}

void AlgorithmServerCommunication::ServerOnConnection(const muduo::net::TcpConnectionPtr& conn)
{
	if (conn->connected())
	{
		//auto pos = conn->name().find(':');
		//auto name = conn->name().substr(0, pos);

		server_connections_.insert(conn);
	}
	else
	{
		server_connections_.erase(conn);
	}
	LOG_INFO << "name:" << conn->name() << "\t" << conn->localAddress().toIpPort() << "<-"
		<< conn->peerAddress().toIpPort() << " is"
		<< (conn->connected() ? "Up" : "Down");
}

void AlgorithmServerCommunication::OnMessage(const muduo::net::TcpConnectionPtr& conn, muduo::net::Buffer* buf, muduo::Timestamp receive_time)
{
	while (buf->readableBytes() >= 4)
	{
		const int32_t len = buf->peekInt32();
		//const int32_t len = muduo::net::sockets::networkToHost32(buf->peekInt32());//���˷������� ����
		printf("len:%x\n", len);
		//const int32_t len = muduo::net::sockets::networkToHost32(
		if (len > 65535 || len < 0)
		{
			LOG_ERROR << "Invalid buf length:" << len;
			buf->retrieveAll(); //���buf
			break;
		}
		else if (buf->readableBytes() >= 4 + len) //����������
		{
			buf->retrieve(4);
			muduo::string message(buf->peek(), len);
			MessageProcess(message);
			buf->retrieve(len);
		}
		else
		{
			break;
		}
	}
}

void AlgorithmServerCommunication::MessageProcess(const std::string& message)
{
	neb::CJsonObject json(message);
	LOG_INFO << "receive the json:\t" 
		<< json.ToFormattedString();
	int taskID;
	bool ret = json.Get("taskID", taskID);
	if (!ret)
	{
		LOG_ERROR << "the message not find taskID field" ;
		return;
	}
	if (task_ == nullptr || taskID != task_->GetTaskID())
	{
		LOG_ERROR << "can not find the task";
		return;
	}

	task_->receiveMessage(json);
}

void AlgorithmServerCommunication::ConnectAlgorithmServer()
{
	auto& algorithm_info = config_->algorithmServerInfo();
	//�ͻ�������
	for (auto iter = algorithm_info.begin(); iter != algorithm_info.end(); ++iter)
	{
		LOG_INFO << "the server " << iter->first << " " << " connecting " << "Addr:" << iter->second.addr_.toIpPort();
		std::unique_ptr<muduo::net::TcpClient> ptr(new muduo::net::TcpClient(loop_, iter->second.addr_, iter->first));
		ptr->setConnectionCallback(std::bind(&AlgorithmServerCommunication::ClientOnConnection, this, std::placeholders::_1)); //�������ӽ����ɹ��ص�����
		ptr->connect(); //��ʼ����
		client_managed_.insert(std::move(ptr));
	}
}

void AlgorithmServerCommunication::ClientOnConnection(const muduo::net::TcpConnectionPtr& conn)
{
	auto start = conn->name().find('-');
	auto end = conn->name().find(':');
	auto name = conn->name().substr(start + 1, end - start - 1);
	if (conn->connected())
	{

		client_connections_.insert({ name,conn });
	}
	else
	{
		client_connections_.erase(name);
	}
	LOG_INFO << "name:" << conn->name() << "\t" << conn->localAddress().toIpPort() << "->"
		<< conn->peerAddress().toIpPort() << " is"
		<< (conn->connected() ? "Up" : "Down");
}

void AlgorithmServerCommunication::Send(const std::string& channel,neb::CJsonObject& json)
{
	if (client_connections_.find(channel) == client_connections_.end())
	{
		std::cout << "error:can not find the send channel" << std::endl;
		return;
	}
	//��ӱ�Ҫ�İ�ͷ
	auto conn = client_connections_[channel];
	const muduo::net::InetAddress local_addr = conn->localAddress();
	const muduo::net::InetAddress target_addr = conn->peerAddress();
	json.Add("localIP", local_addr.toIp());
	json.Add("localPort", local_addr.toPort());
	json.Add("targetIP", target_addr.toIp());
	json.Add("targetPort", target_addr.toPort());

	//����
	std::string send_str = json.ToString();
	//std::cout << send_str << std::endl;
	int32_t len = static_cast<int32_t>(send_str.size());
	LOG_INFO << "send buf length: " << len;
	int32_t be32 = muduo::net::sockets::hostToNetwork32(len); //�����ֽ����Ǵ�˴洢
	conn->send((void*)&be32, sizeof(be32));
	conn->send(send_str);
}
