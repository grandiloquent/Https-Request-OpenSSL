
#include <iostream>
#include "Request.h"
#include "httplib.h"

Request::Request()
{
}

Request::~Request()
{
}
std::string Request::Touch(const char* host, const char* path,
    std::unordered_map<const char*, const char*> headers,
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
        res = client->Get(path);
    }
    else if (method == Request::Method::POST)
    {
        //res = client.Post(host);
    }

    for (std::unordered_map<const char*, const char*>::iterator i = headers.begin(), end = headers.end(); i != end; ++i)
    {
        res->set_header(i->first, i->second);
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

bool Request::Ok(const char* host, const char* path, bool https) const
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
    res = client->Get(path);
    if (res)
    {
        if (res->status == 200)
        {
            return true;
        }
    }
    return false;
}