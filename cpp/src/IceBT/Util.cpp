// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceBT/Util.h>

#include <Ice/LocalException.h>
#include <Ice/Network.h>
#include <IceUtil/StringUtil.h>

using namespace std;
using namespace Ice;
using namespace IceBT;

bool
IceBT::isValidDeviceAddress(const string& str)
{
    //
    // Format is 01:23:45:67:89:0A
    //

    if(str.size() != 17 || str[2] != ':' || str[5] != ':' || str[8] != ':' || str[11] != ':' || str[14] != ':')
    {
        return false;
    }

    for(string::size_type i = 0; i < str.size(); i += 3)
    {
        if(!isxdigit(str[i]) || !isxdigit(str[i + 1]))
        {
            return false;
        }
    }

    return true;
}

bool
IceBT::parseDeviceAddress(const string& str, DeviceAddress& addr)
{
    uint8_t b0, b1, b2, b3, b4, b5;

    if(isValidDeviceAddress(str) &&
       sscanf(str.c_str(), "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx", &b5, &b4, &b3, &b2, &b1, &b0) == 6)
    {
        addr.b[0] = b0;
        addr.b[1] = b1;
        addr.b[2] = b2;
        addr.b[3] = b3;
        addr.b[4] = b4;
        addr.b[5] = b5;
        return true;
    }

    return false;
}

string
IceBT::formatDeviceAddress(const DeviceAddress& addr)
{
    char buf[64];
    sprintf(buf, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx", addr.b[5], addr.b[4], addr.b[3], addr.b[2], addr.b[1],
            addr.b[0]);
    return IceUtilInternal::toUpper(string(buf));
}

string
IceBT::addrToString(const string& addr, Int channel)
{
    ostringstream ostr;
    ostr << addr << '#' << channel;
    return ostr.str();
}

string
IceBT::addrToString(const SocketAddress& addr)
{
    return addrToString(formatDeviceAddress(addr.rc_bdaddr), addr.rc_channel);
}

SocketAddress
IceBT::createAddr(const string& addr, Ice::Int channel)
{
    SocketAddress ret;
    ret.rc_family = AF_BLUETOOTH;
    ret.rc_channel = static_cast<uint8_t>(channel);
    parseDeviceAddress(addr, ret.rc_bdaddr);
    return ret;
}

SOCKET
IceBT::createSocket()
{
    SOCKET fd = ::socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    if(fd == INVALID_SOCKET)
    {
        SocketException ex(__FILE__, __LINE__);
        ex.error = errno;
        throw ex;
    }
    return fd;
}

bool
IceBT::doConnect(SOCKET fd, const SocketAddress& addr)
{
    int size = sizeof(SocketAddress);
    assert(size != 0);

repeatConnect:
    if(::connect(fd, reinterpret_cast<const struct sockaddr*>(&addr), size) == SOCKET_ERROR)
    {
        if(IceInternal::interrupted())
        {
            goto repeatConnect;
        }

        if(IceInternal::connectInProgress())
        {
            return false;
        }

        IceInternal::closeSocketNoThrow(fd);
        if(IceInternal::connectionRefused())
        {
            ConnectionRefusedException ex(__FILE__, __LINE__);
            ex.error = IceInternal::getSocketErrno();
            throw ex;
        }
        else if(IceInternal::connectFailed())
        {
            ConnectFailedException ex(__FILE__, __LINE__);
            ex.error = IceInternal::getSocketErrno();
            throw ex;
        }
        else
        {
            SocketException ex(__FILE__, __LINE__);
            ex.error = IceInternal::getSocketErrno();
            throw ex;
        }
    }
    return true;
}

void
IceBT::doFinishConnect(SOCKET fd)
{
    //
    // Note: we don't close the socket if there's an exception. It's the responsibility
    // of the caller to do so.
    //

    int val;
    socklen_t len = static_cast<socklen_t>(sizeof(int));
    if(::getsockopt(fd, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&val), &len) == SOCKET_ERROR)
    {
        SocketException ex(__FILE__, __LINE__);
        ex.error = IceInternal::getSocketErrno();
        throw ex;
    }

    if(val > 0)
    {
        errno = val;
        if(IceInternal::connectionRefused())
        {
            ConnectionRefusedException ex(__FILE__, __LINE__);
            ex.error = IceInternal::getSocketErrno();
            throw ex;
        }
        else if(IceInternal::connectFailed())
        {
            ConnectFailedException ex(__FILE__, __LINE__);
            ex.error = IceInternal::getSocketErrno();
            throw ex;
        }
        else
        {
            SocketException ex(__FILE__, __LINE__);
            ex.error = IceInternal::getSocketErrno();
            throw ex;
        }
    }
}

