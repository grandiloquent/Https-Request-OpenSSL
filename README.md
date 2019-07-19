
## 命令

```
g++ -DCPPHTTPLIB_OPENSSL_SUPPORT -DDEBUG -o bin\Request.o -c Request.cpp
g++ -g -shared -o bin\libRequest.dll bin\Request.o -lws2_32 -lssl -lcrypto
g++ main.cpp -o bin\main.exe -Lbin -lRequest

g++ -DCPPHTTPLIB_OPENSSL_SUPPORT -DDEBUG -o bin\Request.o -c Request.cpp && g++ -g -shared -o bin\libRequest.dll bin\Request.o -lws2_32 -lssl -lcrypto && g++ main.cpp -o bin\main.exe -Lbin -lRequest && bin\main.exe
g++ main.cpp -o bin\main.exe -Lbin -lRequest && bin\main.exe
```

- https://github.com/openssl/openssl
- https://github.com/yhirose/cpp-httplib
