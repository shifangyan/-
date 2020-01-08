#include "Util.h"
#include<iostream>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>

std::string ReadConfigFile(std::string file_name)
{
	int fd = ::open(file_name.c_str(), O_RDONLY);
	if (fd < 0)
	{
		std::cout << strerror(errno) << std::endl;
		exit(-1);
	}
	int size = 100;
	char buf[size];
	int ret;
	std::string result;
	while ((ret = read(fd, buf, size)) != 0)
	{
		if (ret < 0)
		{
			std::cout << strerror(errno) << std::endl;
			exit(-1);
		}
		else
			result.insert(result.size(), buf, ret);
	}
	return result;
}
