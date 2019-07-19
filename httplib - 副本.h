//
//  httplib.h
//
//  Copyright (c) 2019 Yuji Hirose. All rights reserved.
//  MIT License
//


namespace httplib {

 // namespace detail

enum class HttpVersion { v1_0 = 0, v1_1 };


template <typename uint64_t, typename... Args>
std::pair<std::string, std::string> make_range_header(uint64_t value,
                                                      Args... args);

typedef std::smatch Match;

typedef std::function<std::string(uint64_t offset)> ContentProducer;
typedef std::function<void(const char *data, size_t len)> ContentReceiver;

struct MultipartFile {
  std::string filename;
  std::string content_type;
  size_t offset = 0;
  size_t length = 0;
};
typedef std::multimap<std::string, MultipartFile> MultipartFiles;

struct Request {
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
  const SSL *ssl;
#endif

  bool has_header(const char *key) const;
  std::string get_header_value(const char *key, size_t id = 0) const;
  size_t get_header_value_count(const char *key) const;
  void set_header(const char *key, const char *val);

  bool has_param(const char *key) const;
  std::string get_param_value(const char *key, size_t id = 0) const;
  size_t get_param_value_count(const char *key) const;

  bool has_file(const char *key) const;
  MultipartFile get_file_value(const char *key) const;
};

struct Response {
  std::string version;
  int status;
  Headers headers;
  std::string body;

  ContentProducer content_producer;
  ContentReceiver content_receiver;
  Progress progress;

  bool has_header(const char *key) const;
  std::string get_header_value(const char *key, size_t id = 0) const;
  size_t get_header_value_count(const char *key) const;
  void set_header(const char *key, const char *val);

  void set_redirect(const char *uri);
  void set_content(const char *s, size_t n, const char *content_type);
  void set_content(const std::string &s, const char *content_type);

  Response() : status(-1) {}
};





class BufferStream : public Stream {
 public:
  BufferStream() {}
  virtual ~BufferStream() {}

  virtual int read(char *ptr, size_t size);
  virtual int write(const char *ptr, size_t size);
  virtual int write(const char *ptr);
  virtual std::string get_remote_addr() const;

  const std::string &get_buffer() const;

 private:
  std::string buffer;
};

class Server {
 public:
  typedef std::function<void(const Request &, Response &)> Handler;
  typedef std::function<void(const Request &, const Response &)> Logger;

  Server();

  virtual ~Server();

  virtual bool is_valid() const;

  Server &Get(const char *pattern, Handler handler);
  Server &Post(const char *pattern, Handler handler);

  Server &Put(const char *pattern, Handler handler);
  Server &Patch(const char *pattern, Handler handler);
  Server &Delete(const char *pattern, Handler handler);
  Server &Options(const char *pattern, Handler handler);

  bool set_base_dir(const char *path);

  void set_error_handler(Handler handler);
  void set_logger(Logger logger);

  void set_keep_alive_max_count(size_t count);
  void set_payload_max_length(uint64_t length);

  int bind_to_any_port(const char *host, int socket_flags = 0);
  bool listen_after_bind();

  bool listen(const char *host, int port, int socket_flags = 0);

  bool is_running() const;
  void stop();

 protected:
  bool process_request(Stream &strm, bool last_connection,
                       bool &connection_close,
                       std::function<void(Request &)> setup_request = nullptr);

  size_t keep_alive_max_count_;
  size_t payload_max_length_;

 private:
  typedef std::vector<std::pair<std::regex, Handler>> Handlers;

  socket_t create_server_socket(const char *host, int port,
                                int socket_flags) const;
  int bind_internal(const char *host, int port, int socket_flags);
  bool listen_internal();

  bool routing(Request &req, Response &res);
  bool handle_file_request(Request &req, Response &res);
  bool dispatch_request(Request &req, Response &res, Handlers &handlers);

  bool parse_request_line(const char *s, Request &req);
  void write_response(Stream &strm, bool last_connection, const Request &req,
                      Response &res);

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

class Client {
 public:
  Client(const char *host, int port = 80, time_t timeout_sec = 300);

  virtual ~Client();

  virtual bool is_valid() const;

  std::shared_ptr<Response> Get(const char *path, Progress progress = nullptr);
  std::shared_ptr<Response> Get(const char *path, const Headers &headers,
                                Progress progress = nullptr);

  std::shared_ptr<Response> Get(const char *path,
                                ContentReceiver content_receiver,
                                Progress progress = nullptr);
  std::shared_ptr<Response> Get(const char *path, const Headers &headers,
                                ContentReceiver content_receiver,
                                Progress progress = nullptr);

  std::shared_ptr<Response> Head(const char *path);
  std::shared_ptr<Response> Head(const char *path, const Headers &headers);

  std::shared_ptr<Response> Post(const char *path, const std::string &body,
                                 const char *content_type);
  std::shared_ptr<Response> Post(const char *path, const Headers &headers,
                                 const std::string &body,
                                 const char *content_type);

  std::shared_ptr<Response> Post(const char *path, const Params &params);
  std::shared_ptr<Response> Post(const char *path, const Headers &headers,
                                 const Params &params);

  std::shared_ptr<Response> Put(const char *path, const std::string &body,
                                const char *content_type);
  std::shared_ptr<Response> Put(const char *path, const Headers &headers,
                                const std::string &body,
                                const char *content_type);

  std::shared_ptr<Response> Patch(const char *path, const std::string &body,
                                  const char *content_type);
  std::shared_ptr<Response> Patch(const char *path, const Headers &headers,
                                  const std::string &body,
                                  const char *content_type);

  std::shared_ptr<Response> Delete(const char *path,
                                   const std::string &body = std::string(),
                                   const char *content_type = nullptr);
  std::shared_ptr<Response> Delete(const char *path, const Headers &headers,
                                   const std::string &body = std::string(),
                                   const char *content_type = nullptr);

  std::shared_ptr<Response> Options(const char *path);
  std::shared_ptr<Response> Options(const char *path, const Headers &headers);

  bool send(Request &req, Response &res);

 protected:
  bool process_request(Stream &strm, Request &req, Response &res,
                       bool &connection_close);

  const std::string host_;
  const int port_;
  time_t timeout_sec_;
  const std::string host_and_port_;

 private:
  socket_t create_client_socket() const;
  bool read_response_line(Stream &strm, Response &res);
  void write_request(Stream &strm, Request &req);

  virtual bool read_and_close_socket(socket_t sock, Request &req,
                                     Response &res);
  virtual bool is_ssl() const;
};

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
class SSLSocketStream : public Stream {
public:
  SSLSocketStream(socket_t sock, SSL *ssl);
  virtual ~SSLSocketStream();

