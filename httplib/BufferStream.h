#ifndef CPPHTTPLIB_BUFFERSTREAM_H
#define CPPHTTPLIB_BUFFERSTREAM_H
namespace httplib
{
class BufferStream : public Stream
{
public:
    BufferStream() {}
    virtual ~BufferStream() {}

    virtual int read(char* ptr, size_t size);
    virtual int write(const char* ptr, size_t size);
    virtual int write(const char* ptr);
    virtual std::string get_remote_addr() const;

    const std::string& get_buffer() const;

private:
    std::string buffer;
};
}
#endif