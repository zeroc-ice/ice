// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Network.h>
#include <Ice/LoggerUtil.h>

#include <IceSSL/OpenSSLPluginI.h>
#include <IceSSL/SslConnector.h>
#include <IceSSL/SslTransceiver.h>
#include <IceSSL/TraceLevels.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

TransceiverPtr
IceSSL::SslConnector::connect(int timeout)
{
    if(_traceLevels->network >= 2)
    {
	Trace out(_logger, _traceLevels->networkCat);
	out << "trying to establish ssl connection to " << toString();
    }

    SOCKET fd = createSocket(false);
    setBlock(fd, false);
    doConnect(fd, _addr, timeout);

    if(_traceLevels->network >= 1)
    {
	Trace out(_logger, _traceLevels->networkCat);
	out << "ssl connection established\n" << fdToString(fd);
    }

    return _plugin->createClientTransceiver(
	static_cast<int>(fd), timeout);
}

string
IceSSL::SslConnector::toString() const
{
    return addrToString(_addr);
}

IceSSL::SslConnector::SslConnector(const OpenSSLPluginIPtr& plugin, const string& host, int port) :
    _plugin(plugin),
    _traceLevels(plugin->getTraceLevels()),
    _logger(plugin->getLogger())
{
    getAddress(host, port, _addr);
}

IceSSL::SslConnector::~SslConnector()
{
}
