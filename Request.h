
#ifndef HTTP__REQUEST_H_
#define HTTP__REQUEST_H_

#ifdef _WIN32
#    ifndef WSA_FLAG_NO_HANDLE_INHERIT
#        define WSA_FLAG_NO_HANDLE_INHERIT 0x80
#    endif
#endif

#include <string>
#include <unordered_map>

class Request
{
public:
    enum Method
    {
        GET,
        POST
    };
    Request();
    ~Request();
    std::string Touch(const char* host, const char* path,
        std::unordered_map<const char*, const char*> headers,
        bool https, bool mobile, Request::Method method) const;
    bool Ok(const char* host, const char* path, bool https) const;

private:
    const static int TIMEOUT = 1000 * 5;
};

#endif //HTTP__REQUEST_H_
