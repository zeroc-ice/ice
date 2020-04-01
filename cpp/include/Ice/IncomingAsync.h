//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_INCOMING_ASYNC_H
#define ICE_INCOMING_ASYNC_H

#include <Ice/IncomingAsyncF.h>
#include <Ice/Incoming.h>

namespace IceInternal
{

// TODO: fix this warning
#if defined(_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable:4239)
#endif

//
// We need virtual inheritance from AMDCallback, because we use multiple
// inheritance from Ice::AMDCallback for generated AMD code.
//
class ICE_API IncomingAsync : public IncomingBase,
    public ::std::enable_shared_from_this<IncomingAsync>
{
public:

    IncomingAsync(Incoming&);

    static std::shared_ptr<IncomingAsync> create(Incoming&);

    std::function<void()> response()
    {
        auto self = shared_from_this();
        return [self]()
        {
            self->writeEmptyParams();
            self->completed();
        };
    }

    template<class T>
    std::function<void(const T&)> response()
    {
        auto self = shared_from_this();
        return [self](const T& marshaledResult)
        {
            self->setMarshaledResult(marshaledResult);
            self->completed();
        };
    }

    std::function<void(std::exception_ptr)> exception()
    {
        auto self = shared_from_this();
        return [self](std::exception_ptr ex) { self->completed(ex); };
    }

    void kill(Incoming&);

    void completed();

    void completed(std::exception_ptr);

private:

    void checkResponseSent();
    bool _responseSent;

    //
    // We need a separate ConnectionIPtr, because IncomingBase only
    // holds a ConnectionI* for optimization.
    //
    const ResponseHandlerPtr _responseHandlerCopy;
};

#if defined(_MSC_VER)
#   pragma warning(pop)
#endif

}

#endif