  virtual int read(char *ptr, size_t size);
  virtual int write(const char *ptr, size_t size);
  virtual int write(const char *ptr);
  virtual std::string get_remote_addr() const;

private:
  socket_t sock_;
  SSL *ssl_;
};

class SSLServer : public Server {
public:
  SSLServer(const char *cert_path, const char *private_key_path,
            const char *client_ca_cert_file_path = nullptr,
            const char *client_ca_cert_dir_path = nullptr);

  virtual ~SSLServer();

  virtual bool is_valid() const;

private:
  virtual bool read_and_close_socket(socket_t sock);

  SSL_CTX *ctx_;
  std::mutex ctx_mutex_;
};

class SSLClient : public Client {
public:
  SSLClient(const char *host, int port = 443, time_t timeout_sec = 300,
            const char *client_cert_path = nullptr,
            const char *client_key_path = nullptr);

  virtual ~SSLClient();

  virtual bool is_valid() const;

  void set_ca_cert_path(const char *ca_ceert_file_path,
                        const char *ca_cert_dir_path = nullptr);
  void enable_server_certificate_verification(bool enabled);

  long get_openssl_verify_result() const;

private:
  virtual bool read_and_close_socket(socket_t sock, Request &req,
                                     Response &res);
  virtual bool is_ssl() const;

  bool verify_host(X509 *server_cert) const;
  bool verify_host_with_subject_alt_name(X509 *server_cert) const;
  bool verify_host_with_common_name(X509 *server_cert) const;
  bool check_host_name(const char *pattern, size_t pattern_len) const;

  SSL_CTX *ctx_;
  std::mutex ctx_mutex_;
  std::vector<std::string> host_components_;
  std::string ca_cert_file_path_;
  std::string ca_cert_dir_path_;
  bool server_certificate_verification_ = false;
  long verify_result_ = 0;
};
#endif

/*
 * Implementation
 */
namespace detail {








// NOTE: until the read size reaches `fixed_buffer_size`, use `fixed_buffer`
// to store data. The call can set memory on stack for performance.


















#ifdef CPPHTTPLIB_ZLIB_SUPPORT
inline bool can_compress(const std::string &content_type) {
  return !content_type.find("text/") || content_type == "image/svg+xml" ||
         content_type == "application/javascript" ||
         content_type == "application/json" ||
         content_type == "application/xml" ||
         content_type == "application/xhtml+xml";
}

inline bool compress(std::string &content) {
  z_stream strm;
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;

  auto ret = deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 31, 8,
                          Z_DEFAULT_STRATEGY);
  if (ret != Z_OK) { return false; }

  strm.avail_in = content.size();
  strm.next_in = (Bytef *)content.data();

  std::string compressed;

  const auto bufsiz = 16384;
  char buff[bufsiz];
  do {
    strm.avail_out = bufsiz;
    strm.next_out = (Bytef *)buff;
    ret = deflate(&strm, Z_FINISH);
    assert(ret != Z_STREAM_ERROR);
    compressed.append(buff, bufsiz - strm.avail_out);
  } while (strm.avail_out == 0);

  assert(ret == Z_STREAM_END);
  assert(strm.avail_in == 0);

  content.swap(compressed);

  deflateEnd(&strm);
  return true;
}

class decompressor {
public:
  decompressor() {
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    // 15 is the value of wbits, which should be at the maximum possible value
    // to ensure that any gzip stream can be decoded. The offset of 16 specifies
    // that the stream to decompress will be formatted with a gzip wrapper.
    is_valid_ = inflateInit2(&strm, 16 + 15) == Z_OK;
  }

  ~decompressor() { inflateEnd(&strm); }

  bool is_valid() const { return is_valid_; }

