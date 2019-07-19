
#include <iostream>
#include "Request.h"
#include "httplib.h"

const char *Request::USER_AGENT = "";

Request::Request() {
}

Request::~Request() {
}
std::string Request::Touch(const char *host, const char *path, Request::Method method) const {
  std::shared_ptr<httplib::Response> res{0};

  httplib::SSLClient client(host, 443, TIMEOUT);

  if (method == Request::Method::GET) {
    auto headers = httplib::Headers();

    res = client.Get(path);
    res->set_header("User-Agent", USER_AGENT);

  } else if (method == Request::Method::POST) {
    //res = client.Post(host);
  }
#ifdef DEBUG
  std::cout << method
            << " " << host
            << " " << path
            << std::endl;
#endif
  if (res) {
    if(res->status==200)
    return res->body;
  }
  std::cout << "res is null" << std::endl;
  return "";
}