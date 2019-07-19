#ifndef CPPHTTPLIB_SOCKETSTREAM_H
#define CPPHTTPLIB_SOCKETSTREAM_H
namespace httplib
{
class SocketStream : public Stream
{
public:
    SocketStream(socket_t sock);
    virtual ~SocketStream();

    virtual int read(char* ptr, size_t size);
    virtual int write(const char* ptr, size_t size);
    virtual int write(const char* ptr);
    virtual std::string get_remote_addr() const;

private:
    socket_t sock_;
};
}
#endif