  template <typename T>
  bool decompress(const char *data, size_t data_len, T callback) {
    int ret = Z_OK;
    std::string decompressed;

    // strm.avail_in = content.size();
    // strm.next_in = (Bytef *)content.data();
    strm.avail_in = data_len;
    strm.next_in = (Bytef *)data;

    const auto bufsiz = 16384;
    char buff[bufsiz];
    do {
      strm.avail_out = bufsiz;
      strm.next_out = (Bytef *)buff;

      ret = inflate(&strm, Z_NO_FLUSH);
      assert(ret != Z_STREAM_ERROR);
      switch (ret) {
      case Z_NEED_DICT:
      case Z_DATA_ERROR:
      case Z_MEM_ERROR: inflateEnd(&strm); return false;
      }

      decompressed.append(buff, bufsiz - strm.avail_out);
    } while (strm.avail_out == 0);

    if (ret == Z_STREAM_END) {
      callback(decompressed.data(), decompressed.size());
      return true;
    }

    return false;
  }

private:
  bool is_valid_;
  z_stream strm;
};
#endif













template <typename T, typename U>
bool read_content(Stream &strm, T &x, uint64_t payload_max_length, int &status,
                  Progress progress, U callback) {

  ContentReceiver out = [&](const char *buf, size_t n) { callback(buf, n); };

#ifdef CPPHTTPLIB_ZLIB_SUPPORT
  detail::decompressor decompressor;

  if (!decompressor.is_valid()) {
    status = 500;
    return false;
  }

  if (x.get_header_value("Content-Encoding") == "gzip") {
    out = [&](const char *buf, size_t n) {
      decompressor.decompress(
          buf, n, [&](const char *buf, size_t n) { callback(buf, n); });
    };
  }
#else
  if (x.get_header_value("Content-Encoding") == "gzip") {
    status = 415;
    return false;
  }
#endif

  auto ret = true;
  auto exceed_payload_max_length = false;

  if (is_chunked_transfer_encoding(x.headers)) {
    ret = read_content_chunked(strm, out);
  } else if (!has_header(x.headers, "Content-Length")) {
    ret = read_content_without_length(strm, out);
  } else {
    auto len = get_header_value_uint64(x.headers, "Content-Length", 0);
    if (len > 0) {
      if ((len > payload_max_length) ||
          // For 32-bit platform
          (sizeof(size_t) < sizeof(uint64_t) &&
              len > std::numeric_limits<size_t>::max())) {
        exceed_payload_max_length = true;
        skip_content_with_length(strm, len);
        ret = false;
      } else {
        ret = read_content_with_length(strm, len, progress, out);
      }
    }
  }

  if (!ret) { status = exceed_payload_max_length ? 413 : 400; }

  return ret;
}







inline bool parse_multipart_boundary(const std::string &content_type,
                                     std::string &boundary) {
  auto pos = content_type.find("boundary=");
  if (pos == std::string::npos) { return false; }

  boundary = content_type.substr(pos + 9);
  return true;
}

inline bool parse_multipart_formdata(const std::string &boundary,
                                     const std::string &body,
                                     MultipartFiles &files) {
  static std::string dash = "--";
  static std::string crlf = "\r\n";

  static std::regex re_content_type("Content-Type: (.*?)",
                                    std::regex_constants::icase);

  static std::regex re_content_disposition(
      "Content-Disposition: form-data; name=\"(.*?)\"(?:; filename=\"(.*?)\")?",
      std::regex_constants::icase);

  auto dash_boundary = dash + boundary;

  auto pos = body.find(dash_boundary);
  if (pos != 0) { return false; }

  pos += dash_boundary.size();

  auto next_pos = body.find(crlf, pos);
  if (next_pos == std::string::npos) { return false; }

  pos = next_pos + crlf.size();

  while (pos < body.size()) {
    next_pos = body.find(crlf, pos);
    if (next_pos == std::string::npos) { return false; }

    std::string name;
    MultipartFile file;

    auto header = body.substr(pos, (next_pos - pos));

    while (pos != next_pos) {
      std::smatch m;
      if (std::regex_match(header, m, re_content_type)) {
        file.content_type = m[1];
      } else if (std::regex_match(header, m, re_content_disposition)) {
        name = m[1];
        file.filename = m[2];
      }

      pos = next_pos + crlf.size();

      next_pos = body.find(crlf, pos);
      if (next_pos == std::string::npos) { return false; }

      header = body.substr(pos, (next_pos - pos));
    }

    pos = next_pos + crlf.size();

    next_pos = body.find(crlf + dash_boundary, pos);

    if (next_pos == std::string::npos) { return false; }

    file.offset = pos;
    file.length = next_pos - pos;

    pos = next_pos + crlf.size() + dash_boundary.size();

    next_pos = body.find(crlf, pos);
    if (next_pos == std::string::npos) { return false; }

    files.emplace(name, file);

    pos = next_pos + crlf.size();
  }

  return true;
}

inline std::string to_lower(const char *beg, const char *end) {
  std::string out;
  auto it = beg;
  while (it != end) {
    out += ::tolower(*it);
    it++;
  }
  return out;
}

inline void make_range_header_core(std::string &) {}

template <typename uint64_t>
inline void make_range_header_core(std::string &field, uint64_t value) {
  if (!field.empty()) { field += ", "; }
  field += std::to_string(value) + "-";
}

template <typename uint64_t, typename... Args>
inline void make_range_header_core(std::string &field, uint64_t value1,
                                   uint64_t value2, Args... args) {
  if (!field.empty()) { field += ", "; }
  field += std::to_string(value1) + "-" + std::to_string(value2);
  make_range_header_core(field, args...);
}



} // namespace detail

// Header utilities
template <typename uint64_t, typename... Args>
inline std::pair<std::string, std::string> make_range_header(uint64_t value,
                                                             Args... args) {
  std::string field;
  detail::make_range_header_core(field, value, args...);
  field.insert(0, "bytes=");
  return std::make_pair("Range", field);
}

// Request implementation
inline bool Request::has_header(const char *key) const {
  return detail::has_header(headers, key);
}

inline std::string Request::get_header_value(const char *key, size_t id) const {
  return detail::get_header_value(headers, key, id, "");
}

inline size_t Request::get_header_value_count(const char *key) const {
  auto r = headers.equal_range(key);
  return std::distance(r.first, r.second);
}

inline void Request::set_header(const char *key, const char *val) {
  headers.emplace(key, val);
}

inline bool Request::has_param(const char *key) const {
  return params.find(key) != params.end();
}

inline std::string Request::get_param_value(const char *key, size_t id) const {
  auto it = params.find(key);
  std::advance(it, id);
  if (it != params.end()) { return it->second; }
  return std::string();
}

inline size_t Request::get_param_value_count(const char *key) const {
  auto r = params.equal_range(key);
  return std::distance(r.first, r.second);
}

inline bool Request::has_file(const char *key) const {
  return files.find(key) != files.end();
}

inline MultipartFile Request::get_file_value(const char *key) const {
  auto it = files.find(key);
  if (it != files.end()) { return it->second; }
  return MultipartFile();
}

// Response implementation
inline bool Response::has_header(const char *key) const {
  return headers.find(key) != headers.end();
}

inline std::string Response::get_header_value(const char *key,
                                              size_t id) const {
  return detail::get_header_value(headers, key, id, "");
}

inline size_t Response::get_header_value_count(const char *key) const {
  auto r = headers.equal_range(key);
  return std::distance(r.first, r.second);
}

inline void Response::set_header(const char *key, const char *val) {
  headers.emplace(key, val);
}

inline void Response::set_redirect(const char *url) {
  set_header("Location", url);
  status = 302;
}

inline void Response::set_content(const char *s, size_t n,
                                  const char *content_type) {
  body.assign(s, n);
  set_header("Content-Type", content_type);
}

inline void Response::set_content(const std::string &s,
                                  const char *content_type) {
  body = s;
  set_header("Content-Type", content_type);
}

// Rstream implementation
template <typename... Args>
inline void Stream::write_format(const char *fmt, const Args &... args) {
  const auto bufsiz = 2048;
  char buf[bufsiz];

#if defined(_MSC_VER) && _MSC_VER < 1900
  auto n = _snprintf_s(buf, bufsiz, bufsiz - 1, fmt, args...);
#else
  auto n = snprintf(buf, bufsiz - 1, fmt, args...);
#endif
  if (n > 0) {
    if (n >= bufsiz - 1) {
      std::vector<char> glowable_buf(bufsiz);

      while (n >= static_cast<int>(glowable_buf.size() - 1)) {
        glowable_buf.resize(glowable_buf.size() * 2);
#if defined(_MSC_VER) && _MSC_VER < 1900
        n = _snprintf_s(&glowable_buf[0], glowable_buf.size(),
                        glowable_buf.size() - 1, fmt, args...);
#else
        n = snprintf(&glowable_buf[0], glowable_buf.size() - 1, fmt, args...);
#endif
      }
      write(&glowable_buf[0], n);
    } else {
      write(buf, n);
    }
  }
}

// Socket stream implementation
inline SocketStream::SocketStream(socket_t sock) : sock_(sock) {}

inline SocketStream::~SocketStream() {}

inline int SocketStream::read(char *ptr, size_t size) {
  if (detail::select_read(sock_, CPPHTTPLIB_READ_TIMEOUT_SECOND,
                          CPPHTTPLIB_READ_TIMEOUT_USECOND) > 0) {
    return recv(sock_, ptr, static_cast<int>(size), 0);
  }
  return -1;
}

inline int SocketStream::write(const char *ptr, size_t size) {
  return send(sock_, ptr, static_cast<int>(size), 0);
}

inline int SocketStream::write(const char *ptr) {
  return write(ptr, strlen(ptr));
}

inline std::string SocketStream::get_remote_addr() const {
  return detail::get_remote_addr(sock_);
}

// Buffer stream implementation
inline int BufferStream::read(char *ptr, size_t size) {
#if defined(_MSC_VER) && _MSC_VER < 1900
  return static_cast<int>(buffer._Copy_s(ptr, size, size));
#else
  return static_cast<int>(buffer.copy(ptr, size));
#endif
}

inline int BufferStream::write(const char *ptr, size_t size) {
  buffer.append(ptr, size);
  return static_cast<int>(size);
}

inline int BufferStream::write(const char *ptr) {
  size_t size = strlen(ptr);
  buffer.append(ptr, size);
  return static_cast<int>(size);
}

inline std::string BufferStream::get_remote_addr() const { return ""; }

inline const std::string &BufferStream::get_buffer() const { return buffer; }

// HTTP server implementation
inline Server::Server()
    : keep_alive_max_count_(CPPHTTPLIB_KEEPALIVE_MAX_COUNT),
      payload_max_length_(CPPHTTPLIB_PAYLOAD_MAX_LENGTH), is_running_(false),
      svr_sock_(INVALID_SOCKET), running_threads_(0) {
#ifndef _WIN32
  signal(SIGPIPE, SIG_IGN);
#endif
}

inline Server::~Server() {}

inline Server &Server::Get(const char *pattern, Handler handler) {
  get_handlers_.push_back(std::make_pair(std::regex(pattern), handler));
  return *this;
}

inline Server &Server::Post(const char *pattern, Handler handler) {
  post_handlers_.push_back(std::make_pair(std::regex(pattern), handler));
  return *this;
}

inline Server &Server::Put(const char *pattern, Handler handler) {
  put_handlers_.push_back(std::make_pair(std::regex(pattern), handler));
  return *this;
}

inline Server &Server::Patch(const char *pattern, Handler handler) {
  patch_handlers_.push_back(std::make_pair(std::regex(pattern), handler));
  return *this;
}

inline Server &Server::Delete(const char *pattern, Handler handler) {
  delete_handlers_.push_back(std::make_pair(std::regex(pattern), handler));
  return *this;
}

inline Server &Server::Options(const char *pattern, Handler handler) {
  options_handlers_.push_back(std::make_pair(std::regex(pattern), handler));
  return *this;
}

inline bool Server::set_base_dir(const char *path) {
  if (detail::is_dir(path)) {
    base_dir_ = path;
    return true;
  }
  return false;
}

inline void Server::set_error_handler(Handler handler) {
  error_handler_ = handler;
}

inline void Server::set_logger(Logger logger) { logger_ = logger; }

inline void Server::set_keep_alive_max_count(size_t count) {
  keep_alive_max_count_ = count;
}

inline void Server::set_payload_max_length(uint64_t length) {
  payload_max_length_ = length;
}

inline int Server::bind_to_any_port(const char *host, int socket_flags) {
  return bind_internal(host, 0, socket_flags);
}

inline bool Server::listen_after_bind() { return listen_internal(); }

inline bool Server::listen(const char *host, int port, int socket_flags) {
  if (bind_internal(host, port, socket_flags) < 0) return false;
  return listen_internal();
}

inline bool Server::is_running() const { return is_running_; }

inline void Server::stop() {
  if (is_running_) {
    assert(svr_sock_ != INVALID_SOCKET);
    std::atomic<socket_t> sock(svr_sock_.exchange(INVALID_SOCKET));
    detail::shutdown_socket(sock);
    detail::close_socket(sock);
  }
}

inline bool Server::parse_request_line(const char *s, Request &req) {
  static std::regex re("(GET|HEAD|POST|PUT|PATCH|DELETE|OPTIONS) "
                       "(([^?]+)(?:\\?(.+?))?) (HTTP/1\\.[01])\r\n");

  std::cmatch m;
  if (std::regex_match(s, m, re)) {
    req.version = std::string(m[5]);
    req.method = std::string(m[1]);
    req.target = std::string(m[2]);
    req.path = detail::decode_url(m[3]);

    // Parse query text
    auto len = std::distance(m[4].first, m[4].second);
    if (len > 0) { detail::parse_query_text(m[4], req.params); }

    return true;
  }

  return false;
}

inline void Server::write_response(Stream &strm, bool last_connection,
                                   const Request &req, Response &res) {
  assert(res.status != -1);

  if (400 <= res.status && error_handler_) { error_handler_(req, res); }

  // Response line
  strm.write_format("HTTP/1.1 %d %s\r\n", res.status,
                    detail::status_message(res.status));

  // Headers
  if (last_connection || req.get_header_value("Connection") == "close") {
    res.set_header("Connection", "close");
  }

  if (!last_connection && req.get_header_value("Connection") == "Keep-Alive") {
    res.set_header("Connection", "Keep-Alive");
  }

  if (res.body.empty()) {
    if (!res.has_header("Content-Length")) {
      if (res.content_producer) {
        // Streamed response
        res.set_header("Transfer-Encoding", "chunked");
      } else {
        res.set_header("Content-Length", "0");
      }
    }
  } else {
#ifdef CPPHTTPLIB_ZLIB_SUPPORT
    // TODO: 'Accpet-Encoding' has gzip, not gzip;q=0
    const auto &encodings = req.get_header_value("Accept-Encoding");
    if (encodings.find("gzip") != std::string::npos &&
        detail::can_compress(res.get_header_value("Content-Type"))) {
      if (detail::compress(res.body)) {
        res.set_header("Content-Encoding", "gzip");
      }
    }
#endif

    if (!res.has_header("Content-Type")) {
      res.set_header("Content-Type", "text/plain");
    }

    auto length = std::to_string(res.body.size());
    res.set_header("Content-Length", length.c_str());
  }

  detail::write_headers(strm, res);

  // Body
  if (req.method != "HEAD") {
    if (!res.body.empty()) {
      strm.write(res.body.c_str(), res.body.size());
    } else if (res.content_producer) {
      detail::write_content_chunked(strm, res);
    }
  }

  // Log
  if (logger_) { logger_(req, res); }
}

inline bool Server::handle_file_request(Request &req, Response &res) {
  if (!base_dir_.empty() && detail::is_valid_path(req.path)) {
    std::string path = base_dir_ + req.path;

    if (!path.empty() && path.back() == '/') { path += "index.html"; }

    if (detail::is_file(path)) {
      detail::read_file(path, res.body);
      auto type = detail::find_content_type(path);
      if (type) { res.set_header("Content-Type", type); }
      res.status = 200;
      return true;
    }
  }

  return false;
}

inline socket_t Server::create_server_socket(const char *host, int port,
                                             int socket_flags) const {
  return detail::create_socket(
      host, port,
      [](socket_t sock, struct addrinfo &ai) -> bool {
        if (::bind(sock, ai.ai_addr, static_cast<int>(ai.ai_addrlen))) {
          return false;
        }
        if (::listen(sock, 5)) { // Listen through 5 channels
          return false;
        }
        return true;
      },
      socket_flags);
}

inline int Server::bind_internal(const char *host, int port, int socket_flags) {
  if (!is_valid()) { return -1; }

  svr_sock_ = create_server_socket(host, port, socket_flags);
  if (svr_sock_ == INVALID_SOCKET) { return -1; }

  if (port == 0) {
    struct sockaddr_storage address;
    socklen_t len = sizeof(address);
    if (getsockname(svr_sock_, reinterpret_cast<struct sockaddr *>(&address),
                    &len) == -1) {
      return -1;
    }
    if (address.ss_family == AF_INET) {
      return ntohs(reinterpret_cast<struct sockaddr_in *>(&address)->sin_port);
    } else if (address.ss_family == AF_INET6) {
      return ntohs(
          reinterpret_cast<struct sockaddr_in6 *>(&address)->sin6_port);
    } else {
      return -1;
    }
  } else {
    return port;
  }
}

inline bool Server::listen_internal() {
  auto ret = true;

  is_running_ = true;

  for (;;) {
    if (svr_sock_ == INVALID_SOCKET) {
      // The server socket was closed by 'stop' method.
      break;
    }

    auto val = detail::select_read(svr_sock_, 0, 100000);

    if (val == 0) { // Timeout
      continue;
    }

    socket_t sock = accept(svr_sock_, nullptr, nullptr);

    if (sock == INVALID_SOCKET) {
      if (svr_sock_ != INVALID_SOCKET) {
        detail::close_socket(svr_sock_);
        ret = false;
      } else {
        ; // The server socket was closed by user.
      }
      break;
    }

    // TODO: Use thread pool...
    std::thread([=]() {
      {
        std::lock_guard<std::mutex> guard(running_threads_mutex_);
        running_threads_++;
      }

      read_and_close_socket(sock);

      {
        std::lock_guard<std::mutex> guard(running_threads_mutex_);
        running_threads_--;
      }
    }).detach();
  }

  // TODO: Use thread pool...
  for (;;) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::lock_guard<std::mutex> guard(running_threads_mutex_);
    if (!running_threads_) { break; }
  }

