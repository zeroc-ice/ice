// Copyright (c) ZeroC, Inc.

#include "Network.h"
#include "Ice/LocalExceptions.h"
#include "Ice/LoggerUtil.h" // For setTcpBufSize
#include "Ice/Properties.h" // For setTcpBufSize
#include "Ice/StringUtil.h"
#include "NetworkProxy.h"
#include "ProtocolInstance.h" // For setTcpBufSize
#include "Random.h"

#include "DisableWarnings.h"

#include <cassert>

// TODO: fix this warning
#if defined(_MSC_VER)
#    pragma warning(disable : 4244) // 'argument': conversion from 'int' to 'u_short', possible loss of data
#endif

#if defined(_WIN32)
#    include <Mswsock.h>
#    include <iphlpapi.h>
#    include <mstcpip.h>
#    include <winsock2.h>
#    include <ws2tcpip.h>
#else
#    include <net/if.h>
#    include <sys/ioctl.h>
#endif

#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
#    include <ifaddrs.h>
#endif

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{
    struct AddressCompare
    {
    public:
        bool operator()(const Address& lhs, const Address& rhs) const { return compareAddress(lhs, rhs) < 0; }
    };

    void sortAddresses(vector<Address>& addrs, ProtocolSupport protocol, bool preferIPv6)
    {
        if (protocol == EnableBoth)
        {
            if (preferIPv6)
            {
                stable_partition(
                    addrs.begin(),
                    addrs.end(),
                    [](const Address& ss) { return ss.saStorage.ss_family == AF_INET6; });
            }
            else
            {
                stable_partition(
                    addrs.begin(),
                    addrs.end(),
                    [](const Address& ss) { return ss.saStorage.ss_family != AF_INET6; });
            }
        }
    }

    void setTcpNoDelay(SOCKET fd)
    {
        int flag = 1;
        if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&flag), int(sizeof(int))) == SOCKET_ERROR)
        {
            closeSocketNoThrow(fd);
            throw SocketException(__FILE__, __LINE__, getSocketErrno());
        }
    }

    void setKeepAlive(SOCKET fd)
    {
        int flag = 1;
        if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<char*>(&flag), int(sizeof(int))) == SOCKET_ERROR)
        {
            closeSocketNoThrow(fd);
            throw SocketException(__FILE__, __LINE__, getSocketErrno());
        }
    }

    SOCKET
    createSocketImpl(bool udp, int family)
    {
        SOCKET fd;
        if (udp)
        {
            fd = socket(family, SOCK_DGRAM, IPPROTO_UDP);
        }
        else
        {
            fd = socket(family, SOCK_STREAM, IPPROTO_TCP);
        }

        if (fd == INVALID_SOCKET)
        {
            throw SocketException(__FILE__, __LINE__, getSocketErrno());
        }

        if (!udp)
        {
            setTcpNoDelay(fd);
            setKeepAlive(fd);
        }

        return fd;
    }

    // Only used for multicast.
    vector<Address> getLocalAddresses(ProtocolSupport protocol)
    {
        vector<Address> result;

#if defined(_WIN32)
        DWORD family;
        switch (protocol)
        {
            case EnableIPv4:
                family = AF_INET;
                break;
            case EnableIPv6:
                family = AF_INET6;
                break;
            default:
                family = AF_UNSPEC;
                break;
        }

        DWORD size = 0;
        DWORD rv = GetAdaptersAddresses(family, 0, nullptr, nullptr, &size);
        if (rv == ERROR_BUFFER_OVERFLOW)
        {
            PIP_ADAPTER_ADDRESSES adapter_addresses = (PIP_ADAPTER_ADDRESSES)malloc(size);
            rv = GetAdaptersAddresses(family, 0, nullptr, adapter_addresses, &size);
            if (rv == ERROR_SUCCESS)
            {
                for (PIP_ADAPTER_ADDRESSES aa = adapter_addresses; aa != nullptr; aa = aa->Next)
                {
                    if (aa->OperStatus != IfOperStatusUp)
                    {
                        continue;
                    }
                    for (PIP_ADAPTER_UNICAST_ADDRESS ua = aa->FirstUnicastAddress; ua != nullptr; ua = ua->Next)
                    {
                        Address addr;
                        memcpy(&addr.saStorage, ua->Address.lpSockaddr, ua->Address.iSockaddrLength);
                        if (addr.saStorage.ss_family == AF_INET && protocol != EnableIPv6)
                        {
                            if (addr.saIn.sin_addr.s_addr != 0)
                            {
                                result.push_back(addr);
                                break; // a single address per interface is sufficient
                            }
                        }
                        else if (addr.saStorage.ss_family == AF_INET6 && protocol != EnableIPv4)
                        {
                            if (!IN6_IS_ADDR_UNSPECIFIED(&addr.saIn6.sin6_addr))
                            {
                                result.push_back(addr);
                                break; // a single address per interface is sufficient
                            }
                        }
                    }
                }
            }

            free(adapter_addresses);
        }
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
        struct ifaddrs* ifap;
        if (::getifaddrs(&ifap) == SOCKET_ERROR)
        {
            throw SocketException(__FILE__, __LINE__, getSocketErrno());
        }

        struct ifaddrs* curr = ifap;
        set<string> interfaces;
        while (curr != nullptr)
        {
            if (curr->ifa_addr)
            {
                if (curr->ifa_addr->sa_family == AF_INET && protocol != EnableIPv6)
                {
                    Address addr;
                    memcpy(&addr.saStorage, curr->ifa_addr, sizeof(sockaddr_in));
                    if (addr.saIn.sin_addr.s_addr != 0)
                    {
                        if (interfaces.find(curr->ifa_name) == interfaces.end())
                        {
                            result.push_back(addr);
                            interfaces.insert(curr->ifa_name);
                        }
                    }
                }
                else if (curr->ifa_addr->sa_family == AF_INET6 && protocol != EnableIPv4)
                {
                    Address addr;
                    memcpy(&addr.saStorage, curr->ifa_addr, sizeof(sockaddr_in6));
                    if (!IN6_IS_ADDR_UNSPECIFIED(&addr.saIn6.sin6_addr))
                    {
                        if (interfaces.find(curr->ifa_name) == interfaces.end())
                        {
                            result.push_back(addr);
                            interfaces.insert(curr->ifa_name);
                        }
                    }
                }
            }

            curr = curr->ifa_next;
        }

        ::freeifaddrs(ifap);
#else
        for (int i = 0; i < 2; i++)
        {
            if ((i == 0 && protocol == EnableIPv6) || (i == 1 && protocol == EnableIPv4))
            {
                continue;
            }
            SOCKET fd = createSocketImpl(false, i == 0 ? AF_INET : AF_INET6);

            int cmd = SIOCGIFCONF;
            struct ifconf ifc;
            int numaddrs = 10;
            int old_ifc_len = 0;

            //
            // Need to call ioctl multiple times since we do not know up front
            // how many addresses there will be, and thus how large a buffer we need.
            // We keep increasing the buffer size until subsequent calls return
            // the same length, meaning we have all the addresses.
            //
            while (true)
            {
                int bufsize = numaddrs * static_cast<int>(sizeof(struct ifreq));
                ifc.ifc_len = bufsize;
                ifc.ifc_buf = (char*)malloc(bufsize);

                int rs = ioctl(fd, cmd, &ifc);
                if (rs == SOCKET_ERROR)
                {
                    free(ifc.ifc_buf);
                    closeSocketNoThrow(fd);
                    throw SocketException(__FILE__, __LINE__, getSocketErrno());
                }
                else if (ifc.ifc_len == old_ifc_len)
                {
                    //
                    // Returned same length twice in a row, finished.
                    //
                    break;
                }
                else
                {
                    old_ifc_len = ifc.ifc_len;
                }

                numaddrs += 10;
                free(ifc.ifc_buf);
            }
            closeSocket(fd);

            numaddrs = ifc.ifc_len / static_cast<int>(sizeof(struct ifreq));
            struct ifreq* ifr = ifc.ifc_req;
            set<string> interfaces;
            for (int j = 0; j < numaddrs; ++j)
            {
                if (!(ifr[j].ifr_flags & IFF_LOOPBACK)) // Don't include loopback interface addresses
                {
                    //
                    // On Solaris the above Loopback check does not always work so we double
                    // check the address below. Solaris also returns duplicate entries that need
                    // to be filtered out.
                    //
                    if (ifr[j].ifr_addr.sa_family == AF_INET && protocol != EnableIPv6)
                    {
                        Address addr;
                        memcpy(&addr.saStorage, &ifr[j].ifr_addr, sizeof(sockaddr_in));
                        if (addr.saIn.sin_addr.s_addr != 0)
                        {
                            if (interfaces.find(ifr[j].ifr_name) == interfaces.end())
                            {
                                result.push_back(addr);
                                interfaces.insert(ifr[j].ifr_name);
                            }
                        }
                    }
                    else if (ifr[j].ifr_addr.sa_family == AF_INET6 && protocol != EnableIPv4)
                    {
                        Address addr;
                        memcpy(&addr.saStorage, &ifr[j].ifr_addr, sizeof(sockaddr_in6));
                        if (!IN6_IS_ADDR_UNSPECIFIED(&addr.saIn6.sin6_addr))
                        {
                            if (interfaces.find(ifr[j].ifr_name) == interfaces.end())
                            {
                                result.push_back(addr);
                                interfaces.insert(ifr[j].ifr_name);
                            }
                        }
                    }
                }
            }
            free(ifc.ifc_buf);
        }
#endif

        //
        // Remove potential duplicates from the result.
        //
        set<Address, AddressCompare> seen;
        vector<Address> tmp;
        tmp.swap(result);
        for (const auto& p : tmp)
        {
            if (seen.find(p) == seen.end())
            {
                result.push_back(p);
                seen.insert(p);
            }
        }
        return result;
    }

    bool isWildcard(const string& host, ProtocolSupport protocol, bool& ipv4)
    {
        Address addr = getAddressForServer(host, 0, protocol, true, false);
        if (addr.saStorage.ss_family == AF_INET)
        {
            if (addr.saIn.sin_addr.s_addr == INADDR_ANY)
            {
                ipv4 = true;
                return true;
            }
        }
        else if (addr.saStorage.ss_family == AF_INET6)
        {
            if (IN6_IS_ADDR_UNSPECIFIED(&addr.saIn6.sin6_addr))
            {
                ipv4 = false;
                return true;
            }
        }
        return false;
    }

    int getInterfaceIndex(const string& intf)
    {
        if (intf.empty())
        {
            return 0;
        }

        string name;
        bool isAddr;
        in6_addr addr;
        string::size_type pos = intf.find('%');
        if (pos != string::npos)
        {
            //
            // If it's a link-local address, use the zone indice.
            //
            isAddr = false;
            name = intf.substr(pos + 1);
        }
        else
        {
            //
            // Then check if it's an IPv6 address. If it's an address we'll
            // look for the interface index by address.
            //
            isAddr = inet_pton(AF_INET6, intf.c_str(), &addr) > 0;
            name = intf;
        }

        //
        // Check if index
        //
        int index = -1;
        istringstream p(name);
        if ((p >> index) && p.eof())
        {
            return index;
        }

#ifdef _WIN32
        IP_ADAPTER_ADDRESSES addrs;
        ULONG buflen = 0;
        if (::GetAdaptersAddresses(AF_INET6, 0, 0, &addrs, &buflen) == ERROR_BUFFER_OVERFLOW)
        {
            PIP_ADAPTER_ADDRESSES paddrs;
            char* buf = new char[buflen];
            paddrs = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(buf);
            if (::GetAdaptersAddresses(AF_INET6, 0, 0, paddrs, &buflen) == NO_ERROR)
            {
                while (paddrs)
                {
                    if (isAddr)
                    {
                        PIP_ADAPTER_UNICAST_ADDRESS ipAddr = paddrs->FirstUnicastAddress;
                        while (ipAddr)
                        {
                            if (ipAddr->Address.lpSockaddr->sa_family == AF_INET6)
                            {
                                struct sockaddr_in6* ipv6Addr =
                                    reinterpret_cast<struct sockaddr_in6*>(ipAddr->Address.lpSockaddr);
                                if (memcmp(&addr, &ipv6Addr->sin6_addr, sizeof(in6_addr)) == 0)
                                {
                                    break;
                                }
                            }
                            ipAddr = ipAddr->Next;
                        }
                        if (ipAddr)
                        {
                            index = paddrs->Ipv6IfIndex;
                            break;
                        }
                    }
                    else
                    {
                        //
                        // Don't need to pass a wide string converter as the wide string
                        // come from Windows API.
                        //
                        if (wstringToString(paddrs->FriendlyName, getProcessStringConverter()) == name)
                        {
                            index = paddrs->Ipv6IfIndex;
                            break;
                        }
                    }
                    paddrs = paddrs->Next;
                }
            }
            delete[] buf;
        }
        if (index < 0) // interface not found
        {
            throw Ice::SocketException(__FILE__, __LINE__, WSAEINVAL);
        }
#else

        //
        // Look for an interface with a matching IP address
        //
        if (isAddr)
        {
#    if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
            struct ifaddrs* ifap;
            if (::getifaddrs(&ifap) != SOCKET_ERROR)
            {
                struct ifaddrs* curr = ifap;
                while (curr != nullptr)
                {
                    if (curr->ifa_addr && curr->ifa_addr->sa_family == AF_INET6)
                    {
                        auto* ipv6Addr = reinterpret_cast<struct sockaddr_in6*>(curr->ifa_addr);
                        if (memcmp(&addr, &ipv6Addr->sin6_addr, sizeof(in6_addr)) == 0)
                        {
                            index = static_cast<int>(if_nametoindex(curr->ifa_name));
                            break;
                        }
                    }
                    curr = curr->ifa_next;
                }
                ::freeifaddrs(ifap);
            }
#    else
            SOCKET fd = createSocketImpl(false, AF_INET6);
            int cmd = SIOCGIFCONF;
            struct ifconf ifc;
            int numaddrs = 10;
            int old_ifc_len = 0;

            //
            // Need to call ioctl multiple times since we do not know up front
            // how many addresses there will be, and thus how large a buffer we need.
            // We keep increasing the buffer size until subsequent calls return
            // the same length, meaning we have all the addresses.
            //
            while (true)
            {
                int bufsize = numaddrs * static_cast<int>(sizeof(struct ifreq));
                ifc.ifc_len = bufsize;
                ifc.ifc_buf = (char*)malloc(bufsize);

                int rs = ioctl(fd, cmd, &ifc);
                if (rs == SOCKET_ERROR)
                {
                    free(ifc.ifc_buf);
                    ifc.ifc_buf = 0;
                    break;
                }
                else if (ifc.ifc_len == old_ifc_len)
                {
                    //
                    // Returned same length twice in a row, finished.
                    //
                    break;
                }
                else
                {
                    old_ifc_len = ifc.ifc_len;
                }
                numaddrs += 10;
                free(ifc.ifc_buf);
            }
            closeSocketNoThrow(fd);

            if (ifc.ifc_buf)
            {
                numaddrs = ifc.ifc_len / static_cast<int>(sizeof(struct ifreq));
                struct ifreq* ifr = ifc.ifc_req;
                for (int i = 0; i < numaddrs; ++i)
                {
                    if (ifr[i].ifr_addr.sa_family == AF_INET6)
                    {
                        struct sockaddr_in6* ipv6Addr = reinterpret_cast<struct sockaddr_in6*>(&ifr[i].ifr_addr);
                        if (memcmp(&addr, &ipv6Addr->sin6_addr, sizeof(in6_addr)) == 0)
                        {
                            index = static_cast<int>(if_nametoindex(ifr[i].ifr_name));
                            break;
                        }
                    }
                }
                free(ifc.ifc_buf);
            }
#    endif
        }
        else // Look for an interface with the given name.
        {
            index = static_cast<int>(if_nametoindex(name.c_str()));
        }
        if (index <= 0)
        {
            // index == 0 if if_nametoindex returned 0, < 0 if name wasn't found
            throw Ice::SocketException(__FILE__, __LINE__, index == 0 ? getSocketErrno() : ENXIO);
        }
#endif

        return index;
    }

    struct in_addr getInterfaceAddress(const string& name)
    {
        struct in_addr addr;
        addr.s_addr = INADDR_ANY;
        if (name.empty())
        {
            return addr;
        }

        if (inet_pton(AF_INET, name.c_str(), &addr) > 0)
        {
            return addr;
        }

#ifdef _WIN32
        IP_ADAPTER_ADDRESSES addrs;
        ULONG buflen = 0;
        if (::GetAdaptersAddresses(AF_INET, 0, 0, &addrs, &buflen) == ERROR_BUFFER_OVERFLOW)
        {
            PIP_ADAPTER_ADDRESSES paddrs;
            char* buf = new char[buflen];
            paddrs = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(buf);
            if (::GetAdaptersAddresses(AF_INET, 0, 0, paddrs, &buflen) == NO_ERROR)
            {
                while (paddrs)
                {
                    //
                    // Don't need to pass a wide string converter as the wide string come
                    // from Windows API.
                    //
                    if (wstringToString(paddrs->FriendlyName, getProcessStringConverter()) == name)
                    {
                        struct sockaddr_in addrin;
                        memcpy(
                            &addrin,
                            paddrs->FirstUnicastAddress->Address.lpSockaddr,
                            paddrs->FirstUnicastAddress->Address.iSockaddrLength);
                        delete[] buf;
                        return addrin.sin_addr;
                    }
                    paddrs = paddrs->Next;
                }
            }
            delete[] buf;
        }
        throw Ice::SocketException(__FILE__, __LINE__, WSAEINVAL);
#else
        ifreq if_address;
        strncpy(if_address.ifr_name, name.c_str(), IFNAMSIZ - 1);
        if_address.ifr_name[IFNAMSIZ - 1] = '\0';

        SOCKET fd = createSocketImpl(false, AF_INET);
        int rc = ioctl(fd, SIOCGIFADDR, &if_address);
        closeSocketNoThrow(fd);
        if (rc == SOCKET_ERROR)
        {
            throw Ice::SocketException(__FILE__, __LINE__, getSocketErrno());
        }
        return reinterpret_cast<struct sockaddr_in*>(&if_address.ifr_addr)->sin_addr;
#endif
    }

    int getAddressStorageSize(const Address& addr)
    {
        int size = 0;
        if (addr.saStorage.ss_family == AF_INET)
        {
            size = sizeof(sockaddr_in);
        }
        else if (addr.saStorage.ss_family == AF_INET6)
        {
            size = sizeof(sockaddr_in6);
        }
        return size;
    }

    vector<Address> getLoopbackAddresses(ProtocolSupport protocol, int port = 0)
    {
        vector<Address> result;

        Address addr;
        memset(&addr.saStorage, 0, sizeof(sockaddr_storage));

        //
        // We don't use getaddrinfo when host is empty as it's not portable (some old Linux
        // versions don't support it).
        //
        if (protocol != EnableIPv4)
        {
            addr.saIn6.sin6_family = AF_INET6;
            addr.saIn6.sin6_port = htons(port);
            addr.saIn6.sin6_addr = in6addr_loopback;
            result.push_back(addr);
        }
        if (protocol != EnableIPv6)
        {
            addr.saIn.sin_family = AF_INET;
            addr.saIn.sin_port = htons(port);
            addr.saIn.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            result.push_back(addr);
        }
        return result;
    }
}

