#ifndef CPPHTTPLIB_STREAM_H
#define CPPHTTPLIB_STREAM_H
namespace httplib
{
class Stream
{
public:
    virtual ~Stream() {}
    virtual int read(char* ptr, size_t size) = 0;
    virtual int write(const char* ptr, size_t size1) = 0;
    virtual int write(const char* ptr) = 0;
    virtual std::string get_remote_addr() const = 0;

    template <typename... Args>
    void write_format(const char* fmt, const Args&... args);
};
}
#endif