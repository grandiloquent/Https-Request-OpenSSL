#ifndef CPPHTTPLIB_DETAIL_H
#define CPPHTTPLIB_DETAIL_H

#include "httplib.h"

namespace httplib
{

namespace detail
{
#ifdef _WIN32
    class WSInit
    {
    public:
        WSInit()
        {
            WSADATA wsaData;
            WSAStartup(0x0002, &wsaData);
        }

        ~WSInit() { WSACleanup(); }
    };

    static WSInit wsinit_;
#endif
    //======================================//

    template <class Fn>
    void split(const char* b, const char* e, char d, Fn fn)
    {
        int i = 0;
        int beg = 0;

        while (e ? (b + i != e) : (b[i] != '\0'))
        {
            if (b[i] == d)
            {
                fn(&b[beg], &b[i]);
                beg = i + 1;
            }
            i++;
        }

        if (i)
        {
            fn(&b[beg], &b[i]);
        }
    }
    template <typename Fn>
    socket_t create_socket(const char* host, int port, Fn fn,
        int socket_flags = 0);

    //======================================//

    inline bool from_hex_to_i(const std::string& s, size_t i, size_t cnt,
        int& val);
    inline bool has_header(const Headers& headers, const char* key);
    inline bool is_chunked_transfer_encoding(const Headers& headers);
    inline bool is_connection_error();
    inline bool is_dir(const std::string& path);
    inline bool is_file(const std::string& path);
    inline bool is_hex(char c, int& v);
    inline bool is_valid_path(const std::string& path);
    inline bool read_headers(Stream& strm, Headers& headers);
    inline bool wait_until_socket_is_ready(socket_t sock, time_t sec, time_t usec);
    inline const char* find_content_type(const std::string& path);

    inline const char* status_message(int status);
    inline int close_socket(socket_t sock);
    inline int select_read(socket_t sock, time_t sec, time_t usec);
    inline int shutdown_socket(socket_t sock);
    inline size_t to_utf8(int code, char* buff);
    inline std::string decode_url(const std::string& s);
    inline std::string encode_url(const std::string& s);
    inline std::string file_extension(const std::string& path);
    inline std::string from_i_to_hex(uint64_t n);
    inline std::string get_remote_addr(socket_t sock);

    inline void read_file(const std::string& path, std::string& out);
    inline void set_nonblocking(socket_t sock, bool nonblocking);
    inline void skip_content_with_length(Stream& strm, size_t len);
    inline void parse_query_text(const std::string& s, Params& params);

    inline int close_socket(socket_t sock)
    {
#ifdef _WIN32
        return closesocket(sock);
#else
        return close(sock);
#endif
    }

    inline std::string decode_url(const std::string& s)
    {
        std::string result;

        for (size_t i = 0; i < s.size(); i++)
        {
            if (s[i] == '%' && i + 1 < s.size())
            {
                if (s[i + 1] == 'u')
                {
                    int val = 0;
                    if (from_hex_to_i(s, i + 2, 4, val))
                    {
                        // 4 digits Unicode codes
                        char buff[4];
                        size_t len = to_utf8(val, buff);
                        if (len > 0)
                        {
                            result.append(buff, len);
                        }
                        i += 5; // 'u0000'
                    }
                    else
                    {
                        result += s[i];
                    }
                }
                else
                {
                    int val = 0;
                    if (from_hex_to_i(s, i + 1, 2, val))
                    {
                        // 2 digits hex codes
                        result += val;
                        i += 2; // '00'
                    }
                    else
                    {
                        result += s[i];
                    }
                }
            }
            else if (s[i] == '+')
            {
                result += ' ';
            }
            else
            {
                result += s[i];
            }
        }

        return result;
    }

   

    inline bool parse_multipart_boundary(const std::string& content_type,
        std::string& boundary)
    {
        auto pos = content_type.find("boundary=");
        if (pos == std::string::npos)
        {
            return false;
        }

        boundary = content_type.substr(pos + 9);
        return true;
    }

