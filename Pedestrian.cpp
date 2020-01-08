#include "Pedestrian.h"

Pedestrian::Pedestrian(int xmin, int ymin, int xmax, int ymax):position_(xmin, ymin, xmax-xmin,ymax-xmin)
{
}

void Pedestrian::UpdateAttribute(std::string name, std::string value)
{
	attribute_[name] = value;
}

void Pedestrian::DelAttribute(std::string name)
{
	attribute_.erase(name);
}

Pedestrian::AttributeMap& Pedestrian::GetAllAtribute()
{
	// TODO: 在此处插入 return 语句
	return attribute_;
}

neb::CJsonObject Pedestrian::GetPosition()
{
	neb::CJsonObject position_json;
	position_json.Add("xmin", position_.tl().x);
	position_json.Add("ymin", position_.tl().y);
	position_json.Add("xmax", position_.br().x);
	position_json.Add("ymax", position_.br().y);
	return position_json;
}
