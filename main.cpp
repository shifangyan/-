#include <string>
#include "muduo/net/EventLoop.h"
#include "Configuration.h"
//#include "WebsocketServer.h"
#include "muduo/base/Logging.h"
#include "AlgorithmServerCommunication.h"
#include "JavaBackendCommunication.h"
#include "FrontendServer.h"

using namespace std;
using namespace muduo;
using namespace muduo::net;


int main(int argc,char **argv)
{
	muduo::Logger::setLogLevel(muduo::Logger::LogLevel::DEBUG);

	std::string path = "../../../config.txt";
	Configuration config(path);

	muduo::net::EventLoop loop;

	AlgorithmServerCommunication algoServer_(&config,&loop);
	JavaBackendCommunication backend(&config, &loop,&algoServer_);
	//WebsocketServer webSocketServer(&loop,config.getFrontendAddr());
	FrontendServer frontServer(&loop, &config,&algoServer_);
	//Task task(0, TaskType::Track,&loop,&net,&config);
	//task.AddVideoStream(std::string("rtsp://admin:DKHLdkhl2016@10.51.20.161/main/Channels/1"));
	//net.SetTask(&task);
	//task.Start();

	//HTTP client ≤‚ ‘
	/*HttpRequest httpRequest;
	httpRequest.setHttpMethod(HttpRequest::HttpMethod::kPost);
	std::string path("/test");
	httpRequest.setPath(path);
	httpRequest.setHttpVersion(HttpRequest::HttpVersion::kHttp1_1);
	std::string body("12345678");
	httpRequest.addHeader("Content-Length",std::to_string(body.size()));
	httpRequest.addHeader("Host", "10.51.70.114");
	httpRequest.setBody(body);

	muduo::net::InetAddress addr("10.51.70.114", 8080);
	muduo::net::HttpClient httpClient(&loop, addr);
	httpClient.sendRequest(httpRequest);*/

	//WEBSOCKET CLIENT ≤‚ ‘
//	muduo::net::InetAddress addr("10.51.70.114", 8080);
//	muduo::net::WebSocketClient webSocketClient(&loop, addr);
	
//	webSocketClient.connect();
	//webSocketClient.send(request);

	//WEBSOCKET SERVER≤‚ ‘
//	muduo::net::InetAddress addr(9001);
//	muduo::net::WebsocketServer server(&loop, addr);
	loop.loop();
}