    inline bool parse_multipart_formdata(const std::string& boundary,
        const std::string& body,
        MultipartFiles& files)
    {
        static std::string dash = "--";
        static std::string crlf = "\r\n";

        static std::regex re_content_type("Content-Type: (.*?)",
            std::regex_constants::icase);

        static std::regex re_content_disposition(
            "Content-Disposition: form-data; name=\"(.*?)\"(?:; filename=\"(.*?)\")?",
            std::regex_constants::icase);

        auto dash_boundary = dash + boundary;

        auto pos = body.find(dash_boundary);
        if (pos != 0)
        {
            return false;
        }

        pos += dash_boundary.size();

        auto next_pos = body.find(crlf, pos);
        if (next_pos == std::string::npos)
        {
            return false;
        }

        pos = next_pos + crlf.size();

        while (pos < body.size())
        {
            next_pos = body.find(crlf, pos);
            if (next_pos == std::string::npos)
            {
                return false;
            }

            std::string name;
            MultipartFile file;

            auto header = body.substr(pos, (next_pos - pos));

            while (pos != next_pos)
            {
                std::smatch m;
                if (std::regex_match(header, m, re_content_type))
                {
                    file.content_type = m[1];
                }
                else if (std::regex_match(header, m, re_content_disposition))
                {
                    name = m[1];
                    file.filename = m[2];
                }

                pos = next_pos + crlf.size();

                next_pos = body.find(crlf, pos);
                if (next_pos == std::string::npos)
                {
                    return false;
                }

                header = body.substr(pos, (next_pos - pos));
            }

            pos = next_pos + crlf.size();

            next_pos = body.find(crlf + dash_boundary, pos);

            if (next_pos == std::string::npos)
            {
                return false;
            }

            file.offset = pos;
            file.length = next_pos - pos;

            pos = next_pos + crlf.size() + dash_boundary.size();

            next_pos = body.find(crlf, pos);
            if (next_pos == std::string::npos)
            {
                return false;
            }

            files.emplace(name, file);

            pos = next_pos + crlf.size();
        }

        return true;
    }

    inline std::string to_lower(const char* beg, const char* end)
    {
        std::string out;
        auto it = beg;
        while (it != end)
        {
            out += ::tolower(*it);
            it++;
        }
        return out;
    }

    inline void make_range_header_core(std::string&) {}

    template <typename uint64_t>
    inline void make_range_header_core(std::string& field, uint64_t value)
    {
        if (!field.empty())
        {
            field += ", ";
        }
        field += std::to_string(value) + "-";
    }

    template <typename uint64_t, typename... Args>
    inline void make_range_header_core(std::string& field, uint64_t value1,
        uint64_t value2, Args... args)
    {
        if (!field.empty())
        {
            field += ", ";
        }
        field += std::to_string(value1) + "-" + std::to_string(value2);
        make_range_header_core(field, args...);
    }
    inline std::string encode_url(const std::string& s)
    {
        std::string result;

        for (auto i = 0; s[i]; i++)
        {
            switch (s[i])
            {
            case ' ':
                result += "%20";
                break;
            case '+':
                result += "%2B";
                break;
            case '\r':
                result += "%0D";
                break;
            case '\n':
                result += "%0A";
                break;
            case '\'':
                result += "%27";
                break;
            case ',':
                result += "%2C";
                break;
            case ':':
                result += "%3A";
                break;
            case ';':
                result += "%3B";
                break;
            default:
                auto c = static_cast<uint8_t>(s[i]);
                if (c >= 0x80)
                {
                    result += '%';
                    char hex[4];
                    size_t len = snprintf(hex, sizeof(hex) - 1, "%02X", c);
                    assert(len == 2);
                    result.append(hex, len);
                }
                else
                {
                    result += s[i];
                }
                break;
            }
        }

        return result;
    }

    inline std::string file_extension(const std::string& path)
    {
        std::smatch m;
        auto pat = std::regex("\\.([a-zA-Z0-9]+)$");
        if (std::regex_search(path, m, pat))
        {
            return m[1].str();
        }
        return std::string();
    }

