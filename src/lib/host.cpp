#include <unistd.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <cstring>
#include <iostream>
#include "host.hpp"

namespace WebCFace
{
HostInfo::HostInfo()
{
    char c_hostname[1024];
    if (gethostname(c_hostname, sizeof(c_hostname)) != 0) {
        std::cerr << "[WebCFace] gethostname error" << std::endl;
        c_hostname[0] = '\0';
    }
    name = std::string(c_hostname);

    ifaddrs* ifaddr;
    addr = "";
    if (getifaddrs(&ifaddr) == -1) {
        std::cerr << "[WebCFace] getifaddrs error" << std::endl;
    } else {
        for (ifaddrs* ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr == NULL)
                continue;

            int family, s;
            char host[NI_MAXHOST];
            family = ifa->ifa_addr->sa_family;

            if (family == AF_INET /*|| family == AF_INET6*/) {
                s = getnameinfo(ifa->ifa_addr,
                    (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6),
                    host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
                if (s != 0) {
                    std::cerr << "[WebCFace] getnameinfo() failed: " << strerror(s) << std::endl;
                }
                if (strcmp(host, "127.0.0.1") == 0) {
                    continue;
                }
                // if (addr != "") {
                //     addr += " ";
                // }
                addr += host;
                break;
            }
        }
    }
}
}  // namespace WebCFace
