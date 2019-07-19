
#include <iostream>
#include "Request.h"
#include "httplib.h"

Request::Request()
{
}

Request::~Request()
{
}
std::string Request::Touch(const char* host, const char* path, Request::Method method) const
{
    std::shared_ptr<httplib::Response> res{ 0 };
    httplib::Client client(host);
    if (method == Request::Method::GET)
    {

        res = client.Get(host);
    }
    else if (method == Request::Method::POST)
    {
        //res = client.Post(host);
    }
#ifndef DEBUG
    std::cout << method
              << " " << host
              << " " << path
              << std::endl;
#endif
    if (res)
    {
        std::cout << res->status << std::endl;
        return res->body;
    }
    std::cout << "res is null" << std::endl;
    return "";
}