  is_running_ = false;

  return ret;
}

inline bool Server::routing(Request &req, Response &res) {
  if (req.method == "GET" && handle_file_request(req, res)) { return true; }

  if (req.method == "GET" || req.method == "HEAD") {
    return dispatch_request(req, res, get_handlers_);
  } else if (req.method == "POST") {
    return dispatch_request(req, res, post_handlers_);
  } else if (req.method == "PUT") {
    return dispatch_request(req, res, put_handlers_);
  } else if (req.method == "PATCH") {
    return dispatch_request(req, res, patch_handlers_);
  } else if (req.method == "DELETE") {
    return dispatch_request(req, res, delete_handlers_);
  } else if (req.method == "OPTIONS") {
    return dispatch_request(req, res, options_handlers_);
  }
  return false;
}

inline bool Server::dispatch_request(Request &req, Response &res,
                                     Handlers &handlers) {
  for (const auto &x : handlers) {
    const auto &pattern = x.first;
    const auto &handler = x.second;

    if (std::regex_match(req.path, req.matches, pattern)) {
      handler(req, res);
      return true;
    }
  }
  return false;
}

inline bool
Server::process_request(Stream &strm, bool last_connection,
                        bool &connection_close,
                        std::function<void(Request &)> setup_request) {
  const auto bufsiz = 2048;
  char buf[bufsiz];

  detail::stream_line_reader reader(strm, buf, bufsiz);

  // Connection has been closed on client
  if (!reader.getline()) { return false; }

  Request req;
  Response res;

  res.version = "HTTP/1.1";

  // Check if the request URI doesn't exceed the limit
  if (reader.size() > CPPHTTPLIB_REQUEST_URI_MAX_LENGTH) {
    res.status = 414;
    write_response(strm, last_connection, req, res);
    return true;
  }

  // Request line and headers
  if (!parse_request_line(reader.ptr(), req) ||
      !detail::read_headers(strm, req.headers)) {
    res.status = 400;
    write_response(strm, last_connection, req, res);
    return true;
  }

  if (req.get_header_value("Connection") == "close") {
    connection_close = true;
  }

  req.set_header("REMOTE_ADDR", strm.get_remote_addr().c_str());

  // Body
  if (req.method == "POST" || req.method == "PUT" || req.method == "PATCH") {
    if (!detail::read_content(
        strm, req, payload_max_length_, res.status, Progress(),
        [&](const char *buf, size_t n) { req.body.append(buf, n); })) {
      write_response(strm, last_connection, req, res);
      return true;
    }

    const auto &content_type = req.get_header_value("Content-Type");

    if (!content_type.find("application/x-www-form-urlencoded")) {
      detail::parse_query_text(req.body, req.params);
    } else if (!content_type.find("multipart/form-data")) {
      std::string boundary;
      if (!detail::parse_multipart_boundary(content_type, boundary) ||
          !detail::parse_multipart_formdata(boundary, req.body, req.files)) {
        res.status = 400;
        write_response(strm, last_connection, req, res);
        return true;
      }
    }
  }

  // TODO: Add additional request info
  if (setup_request) { setup_request(req); }

  if (routing(req, res)) {
    if (res.status == -1) { res.status = 200; }
  } else {
    res.status = 404;
  }

  write_response(strm, last_connection, req, res);
  return true;
}

