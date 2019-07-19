
#ifndef HTTP__REQUEST_H_
#define HTTP__REQUEST_H_

#ifdef _WIN32
#    ifndef WSA_FLAG_NO_HANDLE_INHERIT
#        define WSA_FLAG_NO_HANDLE_INHERIT 0x80
#    endif
#endif

#include <string>
class Request {
 public:
  enum Method { GET, POST };
  Request();
  ~Request();
  std::string Touch(const char *host, const char *path, Method method) const;
 private:
  const static char *USER_AGENT;
  const static int TIMEOUT = 1000 * 5;

};

#endif //HTTP__REQUEST_H_
