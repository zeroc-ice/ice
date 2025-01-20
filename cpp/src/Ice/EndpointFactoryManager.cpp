// Copyright (c) ZeroC, Inc.

#include "EndpointFactoryManager.h"
#include "DefaultsAndOverrides.h"
#include "Ice/InputStream.h"
#include "Ice/LocalExceptions.h"
#include "Ice/OutputStream.h"
#include "Ice/StringUtil.h"
#include "Instance.h"
#include "OpaqueEndpointI.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::EndpointFactoryManager::EndpointFactoryManager(InstancePtr instance) : _instance(std::move(instance)) {}

void
IceInternal::EndpointFactoryManager::initialize() const
{
    for (const auto& fact : _factories)
    {
        fact->initialize();
    }
}

void
IceInternal::EndpointFactoryManager::add(const EndpointFactoryPtr& factory)
{
    lock_guard lock(_mutex);
    //
    // TODO: Optimize with a map?
    //
    for (const auto& fact : _factories)
    {
        if (fact->type() == factory->type())
        {
            assert(false); // TODO: Exception?
        }
    }
    _factories.push_back(factory);
}

EndpointFactoryPtr
IceInternal::EndpointFactoryManager::get(int16_t type) const
{
    lock_guard lock(_mutex);
    //
    // TODO: Optimize with a map?
    //
    for (const auto& fact : _factories)
    {
        if (fact->type() == type)
        {
            return fact;
        }
    }
    return nullptr;
}

EndpointIPtr
IceInternal::EndpointFactoryManager::create(string_view str, bool oaEndpoint) const
{
    vector<string> v;
    bool b = IceInternal::splitString(str, " \t\n\r", v);
    if (!b)
    {
        throw ParseException(__FILE__, __LINE__, "mismatched quotes in endpoint '" + string{str} + "'");
    }

    if (v.empty())
    {
        throw ParseException(__FILE__, __LINE__, "endpoint '" + string{str} + "' has no non-whitespace characters");
    }

    string protocol = v.front();
    v.erase(v.begin());

    if (protocol == "default")
    {
        protocol = _instance->defaultsAndOverrides()->defaultProtocol;
    }

    EndpointFactoryPtr factory;
    {
        lock_guard lock(_mutex);
        //
        // TODO: Optimize with a map?
        //
        for (const auto& fact : _factories)
        {
            if (fact->protocol() == protocol)
            {
                factory = fact;
            }
        }
    }

    if (factory)
    {
        EndpointIPtr e = factory->create(v, oaEndpoint);
        if (!v.empty())
        {
            throw ParseException(
                __FILE__,
                __LINE__,
                "unrecognized argument '" + v.front() + "' in endpoint '" + string{str} + "'");
        }
        return e;
    }

    //
    // If the stringified endpoint is opaque, create an unknown endpoint,
    // then see whether the type matches one of the known endpoints.
    //
    if (protocol == "opaque")
    {
        EndpointIPtr ue = make_shared<OpaqueEndpointI>(v);
        if (!v.empty())
        {
            throw ParseException(
                __FILE__,
                __LINE__,
                "unrecognized argument '" + v.front() + "' in endpoint '" + string{str} + "'");
        }
        factory = get(ue->type());
        if (factory)
        {
            //
            // Make a temporary stream, write the opaque endpoint data into the stream,
            // and ask the factory to read the endpoint data from that stream to create
            // the actual endpoint.
            //
            OutputStream bs;
            bs.write(ue->type());
            ue->streamWrite(&bs);
            InputStream is{_instance.get(), bs.getEncoding(), bs, false};
            short type;
            is.read(type);
            is.startEncapsulation();
            EndpointIPtr e = factory->read(&is);
            is.endEncapsulation();
            return e;
        }
        return ue; // Endpoint is opaque, but we don't have a factory for its type.
    }

    return nullptr;
}

EndpointIPtr
IceInternal::EndpointFactoryManager::read(InputStream* s) const
{
    int16_t type;
    s->read(type);

    EndpointFactoryPtr factory = get(type);
    EndpointIPtr e;

    s->startEncapsulation();

    if (factory)
    {
        e = factory->read(s);
    }
    //
    // If the factory failed to read the endpoint, return an opaque endpoint. This can
    // occur if for example the factory delegates to another factory and this factory
    // isn't available. In this case, the factory needs to make sure the stream position
    // is preserved for reading the opaque endpoint.
    //
    if (!e)
    {
        e = make_shared<OpaqueEndpointI>(type, s);
    }

    s->endEncapsulation();

    return e;
}
