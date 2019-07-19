#include <iostream>
#include "Request.h"

int main()
{
    Request request;

    std::string body = request.Touch("cn.bing.com", "/search?q=github%20search%20in%20filename&qs=n&form=QBRE&sp=-1&pq=github%20search%20i%20filename&sc=0-24&sk=&cvid=10BBE06651CD4C31B064731802EFF839", Request::GET);

    std::cout << body << std::endl;
   
    return 0;
}