ReadyCallback::~ReadyCallback() = default; // Out of line to avoid weak vtable

NativeInfo::~NativeInfo() = default; // Out of line to avoid weak vtable

void
NativeInfo::setReadyCallback(const ReadyCallbackPtr& callback)
{
    _readyCallback = callback;
}

#ifdef ICE_USE_IOCP

IceInternal::AsyncInfo::AsyncInfo(SocketOperation s)
{
    ZeroMemory(this, sizeof(AsyncInfo));
    status = s;
}

void
IceInternal::NativeInfo::initialize(HANDLE handle, ULONG_PTR key)
{
    _handle = handle;
    _key = key;
}

void
IceInternal::NativeInfo::completed(SocketOperation operation)
{
    if (!PostQueuedCompletionStatus(_handle, 0, _key, getAsyncInfo(operation)))
    {
        throw Ice::SocketException(__FILE__, __LINE__, GetLastError());
    }
}

#else

void
IceInternal::NativeInfo::setNewFd(SOCKET fd)
{
    assert(_fd == INVALID_SOCKET); // This can only be called once, when the current socket isn't set yet.
    _newFd = fd;
}

bool
IceInternal::NativeInfo::newFd()
{
    if (_newFd == INVALID_SOCKET)
    {
        return false;
    }
    assert(_fd == INVALID_SOCKET);
    swap(_fd, _newFd);
    return true;
}

