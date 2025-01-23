// Copyright (c) ZeroC, Inc.

#ifndef BLOBJECT_H
#define BLOBJECT_H

#include "Ice/Ice.h"
#include "Instance.h"

namespace Glacier2
{
    class Blobject : public Ice::BlobjectArrayAsync, public std::enable_shared_from_this<Blobject>
    {
    public:
        Blobject(std::shared_ptr<Instance>, Ice::ConnectionPtr, Ice::Context);

    protected:
        void invoke(
            Ice::ObjectPrx&,
            std::pair<const std::byte*, const std::byte*>,
            std::function<void(bool, std::pair<const std::byte*, const std::byte*>)>,
            std::function<void(std::exception_ptr)>,
            const Ice::Current&);

        const std::shared_ptr<Instance> _instance;
        const Ice::ConnectionPtr _reverseConnection;

    private:
        const bool _forwardContext;
        const int _requestTraceLevel;
        const Ice::Context _context;
    };
}

#endif