inline bool Server::is_valid() const { return true; }

inline bool Server::read_and_close_socket(socket_t sock) {
  return detail::read_and_close_socket(
      sock, keep_alive_max_count_,
      [this](Stream &strm, bool last_connection, bool &connection_close) {
        return process_request(strm, last_connection, connection_close);
      });
}

// HTTP client implementation
inline Client::Client(const char *host, int port, time_t timeout_sec)
    : host_(host), port_(port), timeout_sec_(timeout_sec),
      host_and_port_(host_ + ":" + std::to_string(port_)) {}

inline Client::~Client() {}

inline bool Client::is_valid() const { return true; }

inline socket_t Client::create_client_socket() const {
  return detail::create_socket(
      host_.c_str(), port_, [=](socket_t sock, struct addrinfo &ai) -> bool {
        detail::set_nonblocking(sock, true);

        auto ret = connect(sock, ai.ai_addr, static_cast<int>(ai.ai_addrlen));
        if (ret < 0) {
          if (detail::is_connection_error() ||
              !detail::wait_until_socket_is_ready(sock, timeout_sec_, 0)) {
            detail::close_socket(sock);
            return false;
          }
        }

        detail::set_nonblocking(sock, false);
        return true;
      });
}

inline bool Client::read_response_line(Stream &strm, Response &res) {
  const auto bufsiz = 2048;
  char buf[bufsiz];

  detail::stream_line_reader reader(strm, buf, bufsiz);

  if (!reader.getline()) { return false; }

  const static std::regex re("(HTTP/1\\.[01]) (\\d+?) .*\r\n");

  std::cmatch m;
  if (std::regex_match(reader.ptr(), m, re)) {
    res.version = std::string(m[1]);
    res.status = std::stoi(std::string(m[2]));
  }

  return true;
}

inline bool Client::send(Request &req, Response &res) {
  if (req.path.empty()) { return false; }

  auto sock = create_client_socket();
  if (sock == INVALID_SOCKET) { return false; }

  return read_and_close_socket(sock, req, res);
}

