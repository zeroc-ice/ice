//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_NETWORK_H
#define ICE_NETWORK_H

#ifdef __hpux
#   define _XOPEN_SOURCE_EXTENDED
#endif

#include <Ice/Config.h>

#include <Ice/NetworkF.h>
#include <Ice/NetworkProxyF.h>
#include <Ice/PropertiesF.h> // For setTcpBufSize
#include <Ice/LoggerF.h> // For setTcpBufSize
#include <Ice/Protocol.h>
#include <Ice/ProtocolInstanceF.h>
#include <Ice/EndpointTypes.h>

#if defined(_WIN32)
#   include <winsock2.h>
#   include <ws2tcpip.h>
typedef int ssize_t;
#else
#   include <unistd.h>
#   include <fcntl.h>
#   include <sys/socket.h>
#   include <sys/poll.h>
#   include <netinet/in.h>
#   include <netinet/tcp.h>
#   include <arpa/inet.h>
#   include <netdb.h>
#endif

#if defined(__linux__) && !defined(ICE_NO_EPOLL)
#   define ICE_USE_EPOLL 1
#elif (defined(__APPLE__) || defined(__FreeBSD__) || defined(__FreeBSD_kernel__)) && TARGET_OS_IPHONE == 0 && !defined(ICE_NO_KQUEUE)
#   define ICE_USE_KQUEUE 1
#elif defined(__APPLE__) && !defined(ICE_NO_CFSTREAM)
#   define ICE_USE_CFSTREAM 1
#elif defined(_WIN32)
#  if !defined(ICE_NO_IOCP)
#     define ICE_USE_IOCP 1
#  else
#     define ICE_USE_SELECT 1
#  endif
#else
#   define ICE_USE_POLL 1
#endif

#if defined(_WIN32) || defined(__osf__)
typedef int socklen_t;
#endif

#if !defined(_WIN32)
#   define SOCKET int
#   define INVALID_SOCKET -1
#   define SOCKET_ERROR -1
#endif

#ifndef SHUT_RD
#   define SHUT_RD 0
#endif

#ifndef SHUT_WR
#   define SHUT_WR 1
#endif

#ifndef SHUT_RDWR
#   define SHUT_RDWR 2
#endif

#ifndef NETDB_INTERNAL
#   define NETDB_INTERNAL -1
#endif

#ifndef NETDB_SUCCESS
#   define NETDB_SUCCESS 0
#endif

namespace IceInternal
{

union Address
{
    Address()
    {
        memset(&saStorage, 0, sizeof(sockaddr_storage));
        saStorage.ss_family = AF_UNSPEC;
    }

    sockaddr sa;
    sockaddr_in saIn;
    sockaddr_in6 saIn6;
    sockaddr_storage saStorage;
};

enum SocketOperation
{
    SocketOperationNone = 0,
    SocketOperationRead = 1,
    // With BSD sockets, write and connect readiness are the same so
    // we use the same value for both.
    SocketOperationWrite = 2,
#ifdef ICE_USE_CFSTREAM
    SocketOperationConnect = 4
#else
    SocketOperationConnect = 2
#endif
};

//
// AsyncInfo struct for Windows IOCP holds the result of
// asynchronous operations after it completed.
//
#if defined(ICE_USE_IOCP)
struct ICE_API AsyncInfo : WSAOVERLAPPED
{
    AsyncInfo(SocketOperation);

    SocketOperation status;
    WSABUF buf;
    DWORD flags;
    DWORD count;
    DWORD error;
};
#endif

class ICE_API ReadyCallback : public virtual ::IceUtil::Shared
{
public:

    virtual ~ReadyCallback();

    virtual void ready(SocketOperation, bool) = 0;
};
typedef IceUtil::Handle<ReadyCallback> ReadyCallbackPtr;

class ICE_API NativeInfo : public virtual IceUtil::Shared
{
public:

    virtual ~NativeInfo();

    NativeInfo(SOCKET socketFd = INVALID_SOCKET) : _fd(socketFd)
#if !defined(ICE_USE_IOCP)
        , _newFd(INVALID_SOCKET)
#endif
    {
    }

    SOCKET fd() const
    {
        return _fd;
    }

    void setReadyCallback(const ReadyCallbackPtr& callback);

    void ready(SocketOperation operation, bool value)
    {
        assert(_readyCallback);
        _readyCallback->ready(operation, value);
    }

