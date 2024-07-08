#include "webcface/server/ip.h"

#ifdef _WIN32
#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

#else
#include <ifaddrs.h>
#include <arpa/inet.h>

#endif

WEBCFACE_NS_BEGIN
namespace server {
std::vector<std::string>
getIpAddresses([[maybe_unused]] const std::shared_ptr<spdlog::logger> &logger) {
    std::vector<std::string> ret;

#ifdef _WIN32
    PIP_ADAPTER_ADDRESSES pAddresses = NULL;
    ULONG outBufLen = 15000;
    for (int i = 0; i < 3; i++) {
        pAddresses = (IP_ADAPTER_ADDRESSES *)MALLOC(outBufLen);
        if (pAddresses == NULL) {
            return {};
        }

        auto dwRetVal = GetAdaptersAddresses(
            AF_INET,
            GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST |
                GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_SKIP_FRIENDLY_NAME,
            NULL, pAddresses, &outBufLen);

        if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
            FREE(pAddresses);
            pAddresses = NULL;
            continue;
        } else if (dwRetVal == NO_ERROR) {
            for (PIP_ADAPTER_ADDRESSES adapter = pAddresses; adapter != NULL;
                 adapter = adapter->Next) {
                std::wstring desc = adapter->Description;
                // logger->info(std::string(desc.begin(), desc.end()));
                // logger->info("status={}", adapter->OperStatus);
                if (adapter->OperStatus != IfOperStatusUp) {
                    continue;
                }
                for (PIP_ADAPTER_UNICAST_ADDRESS address =
                         adapter->FirstUnicastAddress;
                     address != NULL; address = address->Next) {
                    if (address->Address.lpSockaddr->sa_family == AF_INET) {
                        // IPv4
                        SOCKADDR_IN *ipv4 = reinterpret_cast<SOCKADDR_IN *>(
                            address->Address.lpSockaddr);

                        char str_buffer[INET_ADDRSTRLEN] = {0};
                        inet_ntop(AF_INET, &(ipv4->sin_addr), str_buffer,
                                  INET_ADDRSTRLEN);
                        ret.push_back(str_buffer);
                        // logger->info(std::string(str_buffer));
                    }
                }
            }
        } else {
            logger->error("Call to GetAdaptersAddresses failed with error: {}",
                          dwRetVal);
        }
        FREE(pAddresses);
        break;
    }

#else
    // Linux, Mac

    ifaddrs *ifAddrStruct = NULL;

    getifaddrs(&ifAddrStruct);

    for (ifaddrs *ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) {
            continue;
        }
        if (ifa->ifa_addr->sa_family == AF_INET) { // check it is IP4
            // is a valid IP4 Address
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET,
                      &reinterpret_cast<sockaddr_in *>(ifa->ifa_addr)->sin_addr,
                      addressBuffer, INET_ADDRSTRLEN);
            ret.push_back(addressBuffer);
        }
    }
    if (ifAddrStruct != NULL) {
        freeifaddrs(ifAddrStruct);
    }
#endif

    return ret;
}
} // namespace server
WEBCFACE_NS_END