#endif

bool
IceInternal::noMoreFds(int error)
{
#if defined(_WIN32)
    return error == WSAEMFILE;
#else
    return error == EMFILE || error == ENFILE;
#endif
}

string
IceInternal::errorToStringDNS(ErrorCode error)
{
#if defined(_WIN32)
    return IceInternal::errorToString(error);
#else
    return gai_strerror(error);
#endif
}

vector<Address>
IceInternal::getAddresses(const string& host, int port, ProtocolSupport protocol, bool preferIPv6, bool canBlock)
{
    vector<Address> result;

    //
    // We don't use getaddrinfo when host is empty as it's not portable (some old Linux
    // versions don't support it).
    //
    if (host.empty())
    {
        result = getLoopbackAddresses(protocol, port);
        sortAddresses(result, protocol, preferIPv6);
        return result;
    }

    Address addr;
    memset(&addr.saStorage, 0, sizeof(sockaddr_storage));

    struct addrinfo* info = nullptr;
    int retry = 5;

    struct addrinfo hints = {};
    if (protocol == EnableIPv4)
    {
        hints.ai_family = PF_INET;
    }
    else if (protocol == EnableIPv6)
    {
        hints.ai_family = PF_INET6;
    }
    else
    {
        hints.ai_family = PF_UNSPEC;
    }

    if (!canBlock)
    {
        hints.ai_flags = AI_NUMERICHOST;
    }

    int rs = 0;
    do
    {
        rs = getaddrinfo(host.c_str(), nullptr, &hints, &info);
    } while (info == nullptr && rs == EAI_AGAIN && --retry >= 0);

    // In theory, getaddrinfo should only return EAI_NONAME if
    // AI_NUMERICHOST is specified and the host name is not a IP
    // address. However on some platforms (e.g. macOS 10.4.x)
    // EAI_NODATA is also returned so we also check for it.
#ifdef EAI_NODATA
    if (!canBlock && (rs == EAI_NONAME || rs == EAI_NODATA))
#else
    if (!canBlock && rs == EAI_NONAME)
#endif
    {
        return result; // Empty result indicates that a canBlock lookup is necessary.
    }
    else if (rs != 0)
    {
        throw DNSException(__FILE__, __LINE__, rs, host);
    }

    for (struct addrinfo* p = info; p != nullptr; p = p->ai_next)
    {
        memcpy(&addr.saStorage, p->ai_addr, p->ai_addrlen);
        if (p->ai_family == PF_INET)
        {
            addr.saIn.sin_port = htons(port);
        }
        else if (p->ai_family == PF_INET6)
        {
            addr.saIn6.sin6_port = htons(port);
        }

        bool found = false;
        for (const auto& i : result)
        {
            if (compareAddress(i, addr) == 0)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            result.push_back(addr);
        }
    }

    freeaddrinfo(info);

    if (result.empty())
    {
        throw DNSException(__FILE__, __LINE__, 0, host);
    }
    sortAddresses(result, protocol, preferIPv6);
    return result;
}

