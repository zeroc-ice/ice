//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef BLOBJECT_H
#define BLOBJECT_H

#include <Ice/Ice.h>
#include <Glacier2/RequestQueue.h>
#include <Glacier2/Instance.h>

namespace Glacier2
{

class Blobject : public Ice::BlobjectArrayAsync, public std::enable_shared_from_this<Blobject>
{
public:

    Blobject(std::shared_ptr<Instance>, std::shared_ptr<Ice::Connection>, const Ice::Context&);

    void destroy();

    virtual void updateObserver(const std::shared_ptr<Instrumentation::SessionObserver>&);

    void invokeException(std::exception_ptr, std::function<void(std::exception_ptr)>&&);

protected:

    void invoke(std::shared_ptr<Ice::ObjectPrx>&,
                const std::pair<const Ice::Byte*, const Ice::Byte*>&,
                std::function<void(bool, const std::pair<const Ice::Byte*, const Ice::Byte*>&)>,
                std::function<void(std::exception_ptr)>,
                const Ice::Current&);

    const std::shared_ptr<Instance> _instance;
    const std::shared_ptr<Ice::Connection> _reverseConnection;

private:

    const bool _forwardContext;
    const int _requestTraceLevel;
    const int _overrideTraceLevel;
    const std::shared_ptr<RequestQueue> _requestQueue;
    const Ice::Context _context;
};

}

#endif
