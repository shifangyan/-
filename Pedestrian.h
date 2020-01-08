#ifndef __Pedestrian
#define __Pedestrian
//�����ʾһ������ ��Ҫ����ͼƬ�ϵ��������꣬�Լ���������

#include<opencv2/opencv.hpp>
#include<map>
#include<string>
#include"CJsonObject.hpp"

class Pedestrian
{
private:
	cv::Rect position_; //��ʾ���˵�����
	typedef std::map<std::string, std::string> AttributeMap;
	AttributeMap attribute_;  //��ʾ���˵ĸ�������
public:
	//Pedestrian(int x, int y, int width, int height);
	Pedestrian(int xmin, int ymin, int xmax, int ymax);
	void UpdateAttribute(std::string name, std::string value);
	void DelAttribute(std::string name);
	AttributeMap& GetAllAtribute();
	neb::CJsonObject GetPosition();
};
#endif // !__Pedestrian