ProtocolSupport
IceInternal::getProtocolSupport(const Address& addr)
{
    return addr.saStorage.ss_family == AF_INET ? EnableIPv4 : EnableIPv6;
}

Address
IceInternal::getAddressForServer(const string& host, int port, ProtocolSupport protocol, bool preferIPv6, bool canBlock)
{
    //
    // We don't use getaddrinfo when host is empty as it's not portable (some old Linux
    // versions don't support it).
    //
    if (host.empty())
    {
        Address addr;
        memset(&addr.saStorage, 0, sizeof(sockaddr_storage));
        if (protocol != EnableIPv4)
        {
            addr.saIn6.sin6_family = AF_INET6;
            addr.saIn6.sin6_port = htons(port);
            addr.saIn6.sin6_addr = in6addr_any;
        }
        else
        {
            addr.saIn.sin_family = AF_INET;
            addr.saIn.sin_port = htons(port);
            addr.saIn.sin_addr.s_addr = htonl(INADDR_ANY);
        }
        return addr;
    }
    vector<Address> addrs = getAddresses(host, port, protocol, preferIPv6, canBlock);
    return addrs.empty() ? Address() : addrs[0];
}

int
IceInternal::compareAddress(const Address& addr1, const Address& addr2)
{
    if (addr1.saStorage.ss_family < addr2.saStorage.ss_family)
    {
        return -1;
    }
    else if (addr2.saStorage.ss_family < addr1.saStorage.ss_family)
    {
        return 1;
    }

    if (addr1.saStorage.ss_family == AF_INET)
    {
        if (addr1.saIn.sin_port < addr2.saIn.sin_port)
        {
            return -1;
        }
        else if (addr2.saIn.sin_port < addr1.saIn.sin_port)
        {
            return 1;
        }

        if (addr1.saIn.sin_addr.s_addr < addr2.saIn.sin_addr.s_addr)
        {
            return -1;
        }
        else if (addr2.saIn.sin_addr.s_addr < addr1.saIn.sin_addr.s_addr)
        {
            return 1;
        }
    }
    else
    {
        if (addr1.saIn6.sin6_port < addr2.saIn6.sin6_port)
        {
            return -1;
        }
        else if (addr2.saIn6.sin6_port < addr1.saIn6.sin6_port)
        {
            return 1;
        }

        int res = memcmp(&addr1.saIn6.sin6_addr, &addr2.saIn6.sin6_addr, sizeof(in6_addr));
        if (res < 0)
        {
            return -1;
        }
        else if (res > 0)
        {
            return 1;
        }
    }

    return 0;
}

bool
IceInternal::isIPv6Supported()
{
    SOCKET fd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if (fd == INVALID_SOCKET)
    {
        return false;
    }
    else
    {
        closeSocketNoThrow(fd);
        return true;
    }
}

SOCKET
IceInternal::createSocket(bool udp, const Address& addr) { return createSocketImpl(udp, addr.saStorage.ss_family); }
SOCKET
IceInternal::createServerSocket(bool udp, const Address& addr, ProtocolSupport protocol)
{
    SOCKET fd = createSocket(udp, addr);
    if (addr.saStorage.ss_family == AF_INET6 && protocol != EnableIPv4)
    {
        int flag = protocol == EnableIPv6 ? 1 : 0;
        if (setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<char*>(&flag), int(sizeof(int))) == SOCKET_ERROR)
        {
#ifdef _WIN32
            if (getSocketErrno() == WSAENOPROTOOPT)
            {
                return fd; // Windows XP doesn't support IPV6_V6ONLY
            }
#endif
            closeSocketNoThrow(fd);
            throw SocketException(__FILE__, __LINE__, getSocketErrno());
        }
    }
    return fd;
}

void
IceInternal::closeSocketNoThrow(SOCKET fd)
{
#if defined(_WIN32)
    int error = WSAGetLastError();
    closesocket(fd);
    WSASetLastError(error);
#else
    int error = errno;
    close(fd);
    errno = error;
#endif
}

void
IceInternal::closeSocket(SOCKET fd)
{
#if defined(_WIN32)
    int error = WSAGetLastError();
    if (closesocket(fd) == SOCKET_ERROR)
    {
        throw SocketException(__FILE__, __LINE__, getSocketErrno());
    }
    WSASetLastError(error);
#else
    int error = errno;

#    if defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
    //
    // FreeBSD returns ECONNRESET if the underlying object was
    // a stream socket that was shut down by the peer before all
    // pending data was delivered.
    //
    if (close(fd) == SOCKET_ERROR && getSocketErrno() != ECONNRESET)
#    else
    if (close(fd) == SOCKET_ERROR)
#    endif
    {
        throw SocketException(__FILE__, __LINE__, getSocketErrno());
    }
    errno = error;
#endif
}

string
IceInternal::addrToString(const Address& addr)
{
    ostringstream s;
    s << inetAddrToString(addr) << ':' << getPort(addr);
    return s.str();
}

void
IceInternal::fdToLocalAddress(SOCKET fd, Address& addr)
{
    auto len = static_cast<socklen_t>(sizeof(sockaddr_storage));
    if (getsockname(fd, &addr.sa, &len) == SOCKET_ERROR)
    {
        throw SocketException(__FILE__, __LINE__, getSocketErrno());
    }
}

