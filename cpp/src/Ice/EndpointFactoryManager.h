//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_ENDPOINT_FACTORY_MANAGER_H
#define ICE_ENDPOINT_FACTORY_MANAGER_H

#include "EndpointFactoryF.h"
#include "EndpointFactoryManagerF.h"
#include "EndpointIF.h"
#include "Ice/Config.h"
#include "Ice/InstanceF.h"

#include <mutex>
#include <vector>

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
        EndpointFactoryPtr get(std::int16_t) const;
        EndpointIPtr create(const std::string&, bool) const;
        EndpointIPtr read(Ice::InputStream*) const;

    private:
        friend class Instance;

        InstancePtr _instance;
        std::vector<EndpointFactoryPtr> _factories;
        mutable std::mutex _mutex;
    };
}

#endif