    inline const char* find_content_type(const std::string& path)
    {
        auto ext = file_extension(path);
        if (ext == "txt")
        {
            return "text/plain";
        }
        else if (ext == "html")
        {
            return "text/html";
        }
        else if (ext == "css")
        {
            return "text/css";
        }
        else if (ext == "jpeg" || ext == "jpg")
        {
            return "image/jpg";
        }
        else if (ext == "png")
        {
            return "image/png";
        }
        else if (ext == "gif")
        {
            return "image/gif";
        }
        else if (ext == "svg")
        {
            return "image/svg+xml";
        }
        else if (ext == "ico")
        {
            return "image/x-icon";
        }
        else if (ext == "json")
        {
            return "application/json";
        }
        else if (ext == "pdf")
        {
            return "application/pdf";
        }
        else if (ext == "js")
        {
            return "application/javascript";
        }
        else if (ext == "xml")
        {
            return "application/xml";
        }
        else if (ext == "xhtml")
        {
            return "application/xhtml+xml";
        }
        return nullptr;
    }

    inline bool from_hex_to_i(const std::string& s, size_t i, size_t cnt,
        int& val)
    {
        if (i >= s.size())
        {
            return false;
        }

        val = 0;
        for (; cnt; i++, cnt--)
        {
            if (!s[i])
            {
                return false;
            }
            int v = 0;
            if (is_hex(s[i], v))
            {
                val = val * 16 + v;
            }
            else
            {
                return false;
            }
        }
        return true;
    }

    inline std::string from_i_to_hex(uint64_t n)
    {
        const char* charset = "0123456789abcdef";
        std::string ret;
        do
        {
            ret = charset[n & 15] + ret;
            n >>= 4;
        } while (n > 0);
        return ret;
    }

    inline const char* get_header_value(const Headers& headers, const char* key,
        size_t id = 0, const char* def = nullptr)
    {
        auto it = headers.find(key);
        std::advance(it, id);
        if (it != headers.end())
        {
            return it->second.c_str();
        }
        return def;
    }

    inline uint64_t get_header_value_uint64(const Headers& headers, const char* key,
        int def = 0)
    {
        auto it = headers.find(key);
        if (it != headers.end())
        {
            return std::strtoull(it->second.data(), nullptr, 10);
        }
        return def;
    }

    inline std::string get_remote_addr(socket_t sock)
    {
        struct sockaddr_storage addr;
        socklen_t len = sizeof(addr);

        if (!getpeername(sock, (struct sockaddr*)&addr, &len))
        {
            char ipstr[NI_MAXHOST];

            if (!getnameinfo((struct sockaddr*)&addr, len, ipstr, sizeof(ipstr),
                    nullptr, 0, NI_NUMERICHOST))
            {
                return ipstr;
            }
        }

        return std::string();
    }
    inline bool has_header(const Headers& headers, const char* key)
    {
        return headers.find(key) != headers.end();
    }

    inline bool is_chunked_transfer_encoding(const Headers& headers)
    {
        return !strcasecmp(get_header_value(headers, "Transfer-Encoding", 0, ""),
            "chunked");
    }

    inline bool is_connection_error()
    {
#ifdef _WIN32
        return WSAGetLastError() != WSAEWOULDBLOCK;
#else
        return errno != EINPROGRESS;
#endif
    }

    inline bool is_dir(const std::string& path)
    {
        struct stat st;
        return stat(path.c_str(), &st) >= 0 && S_ISDIR(st.st_mode);
    }

    inline bool is_file(const std::string& path)
    {
        struct stat st;
        return stat(path.c_str(), &st) >= 0 && S_ISREG(st.st_mode);
    }

    inline bool is_hex(char c, int& v)
    {
        if (0x20 <= c && isdigit(c))
        {
            v = c - '0';
            return true;
        }
        else if ('A' <= c && c <= 'F')
        {
            v = c - 'A' + 10;
            return true;
        }
        else if ('a' <= c && c <= 'f')
        {
            v = c - 'a' + 10;
            return true;
        }
        return false;
    }

