#ifndef CPPHTTPLIB_CLIENT_H
#define CPPHTTPLIB_CLIENT_H
namespace httplib
{

class Client
{
public:
    Client(const char* host, int port = 80, time_t timeout_sec = 300);

    virtual ~Client();

    virtual bool is_valid() const;

    std::shared_ptr<Response> Get(const char* path, Progress progress = nullptr);
    std::shared_ptr<Response> Get(const char* path, const Headers& headers,
        Progress progress = nullptr);

    std::shared_ptr<Response> Get(const char* path,
        ContentReceiver content_receiver,
        Progress progress = nullptr);
    std::shared_ptr<Response> Get(const char* path, const Headers& headers,
        ContentReceiver content_receiver,
        Progress progress = nullptr);

    std::shared_ptr<Response> Head(const char* path);
    std::shared_ptr<Response> Head(const char* path, const Headers& headers);

    std::shared_ptr<Response> Post(const char* path, const std::string& body,
        const char* content_type);
    std::shared_ptr<Response> Post(const char* path, const Headers& headers,
        const std::string& body,
        const char* content_type);

    std::shared_ptr<Response> Post(const char* path, const Params& params);
    std::shared_ptr<Response> Post(const char* path, const Headers& headers,
        const Params& params);

    std::shared_ptr<Response> Put(const char* path, const std::string& body,
        const char* content_type);
    std::shared_ptr<Response> Put(const char* path, const Headers& headers,
        const std::string& body,
        const char* content_type);

    std::shared_ptr<Response> Patch(const char* path, const std::string& body,
        const char* content_type);
    std::shared_ptr<Response> Patch(const char* path, const Headers& headers,
        const std::string& body,
        const char* content_type);

    std::shared_ptr<Response> Delete(const char* path,
        const std::string& body = std::string(),
        const char* content_type = nullptr);
    std::shared_ptr<Response> Delete(const char* path, const Headers& headers,
        const std::string& body = std::string(),
        const char* content_type = nullptr);

    std::shared_ptr<Response> Options(const char* path);
    std::shared_ptr<Response> Options(const char* path, const Headers& headers);

    bool send(Request& req, Response& res);

protected:
    bool process_request(Stream& strm, Request& req, Response& res,
        bool& connection_close);

    const std::string host_;
    const int port_;
    time_t timeout_sec_;
    const std::string host_and_port_;

private:
    socket_t create_client_socket() const;
    bool read_response_line(Stream& strm, Response& res);
    void write_request(Stream& strm, Request& req);

    virtual bool read_and_close_socket(socket_t sock, Request& req,
        Response& res);
    virtual bool is_ssl() const;
};
}
#endif