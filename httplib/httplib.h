
#ifndef CPPHTTPLIB_HTTPLIB_H
#define CPPHTTPLIB_HTTPLIB_H

#ifdef _WIN32
#    ifndef _CRT_SECURE_NO_WARNINGS
#        define _CRT_SECURE_NO_WARNINGS
#    endif //_CRT_SECURE_NO_WARNINGS

#    ifndef _CRT_NONSTDC_NO_DEPRECATE
#        define _CRT_NONSTDC_NO_DEPRECATE
#    endif //_CRT_NONSTDC_NO_DEPRECATE

#    if defined(_MSC_VER) && _MSC_VER < 1900
#        define snprintf _snprintf_s
#    endif // _MSC_VER

#    ifndef S_ISREG
#        define S_ISREG(m) (((m)&S_IFREG) == S_IFREG)
#    endif // S_ISREG

#    ifndef S_ISDIR
#        define S_ISDIR(m) (((m)&S_IFDIR) == S_IFDIR)
#    endif // S_ISDIR

#    ifndef NOMINMAX
#        define NOMINMAX
#    endif // NOMINMAX

#    include <io.h>
#    include <winsock2.h>
#    include <ws2tcpip.h>

#    pragma comment(lib, "ws2_32.lib")

#    ifndef strcasecmp
#        define strcasecmp _stricmp
#    endif // strcasecmp

typedef SOCKET socket_t;
#else
#    include <arpa/inet.h>
#    include <cstring>
#    include <netdb.h>
#    include <netinet/in.h>
#    include <pthread.h>
#    include <signal.h>
#    include <sys/select.h>
#    include <sys/socket.h>
#    include <unistd.h>

typedef int socket_t;
#    define INVALID_SOCKET (-1)
#endif //_WIN32

#include <assert.h>
#include <atomic>
#include <fcntl.h>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <regex>
#include <string>
#include <sys/stat.h>
#include <thread>

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
#    include <openssl/err.h>
#    include <openssl/ssl.h>
#    include <openssl/x509v3.h>

#    if OPENSSL_VERSION_NUMBER < 0x10100000L
inline const unsigned char* ASN1_STRING_get0_data(const ASN1_STRING* asn1)
{
    return M_ASN1_STRING_data(asn1);
}
#    endif
#endif

#ifdef CPPHTTPLIB_ZLIB_SUPPORT
#    include <zlib.h>
#endif

/*
 * Configuration
 */
#define CPPHTTPLIB_KEEPALIVE_TIMEOUT_SECOND 5
#define CPPHTTPLIB_KEEPALIVE_TIMEOUT_USECOND 0
#define CPPHTTPLIB_KEEPALIVE_MAX_COUNT 5
#define CPPHTTPLIB_READ_TIMEOUT_SECOND 5
#define CPPHTTPLIB_READ_TIMEOUT_USECOND 0
#define CPPHTTPLIB_REQUEST_URI_MAX_LENGTH 8192
#define CPPHTTPLIB_PAYLOAD_MAX_LENGTH (std::numeric_limits<size_t>::max)()
#define CPPHTTPLIB_RECV_BUFSIZ size_t(4096u)

#ifdef _WIN32
#    ifndef WSA_FLAG_NO_HANDLE_INHERIT
#        define WSA_FLAG_NO_HANDLE_INHERIT 0x80
#    endif
#endif

namespace detail
{

struct ci
{
    bool operator()(const std::string& s1, const std::string& s2) const
    {
        return std::lexicographical_compare(
            s1.begin(), s1.end(), s2.begin(), s2.end(),
            [](char c1, char c2) { return ::tolower(c1) < ::tolower(c2); });
    }
};
}

typedef std::multimap<std::string, std::string, detail::ci> Headers;
typedef std::function<bool(uint64_t current, uint64_t total)> Progress;
typedef std::multimap<std::string, std::string> Params;

#include "Stream.h"
#include "SocketStream.h"
#include "stream_line_reader.h"

#endif
