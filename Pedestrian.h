#ifndef __Pedestrian
#define __Pedestrian
//此类表示一个行人 主要有在图片上的像素坐标，以及多种属性

#include<opencv2/opencv.hpp>
#include<map>
#include<string>
#include"CJsonObject.hpp"

class Pedestrian
{
private:
	cv::Rect position_; //表示行人的坐标
	typedef std::map<std::string, std::string> AttributeMap;
	AttributeMap attribute_;  //表示行人的各项属性
public:
	//Pedestrian(int x, int y, int width, int height);
	Pedestrian(int xmin, int ymin, int xmax, int ymax);
	void UpdateAttribute(std::string name, std::string value);
	void DelAttribute(std::string name);
	AttributeMap& GetAllAtribute();
	neb::CJsonObject GetPosition();
};
#endif // !__Pedestrian



