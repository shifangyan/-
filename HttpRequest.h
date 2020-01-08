#ifndef HTTPREQUEST
#define HTTPREQUEST
#include <string>
#include <map>

class HttpRequest
{
public:
	enum HttpMethod  //暂时支持三个
	{
		kGet,
		kPost,
		kHead,
	};
	enum HttpVersion //暂时只支持HTTP/1.1
	{
		kHttp1_1
	};
	void setHttpMethod(const HttpMethod& httpMethod)
	{
		httpMethod_ = httpMethod;
	}
	const std::string getHttpMethod() const
	{
		static char* str[] = { "GET","POST","HEAD" }; //暂时只支持三个
		return str[httpMethod_];
	}
	void setPath(const std::string& path)
	{
		path_ = path;
	}
	const std::string& getPath() const
	{
		return path_;
	}
	void setQuery(const std::string& query)
	{
		query_ = query;
	}
	const std::string& getQuery() const
	{
		return query_;
	}
	void setHttpVersion(const HttpVersion& httpVersion)
	{
		httpVersion_ = httpVersion;
	}
	const std::string getHttpVersion() const
	{
		static const char* httpVersionStr[] = { "HTTP/1.1" }; //暂时只支持1.1
		return httpVersionStr[httpVersion_];
	}
	void addHeader(const std::string& key, const std::string& value)
	{
		headers_[key] = value;
	}
	std::string getHeader(const std::string& key) const
	{
		std::string result;
		auto iter = headers_.find(key);
		if (iter != headers_.end())
			result = iter->second;
		return result;
	}
	const std::map<std::string, std::string>& getHeaders() const
	{
		return headers_;
	}
	void setBody(const std::string& body)
	{
		body_ = body;
	}
	const std::string& getBody() const
	{
		return body_;
	}
private:
	HttpMethod httpMethod_;
	std::string path_;
	std::string query_;
	HttpVersion httpVersion_;
	std::map<std::string, std::string> headers_;
	std::string body_;
};

#endif // !HTTPREQUEST



