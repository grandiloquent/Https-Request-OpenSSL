#include "detail.h"

namespace httplib
{

namespace detail
{
    template <typename Fn>
    socket_t create_socket(const char* host, int port, Fn fn,
        int socket_flags)
    {
#ifdef _WIN32
#    define SO_SYNCHRONOUS_NONALERT 0x20
#    define SO_OPENTYPE 0x7008

        int opt = SO_SYNCHRONOUS_NONALERT;
        setsockopt(INVALID_SOCKET, SOL_SOCKET, SO_OPENTYPE, (char*)&opt,
            sizeof(opt));
#endif

        // Get address info
        struct addrinfo hints;
        struct addrinfo* result;

        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = socket_flags;
        hints.ai_protocol = 0;

        auto service = std::to_string(port);

        if (getaddrinfo(host, service.c_str(), &hints, &result))
        {
            return INVALID_SOCKET;
        }

        for (auto rp = result; rp; rp = rp->ai_next)
        {
            // Create a socket
#ifdef _WIN32
            auto sock = WSASocketW(rp->ai_family, rp->ai_socktype, rp->ai_protocol,
                nullptr, 0, WSA_FLAG_NO_HANDLE_INHERIT);
#else
            auto sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
#endif
            if (sock == INVALID_SOCKET)
            {
                continue;
            }

#ifndef _WIN32
            if (fcntl(sock, F_SETFD, FD_CLOEXEC) == -1)
            {
                continue;
            }
#endif

            // Make 'reuse address' option available
            int yes = 1;
            setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(yes));
#ifdef SO_REUSEPORT
            setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (char*)&yes, sizeof(yes));
#endif

            // bind or connect
            if (fn(sock, *rp))
            {
                freeaddrinfo(result);
                return sock;
            }

            close_socket(sock);
        }

        freeaddrinfo(result);
        return INVALID_SOCKET;
    }

  
}
}