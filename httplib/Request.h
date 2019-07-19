#ifndef CPPHTTPLIB_REQUEST_H
#define CPPHTTPLIB_REQUEST_H
#include "httplib.h"
#include "detail.h"

namespace httplib
{

struct Request
{
    std::string version;
    std::string method;
    std::string target;
    std::string path;
    Headers headers;
    std::string body;
    Params params;
    MultipartFiles files;
    Match matches;

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
    const SSL* ssl;
#endif

    bool has_header(const char* key) const;
    std::string get_header_value(const char* key, size_t id = 0) const;
    size_t get_header_value_count(const char* key) const;
    void set_header(const char* key, const char* val);

    bool has_param(const char* key) const;
    std::string get_param_value(const char* key, size_t id = 0) const;
    size_t get_param_value_count(const char* key) const;

    bool has_file(const char* key) const;
    MultipartFile get_file_value(const char* key) const;
};

inline bool Request::has_header(const char* key) const
{
    return detail::has_header(headers, key);
}

inline std::string Request::get_header_value(const char* key, size_t id) const
{
    return detail::get_header_value(headers, key, id, "");
}

inline size_t Request::get_header_value_count(const char* key) const
{
    auto r = headers.equal_range(key);
    return std::distance(r.first, r.second);
}

inline void Request::set_header(const char* key, const char* val)
{
    headers.emplace(key, val);
}

inline bool Request::has_param(const char* key) const
{
    return params.find(key) != params.end();
}

inline std::string Request::get_param_value(const char* key, size_t id) const
{
    auto it = params.find(key);
    std::advance(it, id);
    if (it != params.end())
    {
        return it->second;
    }
    return std::string();
}

inline size_t Request::get_param_value_count(const char* key) const
{
    auto r = params.equal_range(key);
    return std::distance(r.first, r.second);
}

inline bool Request::has_file(const char* key) const
{
    return files.find(key) != files.end();
}

inline MultipartFile Request::get_file_value(const char* key) const
{
    auto it = files.find(key);
    if (it != files.end())
    {
        return it->second;
    }
    return MultipartFile();
}

// Response implementation
inline bool Response::has_header(const char* key) const
{
    return headers.find(key) != headers.end();
}

inline std::string Response::get_header_value(const char* key,
    size_t id) const
{
    return detail::get_header_value(headers, key, id, "");
}

inline size_t Response::get_header_value_count(const char* key) const
{
    auto r = headers.equal_range(key);
    return std::distance(r.first, r.second);
}

inline void Response::set_header(const char* key, const char* val)
{
    headers.emplace(key, val);
}

inline void Response::set_redirect(const char* url)
{
    set_header("Location", url);
    status = 302;
}

inline void Response::set_content(const char* s, size_t n,
    const char* content_type)
{
    body.assign(s, n);
    set_header("Content-Type", content_type);
}

inline void Response::set_content(const std::string& s,
    const char* content_type)
{
    body = s;
    set_header("Content-Type", content_type);
}
}
#endif