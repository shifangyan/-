#ifndef  __BASE64
#define __BASE64
#include<string>

void base64_encode(const char* bytes_to_encode, unsigned int in_len,std::string& result);
std::string base64_decode(std::string const& encoded_string);
#endif // ! __BASE64


