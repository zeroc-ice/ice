// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

// Note: This pragma is used to disable spurious warning messages having
//       to do with the length of debug symbols exceeding 255 characters.
//       This is due to STL template identifiers expansion.
//       The MSDN Library recommends that you put this pragma directive
//       in place to avoid the warnings.
#ifdef _WIN32
#   pragma warning(disable:4786)
#endif

#include <Ice/Instance.h>
#include <Ice/TraceLevels.h>
#include <Ice/Logger.h>
#include <Ice/Network.h>
#include <Ice/Properties.h>
#include <Ice/Exception.h>

#include <IceSSL/PluginBaseI.h>
#include <IceSSL/SslConnector.h>
#include <IceSSL/SslTransceiver.h>
#include <IceSSL/Exception.h>

#include <sstream>

using namespace std;
using namespace Ice;
using namespace IceInternal;

TransceiverPtr
IceSSL::SslConnector::connect(int timeout)
{
    if (_traceLevels->network >= 2)
    {
	ostringstream s;
	s << "trying to establish ssl connection to " << toString();
	_logger->trace(_traceLevels->networkCat, s.str());
    }

    SOCKET fd = createSocket(false);
    setBlock(fd, false);
    doConnect(fd, _addr, timeout);

    if (_traceLevels->network >= 1)
    {
	ostringstream s;
	s << "ssl connection established\n" << fdToString(fd);
	_logger->trace(_traceLevels->networkCat, s.str());
    }

    // Get the SSL plug-in
    PluginManagerPtr pluginManager = _instance->pluginManager();
    Ice::PluginPtr plugin = pluginManager->getPlugin("IceSSL");
    PluginBaseIPtr sslPlugin = PluginBaseIPtr::dynamicCast(plugin);
    assert(sslPlugin);

    IceSSL::ConnectionPtr connection = sslPlugin->createConnection(IceSSL::Client, fd);
    TransceiverPtr transPtr = new SslTransceiver(_instance, fd, connection);

    return transPtr;
}

string
IceSSL::SslConnector::toString() const
{
    return addrToString(_addr);
}

IceSSL::SslConnector::SslConnector(const InstancePtr& instance, const string& host, int port) :
    _instance(instance),
    _traceLevels(instance->traceLevels()),
    _logger(instance->logger())
{
    getAddress(host.c_str(), port, _addr);
}

IceSSL::SslConnector::~SslConnector()
{
}
