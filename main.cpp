#include <iostream>
#include "Request.h"
#include "Utils.h"

int main() {
  Request request;

  std::string body = request.Touch("cn.bing.com", "/", Request::GET);

  std::cout << body << std::endl;
  return 0;
}