inline void Client::write_request(Stream &strm, Request &req) {
  BufferStream bstrm;

  // Request line
  auto path = detail::encode_url(req.path);

  bstrm.write_format("%s %s HTTP/1.1\r\n", req.method.c_str(), path.c_str());

  // Headers
  if (!req.has_header("Host")) {
    if (is_ssl()) {
      if (port_ == 443) {
        req.set_header("Host", host_.c_str());
      } else {
        req.set_header("Host", host_and_port_.c_str());
      }
    } else {
      if (port_ == 80) {
        req.set_header("Host", host_.c_str());
      } else {
        req.set_header("Host", host_and_port_.c_str());
      }
    }
  }

  if (!req.has_header("Accept")) { req.set_header("Accept", "*/*"); }

  if (!req.has_header("User-Agent")) {
    req.set_header("User-Agent", "cpp-httplib/0.2");
  }

  // TODO: Support KeepAlive connection
  // if (!req.has_header("Connection")) {
  req.set_header("Connection", "close");
  // }

  if (req.body.empty()) {
    if (req.method == "POST" || req.method == "PUT" || req.method == "PATCH") {
      req.set_header("Content-Length", "0");
    }
  } else {
    if (!req.has_header("Content-Type")) {
      req.set_header("Content-Type", "text/plain");
    }

    if (!req.has_header("Content-Length")) {
      auto length = std::to_string(req.body.size());
      req.set_header("Content-Length", length.c_str());
    }
  }

  detail::write_headers(bstrm, req);

  // Body
  if (!req.body.empty()) { bstrm.write(req.body.c_str(), req.body.size()); }

  // Flush buffer
  auto &data = bstrm.get_buffer();
  strm.write(data.data(), data.size());
}

inline bool Client::process_request(Stream &strm, Request &req, Response &res,
                                    bool &connection_close) {
  // Send request
  write_request(strm, req);

  // Receive response and headers
  if (!read_response_line(strm, res) ||
      !detail::read_headers(strm, res.headers)) {
    return false;
  }

  if (res.get_header_value("Connection") == "close" ||
      res.version == "HTTP/1.0") {
    connection_close = true;
  }

  // Body
  if (req.method != "HEAD") {
    ContentReceiver out = [&](const char *buf, size_t n) {
      res.body.append(buf, n);
    };

    if (res.content_receiver) {
      out = [&](const char *buf, size_t n) { res.content_receiver(buf, n); };
    }

    int dummy_status;
    if (!detail::read_content(strm, res, std::numeric_limits<uint64_t>::max(),
                              dummy_status, res.progress, out)) {
      return false;
    }
  }

  return true;
}

inline bool Client::read_and_close_socket(socket_t sock, Request &req,
                                          Response &res) {
  return detail::read_and_close_socket(
      sock, 0,
      [&](Stream &strm, bool /*last_connection*/, bool &connection_close) {
        return process_request(strm, req, res, connection_close);
      });
}

inline bool Client::is_ssl() const { return false; }

inline std::shared_ptr<Response> Client::Get(const char *path,
                                             Progress progress) {
  return Get(path, Headers(), progress);
}

inline std::shared_ptr<Response>
Client::Get(const char *path, const Headers &headers, Progress progress) {
  Request req;
  req.method = "GET";
  req.path = path;
  req.headers = headers;

  auto res = std::make_shared<Response>();
  res->progress = progress;

  return send(req, *res) ? res : nullptr;
}

inline std::shared_ptr<Response> Client::Get(const char *path,
                                             ContentReceiver content_receiver,
                                             Progress progress) {
  return Get(path, Headers(), content_receiver, progress);
}

inline std::shared_ptr<Response> Client::Get(const char *path,
                                             const Headers &headers,
                                             ContentReceiver content_receiver,
                                             Progress progress) {
  Request req;
  req.method = "GET";
  req.path = path;
  req.headers = headers;

  auto res = std::make_shared<Response>();
  res->content_receiver = content_receiver;
  res->progress = progress;

  return send(req, *res) ? res : nullptr;
}

inline std::shared_ptr<Response> Client::Head(const char *path) {
  return Head(path, Headers());
}

inline std::shared_ptr<Response> Client::Head(const char *path,
                                              const Headers &headers) {
  Request req;
  req.method = "HEAD";
  req.headers = headers;
  req.path = path;

  auto res = std::make_shared<Response>();

  return send(req, *res) ? res : nullptr;
}

inline std::shared_ptr<Response> Client::Post(const char *path,
                                              const std::string &body,
                                              const char *content_type) {
  return Post(path, Headers(), body, content_type);
}

inline std::shared_ptr<Response> Client::Post(const char *path,
                                              const Headers &headers,
                                              const std::string &body,
                                              const char *content_type) {
  Request req;
  req.method = "POST";
  req.headers = headers;
  req.path = path;

  req.headers.emplace("Content-Type", content_type);
  req.body = body;

  auto res = std::make_shared<Response>();

  return send(req, *res) ? res : nullptr;
}

inline std::shared_ptr<Response> Client::Post(const char *path,
                                              const Params &params) {
  return Post(path, Headers(), params);
}

inline std::shared_ptr<Response>
Client::Post(const char *path, const Headers &headers, const Params &params) {
  std::string query;
  for (auto it = params.begin(); it != params.end(); ++it) {
    if (it != params.begin()) { query += "&"; }
    query += it->first;
    query += "=";
    query += detail::encode_url(it->second);
  }

  return Post(path, headers, query, "application/x-www-form-urlencoded");
}

inline std::shared_ptr<Response> Client::Put(const char *path,
                                             const std::string &body,
                                             const char *content_type) {
  return Put(path, Headers(), body, content_type);
}

inline std::shared_ptr<Response> Client::Put(const char *path,
                                             const Headers &headers,
                                             const std::string &body,
                                             const char *content_type) {
  Request req;
  req.method = "PUT";
  req.headers = headers;
  req.path = path;

  req.headers.emplace("Content-Type", content_type);
  req.body = body;

  auto res = std::make_shared<Response>();

  return send(req, *res) ? res : nullptr;
}

inline std::shared_ptr<Response> Client::Patch(const char *path,
                                               const std::string &body,
                                               const char *content_type) {
  return Patch(path, Headers(), body, content_type);
}

inline std::shared_ptr<Response> Client::Patch(const char *path,
                                               const Headers &headers,
                                               const std::string &body,
                                               const char *content_type) {
  Request req;
  req.method = "PATCH";
  req.headers = headers;
  req.path = path;

  req.headers.emplace("Content-Type", content_type);
  req.body = body;

  auto res = std::make_shared<Response>();

  return send(req, *res) ? res : nullptr;
}

inline std::shared_ptr<Response> Client::Delete(const char *path,
                                                const std::string &body,
                                                const char *content_type) {
  return Delete(path, Headers(), body, content_type);
}

