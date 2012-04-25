// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/EndpointI.h>
#include <Ice/Instance.h>
#include <Ice/LocalException.h>
#include <Ice/Network.h>
#include <Ice/PropertiesI.h>
#include <Ice/LoggerUtil.h>
#include <IceUtil/MutexPtrLock.h>

using namespace std;
using namespace IceInternal;

namespace
{

IceUtil::Mutex* hashMutex = 0;

class Init
{
public:

    Init()
    {
        hashMutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete hashMutex;
        hashMutex = 0;
    }
};

Init init;

}

Ice::LocalObject* IceInternal::upCast(EndpointI* p) { return p; }
IceUtil::Shared* IceInternal::upCast(EndpointHostResolver* p) { return p; }

const Ice::EncodingVersion&
IceInternal::EndpointI::encoding() const
{
    return _encoding;
}

const Ice::ProtocolVersion&
IceInternal::EndpointI::protocol() const
{
    return _protocol;
}

Ice::Int
IceInternal::EndpointI::ice_getHash() const
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(hashMutex);
    if(!_hashInitialized)
    {
        _hashValue = hashInit();
    }
    return _hashValue;
}

vector<ConnectorPtr>
IceInternal::EndpointI::connectors(const vector<struct sockaddr_storage>& addrs) const
{
    //
    // This method must be extended by endpoints which use the EndpointHostResolver to create
    // connectors from IP addresses.
    //
    assert(false);
    return vector<ConnectorPtr>();
}

IceInternal::EndpointI::EndpointI(const Ice::ProtocolVersion& protocol, const Ice::EncodingVersion& encoding) : 
    _protocol(protocol),
    _encoding(encoding), 
    _hashInitialized(false)
{
}

IceInternal::EndpointI::EndpointI() : 
    _protocol(Ice::currentProtocol),
    _encoding(Ice::currentEncoding), 
    _hashInitialized(false) 
{
}

void
IceInternal::EndpointI::parseOption(const string& option, const string& arg, const string& desc, const string& str)
{
    if(option == "-v")
    {
        if(arg.empty())
        {
            Ice::EndpointParseException ex(__FILE__, __LINE__);
            ex.str = "no argument provided for -v option in endpoint `" + desc + " " + str + "'";
            throw ex;
        }

        try 
        {
            const_cast<Ice::ProtocolVersion&>(_protocol) = Ice::stringToProtocolVersion(arg);
        }
        catch(const Ice::VersionParseException& e)
        {
            Ice::EndpointParseException ex(__FILE__, __LINE__);
            ex.str = "invalid protocol version `" + arg + "' in endpoint `" + desc + " " + str + "':\n" + e.str;
            throw ex;
        }
    }
    else if(option == "-e")
    {
        if(arg.empty())
        {
            Ice::EndpointParseException ex(__FILE__, __LINE__);
            ex.str = "no argument provided for -e option in endpoint `" + desc + " " + str + "'";
            throw ex;
        }

        try 
        {
            const_cast<Ice::EncodingVersion&>(_encoding) = Ice::stringToEncodingVersion(arg);
        }
        catch(const Ice::VersionParseException& e)
        {
            Ice::EndpointParseException ex(__FILE__, __LINE__);
            ex.str = "invalid encoding version `" + arg + "' in endpoint `" + desc + " " + str + "':\n" + e.str;
            throw ex;
        }
    }
    else 
    {
        Ice::EndpointParseException ex(__FILE__, __LINE__);
        ex.str = "unknown option `" + option + "' in `" + desc + " " + str + "'";
        throw ex;
    }
}

IceInternal::EndpointHostResolver::EndpointHostResolver(const InstancePtr& instance) :
    IceUtil::Thread("Ice endpoint host resolver thread"),
    _instance(instance),
    _destroyed(false)
{
    __setNoDelete(true);
    try
    {
        bool hasPriority = _instance->initializationData().properties->getProperty("Ice.ThreadPriority") != "";
        int priority = _instance->initializationData().properties->getPropertyAsInt("Ice.ThreadPriority");
        if(hasPriority)
        {
            start(0, priority);
        }
        else
        {
            start();
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        {
            Ice::Error out(_instance->initializationData().logger);
            out << "cannot create thread for enpoint host resolver:\n" << ex;
        }
        throw;
    }
    __setNoDelete(false);
}

void
IceInternal::EndpointHostResolver::resolve(const string& host, int port, const EndpointIPtr& endpoint,
                                           const EndpointI_connectorsPtr& callback)
{ 
    //
    // Try to get the addresses without DNS lookup. If this doesn't work, we queue a resolve
    // entry and the thread will take care of getting the endpoint addresses.
    //
    try
    {
        vector<struct sockaddr_storage> addrs = getAddresses(host, port, _instance->protocolSupport(), false);
        if(!addrs.empty())
        {
            callback->connectors(endpoint->connectors(addrs));
            return;
        }
    }
    catch(const Ice::LocalException& ex)
    {
        callback->exception(ex);
        return;
    }

    Lock sync(*this);
    assert(!_destroyed);

    ResolveEntry entry;
    entry.host = host;
    entry.port = port;
    entry.endpoint = endpoint;
    entry.callback = callback;
    _queue.push_back(entry);
    notify();
}

void
IceInternal::EndpointHostResolver::destroy()
{
    Lock sync(*this);
    assert(!_destroyed);
    _destroyed = true;
    notify();
}

void
IceInternal::EndpointHostResolver::run()
{
    while(true)
    {
        ResolveEntry resolve;

        {
            Lock sync(*this);

            while(!_destroyed && _queue.empty())
            {
                wait();
            }

            if(_destroyed)
            {
                break;
            }

            resolve = _queue.front();
            _queue.pop_front();
        }

        try
        {
            resolve.callback->connectors(
                resolve.endpoint->connectors(
                    getAddresses(resolve.host, resolve.port, _instance->protocolSupport(), true)));
        }
        catch(const Ice::LocalException& ex)
        {
            resolve.callback->exception(ex);
        }
    }

    for(deque<ResolveEntry>::const_iterator p = _queue.begin(); p != _queue.end(); ++p)
    {
        p->callback->exception(Ice::CommunicatorDestroyedException(__FILE__, __LINE__));
    }
    _queue.clear();
}
