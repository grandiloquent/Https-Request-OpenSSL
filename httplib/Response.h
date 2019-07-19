#ifndef CPPHTTPLIB_RESPONSE_H
#define CPPHTTPLIB_RESPONSE_H
namespace httplib
{
struct Response
{
    std::string version;
    int status;
    Headers headers;
    std::string body;

    ContentProducer content_producer;
    ContentReceiver content_receiver;
    Progress progress;

    bool has_header(const char* key) const;
    std::string get_header_value(const char* key, size_t id = 0) const;
    size_t get_header_value_count(const char* key) const;
    void set_header(const char* key, const char* val);

    void set_redirect(const char* uri);
    void set_content(const char* s, size_t n, const char* content_type);
    void set_content(const std::string& s, const char* content_type);

    Response()
        : status(-1)
    {
    }
};

}
#endif