    //
    // This is implemented by transceiver and acceptor implementations.
    //
#if defined(ICE_USE_IOCP)
    virtual AsyncInfo* getAsyncInfo(SocketOperation) = 0;
    void initialize(HANDLE, ULONG_PTR);
    void completed(SocketOperation);
#else
    bool newFd();
    void setNewFd(SOCKET);
#endif

protected:

    SOCKET _fd;
    ReadyCallbackPtr _readyCallback;

#if defined(ICE_USE_IOCP)
    HANDLE _handle;
    ULONG_PTR _key;
#else
    SOCKET _newFd;
#endif

private:
};
typedef IceUtil::Handle<NativeInfo> NativeInfoPtr;

ICE_API bool noMoreFds(int);
ICE_API std::string errorToStringDNS(int);
ICE_API std::vector<Address> getAddresses(const std::string&, int, ProtocolSupport, Ice::EndpointSelectionType, bool,
                                          bool);
ICE_API ProtocolSupport getProtocolSupport(const Address&);
ICE_API Address getAddressForServer(const std::string&, int, ProtocolSupport, bool, bool);
ICE_API int compareAddress(const Address&, const Address&);

ICE_API bool isIPv6Supported();
ICE_API SOCKET createSocket(bool, const Address&);
ICE_API SOCKET createServerSocket(bool, const Address&, ProtocolSupport);
ICE_API void closeSocketNoThrow(SOCKET);
ICE_API void closeSocket(SOCKET);

ICE_API std::string addrToString(const Address&);
ICE_API void fdToLocalAddress(SOCKET, Address&);
ICE_API bool fdToRemoteAddress(SOCKET, Address&);
ICE_API std::string fdToString(SOCKET, const NetworkProxyPtr&, const Address&);
ICE_API std::string fdToString(SOCKET);
ICE_API void fdToAddressAndPort(SOCKET, std::string&, int&, std::string&, int&);
ICE_API void addrToAddressAndPort(const Address&, std::string&, int&);
ICE_API std::string addressesToString(const Address&, const Address&, bool);
ICE_API bool isAddressValid(const Address&);

ICE_API std::vector<std::string> getHostsForEndpointExpand(const std::string&, ProtocolSupport, bool);
ICE_API std::vector<std::string> getInterfacesForMulticast(const std::string&, ProtocolSupport);

ICE_API std::string inetAddrToString(const Address&);
ICE_API int getPort(const Address&);
ICE_API void setPort(Address&, int);

ICE_API bool isMulticast(const Address&);
ICE_API void setTcpBufSize(SOCKET, const ProtocolInstancePtr&);
ICE_API void setTcpBufSize(SOCKET, int, int, const ProtocolInstancePtr&);

ICE_API void setBlock(SOCKET, bool);
ICE_API void setSendBufferSize(SOCKET, int);
ICE_API int getSendBufferSize(SOCKET);
ICE_API void setRecvBufferSize(SOCKET, int);
ICE_API int getRecvBufferSize(SOCKET);

ICE_API void setMcastGroup(SOCKET, const Address&, const std::string&);
ICE_API void setMcastInterface(SOCKET, const std::string&, const Address&);
ICE_API void setMcastTtl(SOCKET, int, const Address&);
ICE_API void setReuseAddress(SOCKET, bool);
ICE_API Address doBind(SOCKET, const Address&, const std::string& intf = "");
ICE_API void doListen(SOCKET, int);

ICE_API bool interrupted();
ICE_API bool acceptInterrupted();
ICE_API bool noBuffers();
ICE_API bool wouldBlock();
ICE_API bool notConnected();
ICE_API bool recvTruncated();

ICE_API bool connectFailed();
ICE_API bool connectionRefused();
ICE_API bool connectInProgress();
ICE_API bool connectionLost();

ICE_API bool doConnect(SOCKET, const Address&, const Address&);
ICE_API void doFinishConnect(SOCKET);
ICE_API SOCKET doAccept(SOCKET);

ICE_API void createPipe(SOCKET fds[2]);

ICE_API int getSocketErrno();

ICE_API Address getNumericAddress(const std::string&);

#if defined(ICE_USE_IOCP)
ICE_API void doConnectAsync(SOCKET, const Address&, const Address&, AsyncInfo&);
ICE_API void doFinishConnectAsync(SOCKET, AsyncInfo&);
#endif

ICE_API bool isIpAddress(const std::string&);

}

#endif
