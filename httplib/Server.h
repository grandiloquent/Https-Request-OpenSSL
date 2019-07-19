#ifndef CPPHTTPLIB_SERVER_H
#define CPPHTTPLIB_SERVER_H
namespace httplib
{

class Server
{
public:
    typedef std::function<void(const Request&, Response&)> Handler;
    typedef std::function<void(const Request&, const Response&)> Logger;

    Server();

    virtual ~Server();

    virtual bool is_valid() const;

    Server& Get(const char* pattern, Handler handler);
    Server& Post(const char* pattern, Handler handler);

    Server& Put(const char* pattern, Handler handler);
    Server& Patch(const char* pattern, Handler handler);
    Server& Delete(const char* pattern, Handler handler);
    Server& Options(const char* pattern, Handler handler);

    bool set_base_dir(const char* path);

    void set_error_handler(Handler handler);
    void set_logger(Logger logger);

    void set_keep_alive_max_count(size_t count);
    void set_payload_max_length(uint64_t length);

    int bind_to_any_port(const char* host, int socket_flags = 0);
    bool listen_after_bind();

    bool listen(const char* host, int port, int socket_flags = 0);

    bool is_running() const;
    void stop();

protected:
    bool process_request(Stream& strm, bool last_connection,
        bool& connection_close,
        std::function<void(Request&)> setup_request = nullptr);

    size_t keep_alive_max_count_;
    size_t payload_max_length_;

private:
    typedef std::vector<std::pair<std::regex, Handler>> Handlers;

    socket_t create_server_socket(const char* host, int port,
        int socket_flags) const;
    int bind_internal(const char* host, int port, int socket_flags);
    bool listen_internal();

    bool routing(Request& req, Response& res);
    bool handle_file_request(Request& req, Response& res);
    bool dispatch_request(Request& req, Response& res, Handlers& handlers);

    bool parse_request_line(const char* s, Request& req);
    void write_response(Stream& strm, bool last_connection, const Request& req,
        Response& res);

    virtual bool read_and_close_socket(socket_t sock);

    std::atomic<bool> is_running_;
    std::atomic<socket_t> svr_sock_;
    std::string base_dir_;
    Handlers get_handlers_;
    Handlers post_handlers_;
    Handlers put_handlers_;
    Handlers patch_handlers_;
    Handlers delete_handlers_;
    Handlers options_handlers_;
    Handler error_handler_;
    Logger logger_;

    // TODO: Use thread pool...
    std::mutex running_threads_mutex_;
    int running_threads_;
};
}
#endif