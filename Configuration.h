#ifndef CONFIGURATION_H
#define CONFIGURATION_H
#include <map>
#include <string>
#include <vector>
#include "muduo/net/InetAddress.h"

struct AlgorithmServerInfo
{
	muduo::net::InetAddress addr_;
	int type_;
	AlgorithmServerInfo(const muduo::net::InetAddress& addr,int type):
		addr_(addr),
		type_(type)
	{}
	AlgorithmServerInfo():
		addr_(),
		type_(-1)
	{}
	AlgorithmServerInfo(const AlgorithmServerInfo& info):
		addr_(info.addr_),
		type_(info.type_)
	{}
	AlgorithmServerInfo& operator=(const AlgorithmServerInfo& info)
	{
		addr_ = info.addr_;
		type_ = info.type_;
		return *this;
	}
};

struct HttpInfo
{
	muduo::net::InetAddress addr_;
	std::string path_;
};

class Configuration
{
private:
	//���˺��㷨,ǰ�˷���ͨ�ŵı��ؼ�����ַ��Ϣ�ӱ��������ļ��ж�ȡ
	muduo::net::InetAddress backendAddr_;
	muduo::net::InetAddress algorithmServerAddr_;
	muduo::net::InetAddress frontendAddr_;
	//ǰ��,��� http�����Ϣ
	//HttpInfo frontend_addr_;
	HttpInfo peekBackendAddr_;
	//�㷨����Զ˵�ַ��Ϣ�Լ�����ͷ�����Ϣ��Ҫ��java��̨��ȡ
	std::map<std::string, AlgorithmServerInfo> services_managed_;
	//����ͷrtsp��Ϣ
	std::map<int64_t,std::string> camera_rtsp_;
	//����ͷ������Ϣ
	//std::vector<std::vector<int>> camera_distance_;
	std::map<int, std::map<int, int>> camera_distance_;

	
public:
	Configuration(const std::string &path);
	const muduo::net::InetAddress& getJavaBackendAddr() const;
	const muduo::net::InetAddress& getAlgorithmServerAddr() const;
	const muduo::net::InetAddress& getFrontendAddr() const
	{
		return frontendAddr_;
	}
	const HttpInfo& getBackendAddr() const
	{
		return peekBackendAddr_;
	}
	//��������ͷ��Ϣ
	void addCamera(int64_t id, const std::string& rtsp);
	const std::string getCamera(int64_t id) const;
	const std::map<int64_t, std::string>& cameras() const;

	//��������ͷ������Ϣ
	void addCameraDistance(int id1, int id2, int distance);
	int getCameraDistance(int id1, int id2) const;
	const std::map<int, std::map<int, int>>& cameraDistance() const;

	//�����㷨������Ϣ
	void addAlgorithmServer(const std::string& server_name,const AlgorithmServerInfo& algorithm_server_info);
	const AlgorithmServerInfo getAlgorithmServerInfo(const std::string& server_name) const;
	const std::map<std::string, AlgorithmServerInfo>& algorithmServerInfo() const;

};
#endif // !__ConfigurationReader