bool
IceInternal::fdToRemoteAddress(SOCKET fd, Address& addr)
{
    auto len = static_cast<socklen_t>(sizeof(sockaddr_storage));
    if (getpeername(fd, &addr.sa, &len) == SOCKET_ERROR)
    {
        if (notConnected())
        {
            return false;
        }
        else
        {
            throw SocketException(__FILE__, __LINE__, getSocketErrno());
        }
    }
    return true;
}

std::string
IceInternal::fdToString(SOCKET fd, const NetworkProxyPtr& proxy, const Address& target)
{
    if (fd == INVALID_SOCKET)
    {
        return "<closed>";
    }

    ostringstream s;

    Address remoteAddr;
    bool peerConnected = fdToRemoteAddress(fd, remoteAddr);

#ifdef _WIN32
    if (!peerConnected)
    {
        //
        // The local address is only accessible with connected sockets on Windows.
        //
        s << "local address = <not available>";
    }
    else
#endif
    {
        Address localAddr;
        fdToLocalAddress(fd, localAddr);
        s << "local address = " << addrToString(localAddr);
    }

    if (proxy)
    {
        if (!peerConnected)
        {
            remoteAddr = proxy->getAddress();
        }
        s << "\n" + proxy->getName() + " proxy address = " << addrToString(remoteAddr);
        s << "\nremote address = " << addrToString(target);
    }
    else
    {
        if (!peerConnected)
        {
            remoteAddr = target;
        }
        s << "\nremote address = " << addrToString(remoteAddr);
    }

    return s.str();
}

std::string
IceInternal::fdToString(SOCKET fd)
{
    if (fd == INVALID_SOCKET)
    {
        return "<closed>";
    }

    Address localAddr;
    fdToLocalAddress(fd, localAddr);

    Address remoteAddr;
    bool peerConnected = fdToRemoteAddress(fd, remoteAddr);

    return addressesToString(localAddr, remoteAddr, peerConnected);
}

void
IceInternal::fdToAddressAndPort(SOCKET fd, string& localAddress, int& localPort, string& remoteAddress, int& remotePort)
{
    if (fd == INVALID_SOCKET)
    {
        localAddress.clear();
        remoteAddress.clear();
        localPort = -1;
        remotePort = -1;
        return;
    }

    Address localAddr;
    fdToLocalAddress(fd, localAddr);
    addrToAddressAndPort(localAddr, localAddress, localPort);

    Address remoteAddr;
    if (fdToRemoteAddress(fd, remoteAddr))
    {
        addrToAddressAndPort(remoteAddr, remoteAddress, remotePort);
    }
    else
    {
        remoteAddress.clear();
        remotePort = -1;
    }
}

void
IceInternal::addrToAddressAndPort(const Address& addr, string& address, int& port)
{
    address = inetAddrToString(addr);
    port = getPort(addr);
}

std::string
IceInternal::addressesToString(const Address& localAddr, const Address& remoteAddr, bool peerConnected)
{
    ostringstream s;
    s << "local address = " << addrToString(localAddr);
    if (peerConnected)
    {
        s << "\nremote address = " << addrToString(remoteAddr);
    }
    else
    {
        s << "\nremote address = <not connected>";
    }
    return s.str();
}

bool
IceInternal::isAddressValid(const Address& addr)
{
    return addr.saStorage.ss_family != AF_UNSPEC;
}

vector<string>
IceInternal::getInterfacesForMulticast(const string& intf, ProtocolSupport protocolSupport)
{
    vector<string> interfaces;
    bool ipv4Wildcard = false;
    if (isWildcard(intf, protocolSupport, ipv4Wildcard))
    {
        vector<Address> addrs = getLocalAddresses(ipv4Wildcard ? EnableIPv4 : protocolSupport);
        for (const auto& addr : addrs)
        {
            interfaces.push_back(inetAddrToString(addr)); // We keep link local addresses for multicast
        }
    }
    if (interfaces.empty())
    {
        interfaces.push_back(intf);
    }
    return interfaces;
}

string
IceInternal::inetAddrToString(const Address& ss)
{
    int size = getAddressStorageSize(ss);
    if (size == 0)
    {
        return "";
    }

    char namebuf[1024];
    namebuf[0] = '\0';
    getnameinfo(
        &ss.sa,
        static_cast<socklen_t>(size),
        namebuf,
        static_cast<socklen_t>(sizeof(namebuf)),
        nullptr,
        0,
        NI_NUMERICHOST);
    return {namebuf};
}

int
IceInternal::getPort(const Address& addr)
{
    if (addr.saStorage.ss_family == AF_INET)
    {
        return ntohs(addr.saIn.sin_port);
    }
    else if (addr.saStorage.ss_family == AF_INET6)
    {
        return ntohs(addr.saIn6.sin6_port);
    }
    else
    {
        return -1;
    }
}

void
IceInternal::setPort(Address& addr, int port)
{
    if (addr.saStorage.ss_family == AF_INET)
    {
        addr.saIn.sin_port = htons(port);
    }
    else
    {
        assert(addr.saStorage.ss_family == AF_INET6);
        addr.saIn6.sin6_port = htons(port);
    }
}

bool
IceInternal::isMulticast(const Address& addr)
{
    if (addr.saStorage.ss_family == AF_INET)
    {
        return IN_MULTICAST(ntohl(addr.saIn.sin_addr.s_addr));
    }
    else if (addr.saStorage.ss_family == AF_INET6)
    {
        return IN6_IS_ADDR_MULTICAST(&addr.saIn6.sin6_addr);
    }
    return false;
}

void
IceInternal::setTcpBufSize(SOCKET fd, const ProtocolInstancePtr& instance)
{
    assert(fd != INVALID_SOCKET);

    //
    // By default, on Windows we use a 128KB buffer size. On Unix
    // platforms, we use the system defaults.
    //
#ifdef _WIN32
    const int dfltBufSize = 128 * 1024;
#else
    const int dfltBufSize = 0;
#endif
    int32_t rcvSize = instance->properties()->getPropertyAsIntWithDefault("Ice.TCP.RcvSize", dfltBufSize);
    int32_t sndSize = instance->properties()->getPropertyAsIntWithDefault("Ice.TCP.SndSize", dfltBufSize);

    setTcpBufSize(fd, rcvSize, sndSize, instance);
}

void
IceInternal::setTcpBufSize(SOCKET fd, int rcvSize, int sndSize, const ProtocolInstancePtr& instance)
{
    assert(fd != INVALID_SOCKET);

    if (rcvSize > 0)
    {
        //
        // Try to set the buffer size. The kernel will silently adjust
        // the size to an acceptable value. Then read the size back to
        // get the size that was actually set.
        //
        setRecvBufferSize(fd, rcvSize);
        int size = getRecvBufferSize(fd);
        if (size > 0 && size < rcvSize)
        {
            // Warn if the size that was set is less than the requested size and
            // we have not already warned.
            BufSizeWarnInfo winfo = instance->getBufSizeWarn(TCPEndpointType);
            if (!winfo.rcvWarn || rcvSize != winfo.rcvSize)
            {
                Ice::Warning out(instance->logger());
                out << "TCP receive buffer size: requested size of " << rcvSize << " adjusted to " << size;
                instance->setRcvBufSizeWarn(TCPEndpointType, rcvSize);
            }
        }
    }

    if (sndSize > 0)
    {
        //
        // Try to set the buffer size. The kernel will silently adjust
        // the size to an acceptable value. Then read the size back to
        // get the size that was actually set.
        //
        setSendBufferSize(fd, sndSize);
        int size = getSendBufferSize(fd);
        if (size > 0 && size < sndSize)
        {
            // Warn if the size that was set is less than the requested size and
            // we have not already warned.
            BufSizeWarnInfo winfo = instance->getBufSizeWarn(TCPEndpointType);
            if (!winfo.sndWarn || sndSize != winfo.sndSize)
            {
                Ice::Warning out(instance->logger());
                out << "TCP send buffer size: requested size of " << sndSize << " adjusted to " << size;
                instance->setSndBufSizeWarn(TCPEndpointType, sndSize);
            }
        }
    }
}