    inline bool is_valid_path(const std::string& path)
    {
        size_t level = 0;
        size_t i = 0;

        // Skip slash
        while (i < path.size() && path[i] == '/')
        {
            i++;
        }

        while (i < path.size())
        {
            // Read component
            auto beg = i;
            while (i < path.size() && path[i] != '/')
            {
                i++;
            }

            auto len = i - beg;
            assert(len > 0);

            if (!path.compare(beg, len, "."))
            {
                ;
            }
            else if (!path.compare(beg, len, ".."))
            {
                if (level == 0)
                {
                    return false;
                }
                level--;
            }
            else
            {
                level++;
            }

            // Skip slash
            while (i < path.size() && path[i] == '/')
            {
                i++;
            }
        }

        return true;
    }

    inline void parse_query_text(const std::string& s, Params& params)
    {
        split(&s[0], &s[s.size()], '&', [&](const char* b, const char* e) {
            std::string key;
            std::string val;
            split(b, e, '=', [&](const char* b, const char* e) {
                if (key.empty())
                {
                    key.assign(b, e);
                }
                else
                {
                    val.assign(b, e);
                }
            });
            params.emplace(key, decode_url(val));
        });
    }

    template <typename T>
    inline bool read_and_close_socket(socket_t sock, size_t keep_alive_max_count,
        T callback)
    {
        bool ret = false;

        if (keep_alive_max_count > 0)
        {
            auto count = keep_alive_max_count;
            while (count > 0 && detail::select_read(sock, CPPHTTPLIB_KEEPALIVE_TIMEOUT_SECOND, CPPHTTPLIB_KEEPALIVE_TIMEOUT_USECOND) > 0)
            {
                SocketStream strm(sock);
                auto last_connection = count == 1;
                auto connection_close = false;

                ret = callback(strm, last_connection, connection_close);
                if (!ret || connection_close)
                {
                    break;
                }

                count--;
            }
        }
        else
        {
            SocketStream strm(sock);
            auto dummy_connection_close = false;
            ret = callback(strm, true, dummy_connection_close);
        }

        close_socket(sock);
        return ret;
    }

    template <typename T>
    inline bool read_content_chunked(Stream& strm, T callback)
    {
        const auto bufsiz = 16;
        char buf[bufsiz];

        stream_line_reader reader(strm, buf, bufsiz);

        if (!reader.getline())
        {
            return false;
        }

        auto chunk_len = std::stoi(reader.ptr(), 0, 16);

        while (chunk_len > 0)
        {
            if (!read_content_with_length(strm, chunk_len, nullptr, callback))
            {
                return false;
            }

            if (!reader.getline())
            {
                return false;
            }

            if (strcmp(reader.ptr(), "\r\n"))
            {
                break;
            }

            if (!reader.getline())
            {
                return false;
            }

            chunk_len = std::stoi(reader.ptr(), 0, 16);
        }

        if (chunk_len == 0)
        {
            // Reader terminator after chunks
            if (!reader.getline() || strcmp(reader.ptr(), "\r\n"))
                return false;
        }

        return true;
    }

    template <typename T>
    inline bool read_content_with_length(Stream& strm, size_t len,
        Progress progress, T callback)
    {
        char buf[CPPHTTPLIB_RECV_BUFSIZ];

        size_t r = 0;
        while (r < len)
        {
            auto n = strm.read(buf, std::min((len - r), CPPHTTPLIB_RECV_BUFSIZ));
            if (n <= 0)
            {
                return false;
            }

            callback(buf, n);

            r += n;

            if (progress)
            {
                if (!progress(r, len))
                {
                    return false;
                }
            }
        }

        return true;
    }

    template <typename T>
    inline bool read_content_without_length(Stream& strm, T callback)
    {
        char buf[CPPHTTPLIB_RECV_BUFSIZ];
        for (;;)
        {
            auto n = strm.read(buf, CPPHTTPLIB_RECV_BUFSIZ);
            if (n < 0)
            {
                return false;
            }
            else if (n == 0)
            {
                return true;
            }
            callback(buf, n);
        }

        return true;
    }

