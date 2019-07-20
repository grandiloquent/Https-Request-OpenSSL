
#include <iostream>
#include "Request.h"
#include "httplib.h"

const char* Request::PC_USER_AGENT = "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/73.0.3683.56 Safari/537.36";

Request::Request()
{
}

Request::~Request()
{
}
std::string Request::Touch(const char* host, const char* path,
    const char* referer,
    bool https, bool mobile, Request::Method method) const
{

    httplib::Client* client;

    std::shared_ptr<httplib::Response> res{ 0 };
    if (https)
    {
        client = new httplib::SSLClient(host, 443, TIMEOUT);
    }
    else
    {
        client = new httplib::Client(host, 80, TIMEOUT);
    }
    if (method == Request::Method::GET)
    {
        std::cout << "GET" << std::endl;
        res = client->Get(path);
    }
    else if (method == Request::Method::POST)
    {
        //res = client.Post(host);
    }
    if (mobile)
    {
        res->set_header("User-Agent", PC_USER_AGENT);
    }
    else
    {
        res->set_header("User-Agent", PC_USER_AGENT);
    }
    if (referer)
    {
        res->set_header("Referer", referer);
    }

#ifdef DEBUG
    std::cout << method
              << " " << host
              << " " << path
              << std::endl;
#endif
    if (res)
    {
        if (res->status == 200)
        {
            return res->body;
        }
    }
    return std::string();
}