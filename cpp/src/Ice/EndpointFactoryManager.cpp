// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/EndpointFactoryManager.h>
#include <Ice/Endpoint.h>
#include <Ice/OpaqueEndpointI.h>
#include <Ice/OutputStream.h>
#include <Ice/InputStream.h>
#include <Ice/LocalException.h>
#include <Ice/Instance.h>
#include <Ice/Properties.h>
#include <Ice/DefaultsAndOverrides.h>
#include <IceUtil/StringUtil.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(EndpointFactoryManager* p) { return p; }

IceInternal::EndpointFactoryManager::EndpointFactoryManager(const InstancePtr& instance)
    : _instance(instance)
{
}

void
IceInternal::EndpointFactoryManager::add(const EndpointFactoryPtr& factory)
{
    IceUtil::Mutex::Lock sync(*this); // TODO: Necessary?

    //
    // TODO: Optimize with a map?
    //
    for(vector<EndpointFactoryPtr>::size_type i = 0; i < _factories.size(); i++)
    {
        if(_factories[i]->type() == factory->type())
        {
            assert(false); // TODO: Exception?
        }
    }
    _factories.push_back(factory);
}

EndpointFactoryPtr
IceInternal::EndpointFactoryManager::get(Short type) const
{
    IceUtil::Mutex::Lock sync(*this); // TODO: Necessary?

    //
    // TODO: Optimize with a map?
    //
    for(vector<EndpointFactoryPtr>::size_type i = 0; i < _factories.size(); i++)
    {
        if(_factories[i]->type() == type)
        {
            return _factories[i];
        }
    }
    return ICE_NULLPTR;
}

EndpointIPtr
IceInternal::EndpointFactoryManager::create(const string& str, bool oaEndpoint) const
{
    vector<string> v;
    bool b = IceUtilInternal::splitString(str, " \t\n\r", v);
    if(!b)
    {
        EndpointParseException ex(__FILE__, __LINE__);
        ex.str = "mismatched quote";
        throw ex;
    }

    if(v.empty())
    {
        EndpointParseException ex(__FILE__, __LINE__);
        ex.str = "value has no non-whitespace characters";
        throw ex;
    }

    string protocol = v.front();
    v.erase(v.begin());

    if(protocol == "default")
    {
        protocol = _instance->defaultsAndOverrides()->defaultProtocol;
    }

    EndpointFactoryPtr factory;
    {
        IceUtil::Mutex::Lock sync(*this); // TODO: Necessary?

        //
        // TODO: Optimize with a map?
        //
        for(vector<EndpointFactoryPtr>::size_type i = 0; i < _factories.size(); i++)
        {
            if(_factories[i]->protocol() == protocol)
            {
                factory = _factories[i];
            }
        }
    }

    if(factory)
    {
#if 1
        EndpointIPtr e = factory->create(v, oaEndpoint);
        if(!v.empty())
        {
            EndpointParseException ex(__FILE__, __LINE__);
            ex.str = "unrecognized argument `" + v.front() + "' in endpoint `" + str + "'";
            throw ex;
        }
        return e;
#else
        // Code below left in place for debugging.

        EndpointIPtr e = factory->create(str.substr(end), oaEndpoint);
        OutputStream bs(_instance.get(), Ice::currentProtocolEncoding);
        e->streamWrite(&bs);
        bs.i = bs.b.begin();
        short type;
        bs.read(type);
        EndpointIPtr ue = new IceInternal::OpaqueEndpointI(type, &bs);
        consoleErr << "Normal: " << e->toString() << endl;
        consoleErr << "Opaque: " << ue->toString() << endl;
        return e;
#endif
    }

    //
    // If the stringified endpoint is opaque, create an unknown endpoint,
    // then see whether the type matches one of the known endpoints.
    //
    if(protocol == "opaque")
    {
        EndpointIPtr ue = ICE_MAKE_SHARED(OpaqueEndpointI, v);
        if(!v.empty())
        {
            EndpointParseException ex(__FILE__, __LINE__);
            ex.str = "unrecognized argument `" + v.front() + "' in endpoint `" + str + "'";
            throw ex;
        }
        factory = get(ue->type());
        if(factory)
        {
            //
            // Make a temporary stream, write the opaque endpoint data into the stream,
            // and ask the factory to read the endpoint data from that stream to create
            // the actual endpoint.
            //
            OutputStream bs(_instance.get(), Ice::currentProtocolEncoding);
            bs.write(ue->type());
            ue->streamWrite(&bs);
            InputStream is(bs.instance(), bs.getEncoding(), bs);
            short type;
            is.read(type);
            is.startEncapsulation();
            EndpointIPtr e = factory->read(&is);
            is.endEncapsulation();
            return e;
        }
        return ue; // Endpoint is opaque, but we don't have a factory for its type.
    }

    return ICE_NULLPTR;
}

EndpointIPtr
IceInternal::EndpointFactoryManager::read(InputStream* s) const
{
    Short type;
    s->read(type);

    EndpointFactoryPtr factory = get(type);
    EndpointIPtr e;

    s->startEncapsulation();

    if(factory)
    {
        e = factory->read(s);
    }
    else
    {
        e = ICE_MAKE_SHARED(OpaqueEndpointI, type, s);
    }

    s->endEncapsulation();

    return e;
}

void
IceInternal::EndpointFactoryManager::destroy()
{
    for(vector<EndpointFactoryPtr>::size_type i = 0; i < _factories.size(); i++)
    {
        _factories[i]->destroy();
    }
    _factories.clear();
}