void
IceInternal::setBlock(SOCKET fd, bool block)
{
#ifdef _WIN32
    if (block)
    {
        unsigned long arg = 0;
        if (ioctlsocket(fd, FIONBIO, &arg) == SOCKET_ERROR)
        {
            closeSocketNoThrow(fd);
            throw SocketException(__FILE__, __LINE__, WSAGetLastError());
        }
    }
    else
    {
        unsigned long arg = 1;
        if (ioctlsocket(fd, FIONBIO, &arg) == SOCKET_ERROR)
        {
            closeSocketNoThrow(fd);
            throw SocketException(__FILE__, __LINE__, WSAGetLastError());
        }
    }
#else
    if (block)
    {
        int flags = fcntl(fd, F_GETFL);
        flags &= ~O_NONBLOCK;
        if (fcntl(fd, F_SETFL, flags) == SOCKET_ERROR)
        {
            closeSocketNoThrow(fd);
            throw SocketException(__FILE__, __LINE__, errno);
        }
    }
    else
    {
        int flags = fcntl(fd, F_GETFL);
        flags |= O_NONBLOCK;
        if (fcntl(fd, F_SETFL, flags) == SOCKET_ERROR)
        {
            closeSocketNoThrow(fd);
            throw SocketException(__FILE__, __LINE__, errno);
        }
    }
#endif
}

void
IceInternal::setSendBufferSize(SOCKET fd, int sz)
{
    if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<char*>(&sz), int(sizeof(int))) == SOCKET_ERROR)
    {
        closeSocketNoThrow(fd);
        throw SocketException(__FILE__, __LINE__, getSocketErrno());
    }
}

int
IceInternal::getSendBufferSize(SOCKET fd)
{
    int sz;
    socklen_t len = sizeof(sz);
    if (getsockopt(fd, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<char*>(&sz), &len) == SOCKET_ERROR ||
        static_cast<unsigned int>(len) != sizeof(sz))
    {
        closeSocketNoThrow(fd);
        throw SocketException(__FILE__, __LINE__, getSocketErrno());
    }
    return sz;
}

void
IceInternal::setRecvBufferSize(SOCKET fd, int sz)
{
    if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<char*>(&sz), int(sizeof(int))) == SOCKET_ERROR)
    {
        closeSocketNoThrow(fd);
        throw SocketException(__FILE__, __LINE__, getSocketErrno());
    }
}

int
IceInternal::getRecvBufferSize(SOCKET fd)
{
    int sz;
    socklen_t len = sizeof(sz);
    if (getsockopt(fd, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<char*>(&sz), &len) == SOCKET_ERROR ||
        static_cast<unsigned int>(len) != sizeof(sz))
    {
        closeSocketNoThrow(fd);
        throw SocketException(__FILE__, __LINE__, getSocketErrno());
    }
    return sz;
}

void
IceInternal::setMcastGroup(SOCKET fd, const Address& group, const string& intf)
{
    vector<string> interfaces = getInterfacesForMulticast(intf, getProtocolSupport(group));
    set<int> indexes;
    for (const auto& interface : interfaces)
    {
        int rc = 0;
        if (group.saStorage.ss_family == AF_INET)
        {
            struct ip_mreq mreq;
            mreq.imr_multiaddr = group.saIn.sin_addr;
            mreq.imr_interface = getInterfaceAddress(interface);
            rc = setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, reinterpret_cast<char*>(&mreq), int(sizeof(mreq)));
        }
        else
        {
            int index = getInterfaceIndex(interface);
            if (indexes.find(index) == indexes.end()) // Don't join twice the same interface (if it has multiple IPs)
            {
                indexes.insert(index);
                struct ipv6_mreq mreq;
                mreq.ipv6mr_multiaddr = group.saIn6.sin6_addr;
                mreq.ipv6mr_interface = static_cast<unsigned int>(index);
                rc = setsockopt(fd, IPPROTO_IPV6, IPV6_JOIN_GROUP, reinterpret_cast<char*>(&mreq), int(sizeof(mreq)));
            }
        }
        if (rc == SOCKET_ERROR)
        {
            closeSocketNoThrow(fd);
            throw SocketException(__FILE__, __LINE__, getSocketErrno());
        }
    }
}

void
IceInternal::setMcastInterface(SOCKET fd, const string& intf, const Address& addr)
{
    int rc;
    if (addr.saStorage.ss_family == AF_INET)
    {
        struct in_addr iface = getInterfaceAddress(intf);
        rc = setsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF, reinterpret_cast<char*>(&iface), int(sizeof(iface)));
    }
    else
    {
        int interfaceNum = getInterfaceIndex(intf);
        rc = setsockopt(fd, IPPROTO_IPV6, IPV6_MULTICAST_IF, reinterpret_cast<char*>(&interfaceNum), int(sizeof(int)));
    }
    if (rc == SOCKET_ERROR)
    {
        closeSocketNoThrow(fd);
        throw SocketException(__FILE__, __LINE__, getSocketErrno());
    }
}

void
IceInternal::setMcastTtl(SOCKET fd, int ttl, const Address& addr)
{
    int rc;
    if (addr.saStorage.ss_family == AF_INET)
    {
        rc = setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, reinterpret_cast<char*>(&ttl), int(sizeof(int)));
    }
    else
    {
        rc = setsockopt(fd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, reinterpret_cast<char*>(&ttl), int(sizeof(int)));
    }
    if (rc == SOCKET_ERROR)
    {
        closeSocketNoThrow(fd);
        throw SocketException(__FILE__, __LINE__, getSocketErrno());
    }
}

void
IceInternal::setReuseAddress(SOCKET fd, bool reuse)
{
    int flag = reuse ? 1 : 0;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&flag), int(sizeof(int))) == SOCKET_ERROR)
    {
        closeSocketNoThrow(fd);
        throw SocketException(__FILE__, __LINE__, getSocketErrno());
    }
}

Address
IceInternal::doBind(SOCKET fd, const Address& addr, const string&)
{
    int size = getAddressStorageSize(addr);
    assert(size != 0);

    if (::bind(fd, &addr.sa, static_cast<socklen_t>(size)) == SOCKET_ERROR)
    {
        closeSocketNoThrow(fd);
        throw SocketException(__FILE__, __LINE__, getSocketErrno());
    }

    Address local;
    auto len = static_cast<socklen_t>(sizeof(sockaddr_storage));
    [[maybe_unused]] int ret = getsockname(fd, &local.sa, &len);
    assert(ret != SOCKET_ERROR);
    return local;
}

Address
IceInternal::getNumericAddress(const std::string& address)
{
    vector<Address> addrs = getAddresses(address, 0, EnableBoth, false, false);
    if (addrs.empty())
    {
        return {};
    }
    else
    {
        return addrs[0];
    }
}