inline std::shared_ptr<Response> Client::Delete(const char *path,
                                                const Headers &headers,
                                                const std::string &body,
                                                const char *content_type) {
  Request req;
  req.method = "DELETE";
  req.headers = headers;
  req.path = path;

  if (content_type) { req.headers.emplace("Content-Type", content_type); }
  req.body = body;

  auto res = std::make_shared<Response>();

  return send(req, *res) ? res : nullptr;
}

inline std::shared_ptr<Response> Client::Options(const char *path) {
  return Options(path, Headers());
}

inline std::shared_ptr<Response> Client::Options(const char *path,
                                                 const Headers &headers) {
  Request req;
  req.method = "OPTIONS";
  req.path = path;
  req.headers = headers;

  auto res = std::make_shared<Response>();

  return send(req, *res) ? res : nullptr;
}

/*
 * SSL Implementation
 */
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
namespace detail {

template <typename U, typename V, typename T>
inline bool
read_and_close_socket_ssl(socket_t sock, size_t keep_alive_max_count,
                          // TODO: OpenSSL 1.0.2 occasionally crashes...
                          // The upcoming 1.1.0 is going to be thread safe.
                          SSL_CTX *ctx, std::mutex &ctx_mutex,
                          U SSL_connect_or_accept, V setup, T callback) {
  SSL *ssl = nullptr;
  {
    std::lock_guard<std::mutex> guard(ctx_mutex);
    ssl = SSL_new(ctx);
  }

  if (!ssl) {
    close_socket(sock);
    return false;
  }

  auto bio = BIO_new_socket(sock, BIO_NOCLOSE);
  SSL_set_bio(ssl, bio, bio);

  if (!setup(ssl)) {
    SSL_shutdown(ssl);
    {
      std::lock_guard<std::mutex> guard(ctx_mutex);
      SSL_free(ssl);
    }

    close_socket(sock);
    return false;
  }

  bool ret = false;

  if (SSL_connect_or_accept(ssl) == 1) {
    if (keep_alive_max_count > 0) {
      auto count = keep_alive_max_count;
      while (count > 0 &&
             detail::select_read(sock, CPPHTTPLIB_KEEPALIVE_TIMEOUT_SECOND,
                                 CPPHTTPLIB_KEEPALIVE_TIMEOUT_USECOND) > 0) {
        SSLSocketStream strm(sock, ssl);
        auto last_connection = count == 1;
        auto connection_close = false;

        ret = callback(ssl, strm, last_connection, connection_close);
        if (!ret || connection_close) { break; }

        count--;
      }
    } else {
      SSLSocketStream strm(sock, ssl);
      auto dummy_connection_close = false;
      ret = callback(ssl, strm, true, dummy_connection_close);
    }
  }

  SSL_shutdown(ssl);
  {
    std::lock_guard<std::mutex> guard(ctx_mutex);
    SSL_free(ssl);
  }

  close_socket(sock);

  return ret;
}

class SSLInit {
public:
  SSLInit() {
    SSL_load_error_strings();
    SSL_library_init();
  }

  ~SSLInit() { ERR_free_strings(); }
};

static SSLInit sslinit_;

} // namespace detail

// SSL socket stream implementation
inline SSLSocketStream::SSLSocketStream(socket_t sock, SSL *ssl)
    : sock_(sock), ssl_(ssl) {}

inline SSLSocketStream::~SSLSocketStream() {}

inline int SSLSocketStream::read(char *ptr, size_t size) {
  if (SSL_pending(ssl_) > 0 ||
      detail::select_read(sock_, CPPHTTPLIB_READ_TIMEOUT_SECOND,
                          CPPHTTPLIB_READ_TIMEOUT_USECOND) > 0) {
    return SSL_read(ssl_, ptr, size);
  }
  return -1;
}

inline int SSLSocketStream::write(const char *ptr, size_t size) {
  return SSL_write(ssl_, ptr, size);
}

inline int SSLSocketStream::write(const char *ptr) {
  return write(ptr, strlen(ptr));
}

inline std::string SSLSocketStream::get_remote_addr() const {
  return detail::get_remote_addr(sock_);
}

// SSL HTTP server implementation
inline SSLServer::SSLServer(const char *cert_path, const char *private_key_path,
                            const char *client_ca_cert_file_path,
                            const char *client_ca_cert_dir_path) {
  ctx_ = SSL_CTX_new(SSLv23_server_method());

  if (ctx_) {
    SSL_CTX_set_options(ctx_,
                        SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 |
                            SSL_OP_NO_COMPRESSION |
                            SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION);

    // auto ecdh = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
    // SSL_CTX_set_tmp_ecdh(ctx_, ecdh);
    // EC_KEY_free(ecdh);

    if (SSL_CTX_use_certificate_chain_file(ctx_, cert_path) != 1 ||
        SSL_CTX_use_PrivateKey_file(ctx_, private_key_path, SSL_FILETYPE_PEM) !=
            1) {
      SSL_CTX_free(ctx_);
      ctx_ = nullptr;
    } else if (client_ca_cert_file_path || client_ca_cert_dir_path) {
      // if (client_ca_cert_file_path) {
      //   auto list = SSL_load_client_CA_file(client_ca_cert_file_path);
      //   SSL_CTX_set_client_CA_list(ctx_, list);
      // }

      SSL_CTX_load_verify_locations(ctx_, client_ca_cert_file_path,
                                    client_ca_cert_dir_path);

      SSL_CTX_set_verify(
          ctx_,
          SSL_VERIFY_PEER |
              SSL_VERIFY_FAIL_IF_NO_PEER_CERT, // SSL_VERIFY_CLIENT_ONCE,
          nullptr);
    }
  }
}

inline SSLServer::~SSLServer() {
  if (ctx_) { SSL_CTX_free(ctx_); }
}

inline bool SSLServer::is_valid() const { return ctx_; }

inline bool SSLServer::read_and_close_socket(socket_t sock) {
  return detail::read_and_close_socket_ssl(
      sock, keep_alive_max_count_, ctx_, ctx_mutex_, SSL_accept,
      [](SSL * /*ssl*/) { return true; },
      [this](SSL *ssl, Stream &strm, bool last_connection,
             bool &connection_close) {
        return process_request(strm, last_connection, connection_close,
                               [&](Request &req) { req.ssl = ssl; });
      });
}

// SSL HTTP client implementation
inline SSLClient::SSLClient(const char *host, int port, time_t timeout_sec,
                            const char *client_cert_path,
                            const char *client_key_path)
    : Client(host, port, timeout_sec) {
  ctx_ = SSL_CTX_new(SSLv23_client_method());

  detail::split(&host_[0], &host_[host_.size()], '.',
                [&](const char *b, const char *e) {
                  host_components_.emplace_back(std::string(b, e));
                });
  if (client_cert_path && client_key_path) {
    if (SSL_CTX_use_certificate_file(ctx_, client_cert_path,
                                     SSL_FILETYPE_PEM) != 1 ||
        SSL_CTX_use_PrivateKey_file(ctx_, client_key_path, SSL_FILETYPE_PEM) !=
            1) {
      SSL_CTX_free(ctx_);
      ctx_ = nullptr;
    }
  }
}

