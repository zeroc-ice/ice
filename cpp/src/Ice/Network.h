// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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

#ifdef ICE_OS_WINRT
#   include <Ice/EventHandlerF.h>
#elif defined(_WIN32)
#   include <winsock2.h>
#   include <ws2tcpip.h>
#  if !defined(__MINGW32__)
typedef int ssize_t;
#  endif
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

#if defined(__linux) && !defined(ICE_NO_EPOLL)
#   define ICE_USE_EPOLL 1
#elif (defined(__APPLE__) || defined(__FreeBSD__) || defined(__FreeBSD_kernel__)) && TARGET_OS_IPHONE == 0 && !defined(ICE_NO_KQUEUE)
#   define ICE_USE_KQUEUE 1
#elif defined(__APPLE__) && !defined(ICE_NO_CFSTREAM)
#   define ICE_USE_CFSTREAM 1
#elif defined(_WIN32)
#  if defined(ICE_OS_WINRT)
#  elif !defined(ICE_NO_IOCP)
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
#elif defined(ICE_OS_WINRT)
    typedef Platform::Object^ SOCKET;
#   define INVALID_SOCKET nullptr
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

#if defined(__MINGW32__) && !defined WSAID_CONNECTEX
#  define WSAID_CONNECTEX {0x25a207b9,0xddf3,0x4660,{0x8e,0xe9,0x76,0xe5,0x8c,0x74,0x06,0x3e}}
#  define WSAID_ACCEPTEX {0xb5367df1,0xcbac,0x11cf,{0x95,0xca,0x00,0x80,0x5f,0x48,0xa1,0x92}}
#  define SO_UPDATE_ACCEPT_CONTEXT   0x700B
#  define SO_UPDATE_CONNECT_CONTEXT  0x7010
    typedef BOOL (PASCAL FAR * LPFN_CONNECTEX) (IN SOCKET s, IN const struct sockaddr FAR *name, IN int namelen,
                                                IN PVOID lpSendBuffer OPTIONAL, IN DWORD dwSendDataLength,
                                                OUT LPDWORD lpdwBytesSent, IN LPOVERLAPPED lpOverlapped);

    typedef BOOL (PASCAL FAR * LPFN_ACCEPTEX)(IN SOCKET sListenSocket, IN SOCKET sAcceptSocket,
                                              IN PVOID lpOutputBuffer, IN DWORD dwReceiveDataLength,
                                              IN DWORD dwLocalAddressLength, IN DWORD dwRemoteAddressLength,
                                              OUT LPDWORD lpdwBytesReceived, IN LPOVERLAPPED lpOverlapped);
#endif

namespace IceInternal
{

//
// Use Address struct or union depending on the platform
//
#ifdef ICE_OS_WINRT
struct ICE_API Address
{
    Windows::Networking::HostName^ host;
    Platform::String^ port;
};
#else
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
#endif

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
// On WinRT, wait only for read to return, on IOCP/Win32 wait for
// both pending read and write operations to complete (#ICE-6695).
//
#if defined(ICE_OS_WINRT)
const int SocketOperationWaitForClose = 1;
#elif defined(ICE_USE_IOCP)
const int SocketOperationWaitForClose = 3;
#endif

//
// AsyncInfo struct for Windows IOCP or WinRT holds the result of
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
    int error;
};
#elif defined(ICE_OS_WINRT)
struct ICE_API AsyncInfo
{
    int count;
    int error;
};

delegate void SocketOperationCompletedHandler(int);
#endif

class ICE_API NativeInfo : virtual public IceUtil::Shared
{
public:

    NativeInfo(SOCKET socketFd = INVALID_SOCKET) : _fd(socketFd)
    {
    }

    SOCKET fd() const
    {
        return _fd;
    }

    //
    // This is implemented by transceiver and acceptor implementations.
    //
#if defined(ICE_USE_IOCP)
    virtual AsyncInfo* getAsyncInfo(SocketOperation) = 0;
    void initialize(HANDLE, ULONG_PTR);
    void completed(SocketOperation operation);
#elif defined(ICE_OS_WINRT)
    virtual void setCompletedHandler(SocketOperationCompletedHandler^) = 0;
    void completed(SocketOperation operation);
#endif

protected:

    SOCKET _fd;

#if defined(ICE_USE_IOCP)
    HANDLE _handle;
    ULONG_PTR _key;
#elif defined(ICE_OS_WINRT)
    SocketOperationCompletedHandler^ _completedHandler;
#endif
};
typedef IceUtil::Handle<NativeInfo> NativeInfoPtr;

ICE_API bool noMoreFds(int);
ICE_API std::string errorToStringDNS(int);
ICE_API std::vector<Address> getAddresses(const std::string&, int, ProtocolSupport, Ice::EndpointSelectionType, bool,
                                          bool);
ICE_API ProtocolSupport getProtocolSupport(const Address&);
ICE_API Address getAddressForServer(const std::string&, int, ProtocolSupport, bool);
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

ICE_API Address doBind(SOCKET, const Address&);

#ifndef ICE_OS_WINRT
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

ICE_API void doListen(SOCKET, int);
ICE_API bool doConnect(SOCKET, const Address&, const Address&);
ICE_API void doFinishConnect(SOCKET);
ICE_API SOCKET doAccept(SOCKET);

ICE_API void createPipe(SOCKET fds[2]);

ICE_API int getSocketErrno();

ICE_API Address getNumericAddress(const std::string&);
#else
ICE_API void checkConnectErrorCode(const char*, int, HRESULT, Windows::Networking::HostName^);
ICE_API void checkErrorCode(const char*, int, HRESULT);
#endif

#if defined(ICE_USE_IOCP)
ICE_API void doConnectAsync(SOCKET, const Address&, const Address&, AsyncInfo&);
ICE_API void doFinishConnectAsync(SOCKET, AsyncInfo&);
#endif
}

#endif