string
IceInternal::normalizeIPv6Address(const string& host)
{
    if (host.find(':') != string::npos)
    {
        struct in6_addr result;
        if (inet_pton(AF_INET6, host.c_str(), &result) == 1)
        {
            // Normalize the address
            char buf[INET6_ADDRSTRLEN];
            if (inet_ntop(AF_INET6, &result, buf, sizeof(buf)) != nullptr)
            {
                return string{buf};
            }
            // else conversion to string failed, keep host as is
        }
        // else it's not a valid IPv6 address keep host as is
    }
    return host;
}

SyscallException::ErrorCode
IceInternal::getSocketErrno()
{
#if defined(_WIN32)
    // We standardize on DWORD aka unsigned long for all system error codes on Windows.
    return static_cast<SyscallException::ErrorCode>(WSAGetLastError());
#else
    return errno;
#endif
}

bool
IceInternal::interrupted()
{
#ifdef _WIN32
    return WSAGetLastError() == WSAEINTR;
#else
#    ifdef EPROTO
    return errno == EINTR || errno == EPROTO;
#    else
    return errno == EINTR;
#    endif
#endif
}

bool
IceInternal::acceptInterrupted()
{
    if (interrupted())
    {
        return true;
    }

#ifdef _WIN32
    int error = WSAGetLastError();
    return error == WSAECONNABORTED || error == WSAECONNRESET || error == WSAETIMEDOUT;
#else
    return errno == ECONNABORTED || errno == ECONNRESET || errno == ETIMEDOUT;
#endif
}

bool
IceInternal::noBuffers()
{
#ifdef _WIN32
    int error = WSAGetLastError();
    return error == WSAENOBUFS || error == WSAEFAULT;
#else
    return errno == ENOBUFS;
#endif
}

bool
IceInternal::wouldBlock()
{
#ifdef _WIN32
    int error = WSAGetLastError();
    return error == WSAEWOULDBLOCK || error == WSA_IO_PENDING || error == ERROR_IO_PENDING;
#else
    return errno == EAGAIN || errno == EWOULDBLOCK;
#endif
}

bool
IceInternal::connectFailed()
{
#if defined(_WIN32)
    int error = WSAGetLastError();
    return error == WSAECONNREFUSED || error == WSAETIMEDOUT || error == WSAENETUNREACH || error == WSAEHOSTUNREACH ||
           error == WSAECONNRESET || error == WSAESHUTDOWN || error == WSAECONNABORTED || error == ERROR_SEM_TIMEOUT ||
           error == ERROR_NETNAME_DELETED;
#else
    return errno == ECONNREFUSED || errno == ETIMEDOUT || errno == ENETUNREACH || errno == EHOSTUNREACH ||
           errno == ECONNRESET || errno == ESHUTDOWN || errno == ECONNABORTED;
#endif
}

bool
IceInternal::connectionRefused()
{
#if defined(_WIN32)
    int error = WSAGetLastError();
    return error == WSAECONNREFUSED || error == ERROR_CONNECTION_REFUSED;
#else
    return errno == ECONNREFUSED;
#endif
}

bool
IceInternal::connectionLost()
{
#ifdef _WIN32
    int error = WSAGetLastError();
    return error == WSAECONNRESET || error == WSAESHUTDOWN || error == WSAENOTCONN ||
#    ifdef ICE_USE_IOCP
           error == ERROR_NETNAME_DELETED ||
#    endif
           error == WSAECONNABORTED;
#else
    return errno == ECONNRESET || errno == ENOTCONN || errno == ESHUTDOWN || errno == ECONNABORTED || errno == EPIPE;
#endif
}

bool
IceInternal::connectInProgress()
{
#ifdef _WIN32
    int error = WSAGetLastError();
    return error == WSAEWOULDBLOCK || error == WSA_IO_PENDING || error == ERROR_IO_PENDING;
#else
    return errno == EINPROGRESS;
#endif
}

bool
IceInternal::notConnected()
{
#ifdef _WIN32
    return WSAGetLastError() == WSAENOTCONN;
#elif defined(__APPLE__) || defined(__FreeBSD__)
    return errno == ENOTCONN || errno == EINVAL;
#else
    return errno == ENOTCONN;
#endif
}

bool
IceInternal::recvTruncated()
{
#ifdef _WIN32
    int err = WSAGetLastError();
    return err == WSAEMSGSIZE || err == ERROR_MORE_DATA;
#else
    // We don't get an error under Linux if a datagram is truncated.
    return false;
#endif
}

void
IceInternal::doListen(SOCKET fd, int backlog)
{
repeatListen:
    if (::listen(fd, backlog) == SOCKET_ERROR)
    {
        if (interrupted())
        {
            goto repeatListen;
        }

        closeSocketNoThrow(fd);
        throw SocketException(__FILE__, __LINE__, getSocketErrno());
    }
}

bool
IceInternal::doConnect(SOCKET fd, const Address& addr, const Address& sourceAddr)
{
    if (isAddressValid(sourceAddr))
    {
        doBind(fd, sourceAddr);
    }

repeatConnect:
    int size = getAddressStorageSize(addr);
    assert(size != 0);

    if (::connect(fd, &addr.sa, static_cast<socklen_t>(size)) == SOCKET_ERROR)
    {
        if (interrupted())
        {
            goto repeatConnect;
        }

        if (connectInProgress())
        {
            return false;
        }

        closeSocketNoThrow(fd);
        if (connectionRefused())
        {
            throw ConnectionRefusedException{__FILE__, __LINE__};
        }
        else if (connectFailed())
        {
            throw ConnectFailedException(__FILE__, __LINE__, getSocketErrno());
        }
        else
        {
            throw SocketException(__FILE__, __LINE__, getSocketErrno());
        }
    }

#if defined(__linux__)
    //
    // Prevent self connect (self connect happens on Linux when a client tries to connect to
    // a server which was just deactivated if the client socket re-uses the same ephemeral
    // port as the server).
    //
    Address localAddr;
    try
    {
        fdToLocalAddress(fd, localAddr);
        if (compareAddress(addr, localAddr) == 0)
        {
            throw ConnectionRefusedException{__FILE__, __LINE__};
        }
    }
    catch (const LocalException&)
    {
        closeSocketNoThrow(fd);
        throw;
    }
#endif
    return true;
}

void
IceInternal::doFinishConnect(SOCKET fd)
{
    //
    // Note: we don't close the socket if there's an exception. It's the responsibility
    // of the caller to do so.
    //

    //
    // Strange windows bug: The following call to Sleep() is
    // necessary, otherwise no error is reported through
    // getsockopt.
    //
#if defined(_WIN32)
    Sleep(0);
#endif

    int val;
    auto len = static_cast<socklen_t>(sizeof(int));
    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&val), &len) == SOCKET_ERROR)
    {
        throw SocketException(__FILE__, __LINE__, getSocketErrno());
    }

    if (val > 0)
    {
#if defined(_WIN32)
        WSASetLastError(val);
#else
        errno = val;
#endif
        if (connectionRefused())
        {
            throw ConnectionRefusedException{__FILE__, __LINE__};
        }
        else if (connectFailed())
        {
            throw ConnectFailedException(__FILE__, __LINE__, getSocketErrno());
        }
        else
        {
            throw SocketException(__FILE__, __LINE__, getSocketErrno());
        }
    }

