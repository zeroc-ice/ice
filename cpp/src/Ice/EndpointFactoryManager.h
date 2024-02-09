//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_ENDPOINT_FACTORY_MANAGER_H
#define ICE_ENDPOINT_FACTORY_MANAGER_H

#include <Ice/Config.h>
#include <Ice/InstanceF.h>
#include <Ice/EndpointIF.h>
#include <Ice/EndpointFactoryF.h>
#include <Ice/EndpointFactoryManagerF.h>

#include <mutex>

namespace Ice
{

class InputStream;

}

namespace IceInternal
{

class EndpointFactoryManager
{
public:

    EndpointFactoryManager(const InstancePtr&);
    void initialize() const;
    void add(const EndpointFactoryPtr&);
    EndpointFactoryPtr get(::Ice::Short) const;
    EndpointIPtr create(const std::string&, bool) const;
    EndpointIPtr read(Ice::InputStream*) const;

private:

    void destroy();
    friend class Instance;

    InstancePtr _instance;
    std::vector<EndpointFactoryPtr> _factories;
    mutable std::mutex _mutex;
};

}

#endif