inline SSLClient::~SSLClient() {
  if (ctx_) { SSL_CTX_free(ctx_); }
}

inline bool SSLClient::is_valid() const { return ctx_; }

inline void SSLClient::set_ca_cert_path(const char *ca_cert_file_path,
                                        const char *ca_cert_dir_path) {
  if (ca_cert_file_path) { ca_cert_file_path_ = ca_cert_file_path; }
  if (ca_cert_dir_path) { ca_cert_dir_path_ = ca_cert_dir_path; }
}

inline void SSLClient::enable_server_certificate_verification(bool enabled) {
  server_certificate_verification_ = enabled;
}

inline long SSLClient::get_openssl_verify_result() const {
  return verify_result_;
}

inline bool SSLClient::read_and_close_socket(socket_t sock, Request &req,
                                             Response &res) {

  return is_valid() &&
         detail::read_and_close_socket_ssl(
             sock, 0, ctx_, ctx_mutex_,
             [&](SSL *ssl) {
               if (ca_cert_file_path_.empty()) {
                 SSL_CTX_set_verify(ctx_, SSL_VERIFY_NONE, nullptr);
               } else {
                 if (!SSL_CTX_load_verify_locations(
                         ctx_, ca_cert_file_path_.c_str(), nullptr)) {
                   return false;
                 }
                 SSL_CTX_set_verify(ctx_, SSL_VERIFY_PEER, nullptr);
               }

               if (SSL_connect(ssl) != 1) { return false; }

               if (server_certificate_verification_) {
                 verify_result_ = SSL_get_verify_result(ssl);

                 if (verify_result_ != X509_V_OK) { return false; }

                 auto server_cert = SSL_get_peer_certificate(ssl);

                 if (server_cert == nullptr) { return false; }

                 if (!verify_host(server_cert)) {
                   X509_free(server_cert);
                   return false;
                 }
                 X509_free(server_cert);
               }

               return true;
             },
             [&](SSL *ssl) {
               SSL_set_tlsext_host_name(ssl, host_.c_str());
               return true;
             },
             [&](SSL * /*ssl*/, Stream &strm, bool /*last_connection*/,
                 bool &connection_close) {
               return process_request(strm, req, res, connection_close);
             });
}

inline bool SSLClient::is_ssl() const { return true; }

inline bool SSLClient::verify_host(X509 *server_cert) const {
  /* Quote from RFC2818 section 3.1 "Server Identity"

     If a subjectAltName extension of type dNSName is present, that MUST
     be used as the identity. Otherwise, the (most specific) Common Name
     field in the Subject field of the certificate MUST be used. Although
     the use of the Common Name is existing practice, it is deprecated and
     Certification Authorities are encouraged to use the dNSName instead.

     Matching is performed using the matching rules specified by
     [RFC2459].  If more than one identity of a given type is present in
     the certificate (e.g., more than one dNSName name, a match in any one
     of the set is considered acceptable.) Names may contain the wildcard
     character * which is considered to match any single domain name
     component or component fragment. E.g., *.a.com matches foo.a.com but
     not bar.foo.a.com. f*.com matches foo.com but not bar.com.

     In some cases, the URI is specified as an IP address rather than a
     hostname. In this case, the iPAddress subjectAltName must be present
     in the certificate and must exactly match the IP in the URI.

  */
  return verify_host_with_subject_alt_name(server_cert) ||
         verify_host_with_common_name(server_cert);
}

inline bool
SSLClient::verify_host_with_subject_alt_name(X509 *server_cert) const {
  auto ret = false;

  auto type = GEN_DNS;

  struct in6_addr addr6;
  struct in_addr addr;
  size_t addr_len = 0;

  if (inet_pton(AF_INET6, host_.c_str(), &addr6)) {
    type = GEN_IPADD;
    addr_len = sizeof(struct in6_addr);
  } else if (inet_pton(AF_INET, host_.c_str(), &addr)) {
    type = GEN_IPADD;
    addr_len = sizeof(struct in_addr);
  }

  auto alt_names = static_cast<const struct stack_st_GENERAL_NAME *>(
      X509_get_ext_d2i(server_cert, NID_subject_alt_name, nullptr, nullptr));

  if (alt_names) {
    auto dsn_matched = false;
    auto ip_mached = false;

    auto count = sk_GENERAL_NAME_num(alt_names);

    for (auto i = 0; i < count && !dsn_matched; i++) {
      auto val = sk_GENERAL_NAME_value(alt_names, i);
      if (val->type == type) {
        auto name = (const char *)ASN1_STRING_get0_data(val->d.ia5);
        auto name_len = (size_t)ASN1_STRING_length(val->d.ia5);

        if (strlen(name) == name_len) {
          switch (type) {
          case GEN_DNS: dsn_matched = check_host_name(name, name_len); break;

          case GEN_IPADD:
            if (!memcmp(&addr6, name, addr_len) ||
                !memcmp(&addr, name, addr_len)) {
              ip_mached = true;
            }
            break;
          }
        }
      }
    }

    if (dsn_matched || ip_mached) { ret = true; }
  }

  GENERAL_NAMES_free((STACK_OF(GENERAL_NAME) *)alt_names);

  return ret;
}

inline bool SSLClient::verify_host_with_common_name(X509 *server_cert) const {
  const auto subject_name = X509_get_subject_name(server_cert);

  if (subject_name != nullptr) {
    char name[BUFSIZ];
    auto name_len = X509_NAME_get_text_by_NID(subject_name, NID_commonName,
                                              name, sizeof(name));

    if (name_len != -1) { return check_host_name(name, name_len); }
  }

  return false;
}

inline bool SSLClient::check_host_name(const char *pattern,
                                       size_t pattern_len) const {
  if (host_.size() == pattern_len && host_ == pattern) { return true; }

  // Wildcard match
  // https://bugs.launchpad.net/ubuntu/+source/firefox-3.0/+bug/376484
  std::vector<std::string> pattern_components;
  detail::split(&pattern[0], &pattern[pattern_len], '.',
                [&](const char *b, const char *e) {
                  pattern_components.emplace_back(std::string(b, e));
                });

  if (host_components_.size() != pattern_components.size()) { return false; }

  auto itr = pattern_components.begin();
  for (const auto &h : host_components_) {
    auto &p = *itr;
    if (p != h && p != "*") {
      auto partial_match = (p.size() > 0 && p[p.size() - 1] == '*' &&
                            !p.compare(0, p.size() - 1, h));
      if (!partial_match) { return false; }
    }
    ++itr;
  }

  return true;
}
#endif

} // namespace httplib

#endif // CPPHTTPLIB_HTTPLIB_H