SocketAddress
IceBT::doBind(SOCKET fd, const SocketAddress& addr)
{
    if(addr.rc_channel <= 0)
    {
        //
        // Find an available by channel by looping over the valid channel numbers (1-30) until bind succeeds.
        //
        SocketAddress tmp = addr;
        for(tmp.rc_channel = 1; tmp.rc_channel <= 30; ++tmp.rc_channel)
        {
            if(::bind(fd, reinterpret_cast<const struct sockaddr*>(&tmp), sizeof(SocketAddress)) == 0)
            {
                break;
            }
        }
        if(tmp.rc_channel > 30)
        {
            IceInternal::closeSocketNoThrow(fd);
            SocketException ex(__FILE__, __LINE__);
            ex.error = errno;
            throw ex;
        }
    }
    else
    {
        if(::bind(fd, reinterpret_cast<const struct sockaddr*>(&addr), sizeof(SocketAddress)) == SOCKET_ERROR)
        {
            IceInternal::closeSocketNoThrow(fd);
            SocketException ex(__FILE__, __LINE__);
            ex.error = errno;
            throw ex;
        }
    }

    SocketAddress local;
    socklen_t len = static_cast<socklen_t>(sizeof(SocketAddress));
#  ifdef NDEBUG
    getsockname(fd, reinterpret_cast<struct sockaddr*>(&local), &len);
#  else
    int ret = getsockname(fd, reinterpret_cast<struct sockaddr*>(&local), &len);
    assert(ret != SOCKET_ERROR);
#  endif
    return local;
}

SOCKET
IceBT::doAccept(SOCKET fd)
{
    int ret;

repeatAccept:
    if((ret = ::accept(fd, 0, 0)) == INVALID_SOCKET)
    {
        if(IceInternal::acceptInterrupted())
        {
            goto repeatAccept;
        }

        SocketException ex(__FILE__, __LINE__);
        ex.error = errno;
        throw ex;
    }

    return ret;
}

namespace
{

void
fdToLocalAddress(SOCKET fd, SocketAddress& addr)
{
    socklen_t len = static_cast<socklen_t>(sizeof(sockaddr_rc));
    if(::getsockname(fd, reinterpret_cast<struct sockaddr*>(&addr), &len) == SOCKET_ERROR)
    {
        IceInternal::closeSocketNoThrow(fd);
        SocketException ex(__FILE__, __LINE__);
        ex.error = IceInternal::getSocketErrno();
        throw ex;
    }
}

bool
fdToRemoteAddress(SOCKET fd, SocketAddress& addr)
{
    socklen_t len = static_cast<socklen_t>(sizeof(sockaddr_rc));
    if(::getpeername(fd, reinterpret_cast<struct sockaddr*>(&addr), &len) == SOCKET_ERROR)
    {
        if(IceInternal::notConnected())
        {
            return false;
        }
        else
        {
            IceInternal::closeSocketNoThrow(fd);
            SocketException ex(__FILE__, __LINE__);
            ex.error = IceInternal::getSocketErrno();
            throw ex;
        }
    }

    return true;
}

string
addressesToString(const SocketAddress& localAddr, const SocketAddress& remoteAddr, bool peerConnected)
{
    ostringstream s;
    s << "local address = " << addrToString(localAddr);
    if(peerConnected)
    {
        s << "\nremote address = " << addrToString(remoteAddr);
    }
    else
    {
        s << "\nremote address = <not connected>";
    }
    return s.str();
}

void
addrToAddressAndChannel(const SocketAddress& addr, string& address, int& channel)
{
    address = formatDeviceAddress(addr.rc_bdaddr);
    channel = addr.rc_channel;
}

}

string
IceBT::fdToString(SOCKET fd)
{
    if(fd == INVALID_SOCKET)
    {
        return "<closed>";
    }

    SocketAddress localAddr;
    fdToLocalAddress(fd, localAddr);

    SocketAddress remoteAddr;
    bool peerConnected = fdToRemoteAddress(fd, remoteAddr);

    return addressesToString(localAddr, remoteAddr, peerConnected);
}

void
IceBT::fdToAddressAndChannel(SOCKET fd, string& localAddress, int& localChannel, string& remoteAddress,
                             int& remoteChannel)
{
    if(fd == INVALID_SOCKET)
    {
        localAddress.clear();
        remoteAddress.clear();
        localChannel = -1;
        remoteChannel = -1;
        return;
    }

    SocketAddress localAddr;
    fdToLocalAddress(fd, localAddr);
    addrToAddressAndChannel(localAddr, localAddress, localChannel);

    SocketAddress remoteAddr;
    if(fdToRemoteAddress(fd, remoteAddr))
    {
        addrToAddressAndChannel(remoteAddr, remoteAddress, remoteChannel);
    }
    else
    {
        remoteAddress.clear();
        remoteChannel = -1;
    }
}

int
IceBT::compareAddress(const SocketAddress& addr1, const SocketAddress& addr2)
{
    if(addr1.rc_family < addr2.rc_family)
    {
        return -1;
    }
    else if(addr2.rc_family < addr1.rc_family)
    {
        return 1;
    }

    if(addr1.rc_channel < addr2.rc_channel)
    {
        return -1;
    }
    else if(addr2.rc_channel < addr1.rc_channel)
    {
        return 1;
    }

    return ::memcmp(&addr1.rc_bdaddr, &addr2.rc_bdaddr, sizeof(bdaddr_t));
}