    inline void read_file(const std::string& path, std::string& out)
    {
        std::ifstream fs(path, std::ios_base::binary);
        fs.seekg(0, std::ios_base::end);
        auto size = fs.tellg();
        fs.seekg(0);
        out.resize(static_cast<size_t>(size));
        fs.read(&out[0], size);
    }

    inline bool read_headers(Stream& strm, Headers& headers)
    {
        static std::regex re(R"((.+?):\s*(.+?)\s*\r\n)");

        const auto bufsiz = 2048;
        char buf[bufsiz];

        stream_line_reader reader(strm, buf, bufsiz);

        for (;;)
        {
            if (!reader.getline())
            {
                return false;
            }
            if (!strcmp(reader.ptr(), "\r\n"))
            {
                break;
            }
            std::cmatch m;
            if (std::regex_match(reader.ptr(), m, re))
            {
                auto key = std::string(m[1]);
                auto val = std::string(m[2]);
                headers.emplace(key, val);
            }
        }

        return true;
    }

    inline int select_read(socket_t sock, time_t sec, time_t usec)
    {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(sock, &fds);

        timeval tv;
        tv.tv_sec = static_cast<long>(sec);
        tv.tv_usec = static_cast<long>(usec);

        return select(static_cast<int>(sock + 1), &fds, nullptr, nullptr, &tv);
    }

    inline void set_nonblocking(socket_t sock, bool nonblocking)
    {
#ifdef _WIN32
        auto flags = nonblocking ? 1UL : 0UL;
        ioctlsocket(sock, FIONBIO, &flags);
#else
        auto flags = fcntl(sock, F_GETFL, 0);
        fcntl(sock, F_SETFL,
            nonblocking ? (flags | O_NONBLOCK) : (flags & (~O_NONBLOCK)));
#endif
    }

    inline int shutdown_socket(socket_t sock)
    {
#ifdef _WIN32
        return shutdown(sock, SD_BOTH);
#else
        return shutdown(sock, SHUT_RDWR);
#endif
    }

    inline void skip_content_with_length(Stream& strm, size_t len)
    {
        char buf[CPPHTTPLIB_RECV_BUFSIZ];
        size_t r = 0;
        while (r < len)
        {
            auto n = strm.read(buf, std::min((len - r), CPPHTTPLIB_RECV_BUFSIZ));
            if (n <= 0)
            {
                return;
            }
            r += n;
        }
    }

    inline const char* status_message(int status)
    {
        switch (status)
        {
        case 200:
            return "OK";
        case 301:
            return "Moved Permanently";
        case 302:
            return "Found";
        case 303:
            return "See Other";
        case 304:
            return "Not Modified";
        case 400:
            return "Bad Request";
        case 403:
            return "Forbidden";
        case 404:
            return "Not Found";
        case 413:
            return "Payload Too Large";
        case 414:
            return "Request-URI Too Long";
        case 415:
            return "Unsupported Media Type";
        default:
        case 500:
            return "Internal Server Error";
        }
    }

    inline size_t to_utf8(int code, char* buff)
    {
        if (code < 0x0080)
        {
            buff[0] = (code & 0x7F);
            return 1;
        }
        else if (code < 0x0800)
        {
            buff[0] = (0xC0 | ((code >> 6) & 0x1F));
            buff[1] = (0x80 | (code & 0x3F));
            return 2;
        }
        else if (code < 0xD800)
        {
            buff[0] = (0xE0 | ((code >> 12) & 0xF));
            buff[1] = (0x80 | ((code >> 6) & 0x3F));
            buff[2] = (0x80 | (code & 0x3F));
            return 3;
        }
        else if (code < 0xE000)
        { // D800 - DFFF is invalid...
            return 0;
        }
        else if (code < 0x10000)
        {
            buff[0] = (0xE0 | ((code >> 12) & 0xF));
            buff[1] = (0x80 | ((code >> 6) & 0x3F));
            buff[2] = (0x80 | (code & 0x3F));
            return 3;
        }
        else if (code < 0x110000)
        {
            buff[0] = (0xF0 | ((code >> 18) & 0x7));
            buff[1] = (0x80 | ((code >> 12) & 0x3F));
            buff[2] = (0x80 | ((code >> 6) & 0x3F));
            buff[3] = (0x80 | (code & 0x3F));
            return 4;
        }

        // NOTREACHED
        return 0;
    }