#if defined(__linux__)
    //
    // Prevent self connect (self connect happens on Linux when a client tries to connect to
    // a server which was just deactivated if the client socket re-uses the same ephemeral
    // port as the server).
    //
    Address localAddr;
    fdToLocalAddress(fd, localAddr);
    Address remoteAddr;
    if (fdToRemoteAddress(fd, remoteAddr) && compareAddress(remoteAddr, localAddr) == 0)
    {
        throw ConnectionRefusedException{__FILE__, __LINE__};
    }
#endif
}

SOCKET
IceInternal::doAccept(SOCKET fd)
{
#ifdef _WIN32
    SOCKET ret;
#else
    int ret;
#endif

repeatAccept:
    if ((ret = ::accept(fd, nullptr, nullptr)) == INVALID_SOCKET)
    {
        if (acceptInterrupted())
        {
            goto repeatAccept;
        }

        throw SocketException(__FILE__, __LINE__, getSocketErrno());
    }

    setTcpNoDelay(ret);
    setKeepAlive(ret);
    return ret;
}

void
IceInternal::createPipe(SOCKET fds[2])
{
#ifdef _WIN32

    SOCKET fd = createSocketImpl(false, AF_INET);
    setBlock(fd, true);

    Address addr;
    memset(&addr.saStorage, 0, sizeof(sockaddr_storage));

    addr.saIn.sin_family = AF_INET;
    addr.saIn.sin_port = htons(0);
    addr.saIn.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    addr = doBind(fd, addr);
    doListen(fd, 1);

    try
    {
        fds[0] = createSocketImpl(false, AF_INET);
    }
    catch (...)
    {
        ::closesocket(fd);
        throw;
    }

    try
    {
        setBlock(fds[0], true);
        [[maybe_unused]] bool connected = doConnect(fds[0], addr, Address());
        assert(connected);
    }
    catch (...)
    {
        // fds[0] is closed by doConnect
        ::closesocket(fd);
        throw;
    }

    try
    {
        fds[1] = doAccept(fd);
    }
    catch (...)
    {
        ::closesocket(fds[0]);
        ::closesocket(fd);
        throw;
    }

    ::closesocket(fd);

    try
    {
        setBlock(fds[1], true);
    }
    catch (...)
    {
        ::closesocket(fds[0]);
        // fds[1] is closed by setBlock
        throw;
    }

#else

    if (::pipe(fds) != 0)
    {
        throw SyscallException{__FILE__, __LINE__, "pipe failed", errno};
    }

    try
    {
        setBlock(fds[0], true);
    }
    catch (...)
    {
        // fds[0] is closed by setBlock
        closeSocketNoThrow(fds[1]);
        throw;
    }

    try
    {
        setBlock(fds[1], true);
    }
    catch (...)
    {
        closeSocketNoThrow(fds[0]);
        // fds[1] is closed by setBlock
        throw;
    }

#endif
}

#if defined(ICE_USE_IOCP)
void
IceInternal::doConnectAsync(SOCKET fd, const Address& addr, const Address& sourceAddr, AsyncInfo& info)
{
    //
    // NOTE: It's the caller's responsability to close the socket upon
    // failure to connect. The socket isn't closed by this method.
    //
    Address bindAddr;
    if (isAddressValid(sourceAddr))
    {
        bindAddr = sourceAddr;
    }
    else
    {
        memset(&bindAddr.saStorage, 0, sizeof(sockaddr_storage));
        if (addr.saStorage.ss_family == AF_INET)
        {
            bindAddr.saIn.sin_family = AF_INET;
            bindAddr.saIn.sin_port = htons(0);
            bindAddr.saIn.sin_addr.s_addr = htonl(INADDR_ANY);
        }
        else if (addr.saStorage.ss_family == AF_INET6)
        {
            bindAddr.saIn6.sin6_family = AF_INET6;
            bindAddr.saIn6.sin6_port = htons(0);
            bindAddr.saIn6.sin6_addr = in6addr_any;
        }
    }

    int size = getAddressStorageSize(bindAddr);
    assert(size != 0);

    if (::bind(fd, &bindAddr.sa, size) == SOCKET_ERROR)
    {
        throw SocketException(__FILE__, __LINE__, getSocketErrno());
    }

    LPFN_CONNECTEX ConnectEx = nullptr;   // a pointer to the 'ConnectEx()' function
    GUID GuidConnectEx = WSAID_CONNECTEX; // The Guid
    DWORD dwBytes;
    if (WSAIoctl(
            fd,
            SIO_GET_EXTENSION_FUNCTION_POINTER,
            &GuidConnectEx,
            sizeof(GuidConnectEx),
            &ConnectEx,
            sizeof(ConnectEx),
            &dwBytes,
            nullptr,
            nullptr) == SOCKET_ERROR)
    {
        throw SocketException(__FILE__, __LINE__, getSocketErrno());
    }

    if (!ConnectEx(fd, &addr.sa, size, 0, 0, 0, &info))
    {
        if (!connectInProgress())
        {
            if (connectionRefused())
            {
                throw ConnectionRefusedException{__FILE__, __LINE__};
            }
            else if (connectFailed())
            {
                throw ConnectFailedException(__FILE__, __LINE__, getSocketErrno());
            }
            else
            {
                throw SocketException(__FILE__, __LINE__, getSocketErrno());
            }
        }
    }
}

void
IceInternal::doFinishConnectAsync(SOCKET fd, AsyncInfo& info)
{
    //
    // NOTE: It's the caller's responsability to close the socket upon
    // failure to connect. The socket isn't closed by this method.
    //

    if (info.error != ERROR_SUCCESS)
    {
        WSASetLastError(info.error);
        if (connectionRefused())
        {
            throw ConnectionRefusedException{__FILE__, __LINE__};
        }
        else if (connectFailed())
        {
            throw ConnectFailedException(__FILE__, __LINE__, getSocketErrno());
        }
        else
        {
            throw SocketException(__FILE__, __LINE__, getSocketErrno());
        }
    }

    if (setsockopt(fd, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, nullptr, 0) == SOCKET_ERROR)
    {
        throw SocketException(__FILE__, __LINE__, getSocketErrno());
    }
}
#endif

bool
IceInternal::isIpAddress(const string& name)
{
    in_addr addr;
    in6_addr addr6;

    return inet_pton(AF_INET, name.c_str(), &addr) > 0 || inet_pton(AF_INET6, name.c_str(), &addr6) > 0;
}

bool
IceInternal::isLoopbackOrMulticastAddress(const string& name)
{
    if (name.empty())
    {
        return false;
    }
    else
    {
        in_addr addr;
        in6_addr addr6;
        if (inet_pton(AF_INET, name.c_str(), &addr) > 0)
        {
            // It's an IPv4 address
            return addr.s_addr == htonl(INADDR_LOOPBACK) || IN_MULTICAST(ntohl(addr.s_addr));
        }
        else if (inet_pton(AF_INET6, name.c_str(), &addr6) > 0)
        {
            // It's an IPv6 address
            return IN6_IS_ADDR_LOOPBACK(&addr6) || IN6_IS_ADDR_MULTICAST(&addr6);
        }
        return false;
    }
}

string
IceInternal::getHostName()
{
    char name[256];
    if (gethostname(name, sizeof(name)) != 0)
    {
        throw SocketException{__FILE__, __LINE__, getSocketErrno()};
    }
    return name;
}