    inline bool wait_until_socket_is_ready(socket_t sock, time_t sec, time_t usec)
    {
        fd_set fdsr;
        FD_ZERO(&fdsr);
        FD_SET(sock, &fdsr);

        auto fdsw = fdsr;
        auto fdse = fdsr;

        timeval tv;
        tv.tv_sec = static_cast<long>(sec);
        tv.tv_usec = static_cast<long>(usec);

        if (select(static_cast<int>(sock + 1), &fdsr, &fdsw, &fdse, &tv) < 0)
        {
            return false;
        }
        else if (FD_ISSET(sock, &fdsr) || FD_ISSET(sock, &fdsw))
        {
            int error = 0;
            socklen_t len = sizeof(error);
            if (getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&error, &len) < 0 || error)
            {
                return false;
            }
        }
        else
        {
            return false;
        }

        return true;
    }

    template <typename T>
    inline void write_content_chunked(Stream& strm, const T& x)
    {
        auto chunked_response = !x.has_header("Content-Length");
        uint64_t offset = 0;
        auto data_available = true;
        while (data_available)
        {
            auto chunk = x.content_producer(offset);
            offset += chunk.size();
            data_available = !chunk.empty();

            // Emit chunked response header and footer for each chunk
            if (chunked_response)
            {
                chunk = from_i_to_hex(chunk.size()) + "\r\n" + chunk + "\r\n";
            }

            if (strm.write(chunk.c_str(), chunk.size()) < 0)
            {
                break; // Stop on error
            }
        }
    }

    template <typename T>
    inline void write_headers(Stream& strm, const T& info)
    {
        for (const auto& x : info.headers)
        {
            strm.write_format("%s: %s\r\n", x.first.c_str(), x.second.c_str());
        }
        strm.write("\r\n");
    }

     template <typename T, typename U>
    bool read_content(Stream& strm, T& x, uint64_t payload_max_length, int& status,
        Progress progress, U callback)
    {

        ContentReceiver out = [&](const char* buf, size_t n) { callback(buf, n); };

#ifdef CPPHTTPLIB_ZLIB_SUPPORT
        detail::decompressor decompressor;

        if (!decompressor.is_valid())
        {
            status = 500;
            return false;
        }

        if (x.get_header_value("Content-Encoding") == "gzip")
        {
            out = [&](const char* buf, size_t n) {
                decompressor.decompress(
                    buf, n, [&](const char* buf, size_t n) { callback(buf, n); });
            };
        }
#else
        if (x.get_header_value("Content-Encoding") == "gzip")
        {
            status = 415;
            return false;
        }
#endif

        auto ret = true;
        auto exceed_payload_max_length = false;

        if (is_chunked_transfer_encoding(x.headers))
        {
            ret = read_content_chunked(strm, out);
        }
        else if (!has_header(x.headers, "Content-Length"))
        {
            ret = read_content_without_length(strm, out);
        }
        else
        {
            auto len = get_header_value_uint64(x.headers, "Content-Length", 0);
            if (len > 0)
            {
                if ((len > payload_max_length) ||
                    // For 32-bit platform
                    (sizeof(size_t) < sizeof(uint64_t) && len > std::numeric_limits<size_t>::max()))
                {
                    exceed_payload_max_length = true;
                    skip_content_with_length(strm, len);
                    ret = false;
                }
                else
                {
                    ret = read_content_with_length(strm, len, progress, out);
                }
            }
        }

        if (!ret)
        {
            status = exceed_payload_max_length ? 413 : 400;
        }

        return ret;
    }
}
}
#endif