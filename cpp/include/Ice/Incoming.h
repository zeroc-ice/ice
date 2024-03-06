//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_INCOMING_H
#define ICE_INCOMING_H

#include <Ice/InstanceF.h>
#include <Ice/ConnectionIF.h>
#include <Ice/ServantManagerF.h>
#include <Ice/OutputStream.h>
#include <Ice/InputStream.h>
#include <Ice/Object.h>
#include <Ice/Current.h>
#include <Ice/ObserverHelper.h>
#include <Ice/ResponseHandlerF.h>

#include "MarshaledResult.h"

#include <atomic>
#include <deque> // TODO: remove

namespace Ice
{
    class ServantLocator;
}

namespace IceInternal
{

class ICE_API Incoming
{
public:

    Incoming(Instance*, ResponseHandlerPtr, Ice::Connection*, const Ice::ObjectAdapterPtr&, bool, std::uint8_t, std::int32_t);
    Incoming(Incoming&&);
    Incoming(const Incoming&) = delete;
    Incoming& operator=(const Incoming&) = delete;

    Ice::OutputStream* startWriteParams();
    void endWriteParams();
    void writeEmptyParams();
    void writeParamEncaps(const std::uint8_t*, std::int32_t, bool ok);
    void setMarshaledResult(const Ice::MarshaledResult&);

    void response(bool amd);
    void exception(std::exception_ptr, bool amd);

    void setFormat(Ice::FormatType format)
    {
        _format = format;
    }

    void invoke(const ServantManagerPtr&, Ice::InputStream*);

    // Inlined for speed optimization.
    void skipReadParams()
    {
        _current.encoding = _is->skipEncapsulation();
    }
    Ice::InputStream* startReadParams()
    {
        //
        // Remember the encoding used by the input parameters, we'll
        // encode the response parameters with the same encoding.
        //
        _current.encoding = _is->startEncapsulation();
        return _is;
    }
    void endReadParams() const
    {
        _is->endEncapsulation();
    }
    void readEmptyParams()
    {
        _current.encoding = _is->skipEmptyEncapsulation();
    }
    void readParamEncaps(const std::uint8_t*& v, std::int32_t& sz)
    {
        _current.encoding = _is->readEncapsulation(v, sz);
    }

    const Ice::Current& current() const { return _current; }

    // Async dispatch writes an empty response and completes successfully.
    void response();

    // Async dispatch writes a marshaled result and completes successfully.
    void response(const Ice::MarshaledResult& marshaledResult);

    // Async dispatch completes successfully. Call this function after writing the response.
    void completed();

    // Async dispatch completes with an exception. This can throw, for example, if the response has already been sent.
    void completed(std::exception_ptr ex);

    // Handle an exception that was thrown by an async dispatch. Use this function when in the dispatch thread.
    void failed(std::exception_ptr) noexcept;

private:

    void setResponseSent();

    void warning(const Ice::Exception&) const;
    void warning(std::exception_ptr) const;

    bool servantLocatorFinished(bool);

    void handleException(std::exception_ptr, bool);

    Ice::Current _current;
    std::shared_ptr<Ice::Object> _servant;
    std::shared_ptr<Ice::ServantLocator> _locator;
    ::std::shared_ptr<void> _cookie;
    DispatchObserver _observer;
    bool _isTwoWay;
    std::uint8_t _compress;
    Ice::FormatType _format;
    Ice::OutputStream _os;

    ResponseHandlerPtr _responseHandler;

    // _is points to an object allocated on the stack of the dispatch thread.
    Ice::InputStream* _is;

    // This flag is set when the user calls an async response or exception callback. A second call is incorrect and
    // results in ResponseSentException.
    // We don't need an atomic flag since it's purely for error detection - something that should never happen in
    // correctly written code.
    bool _responseSent = false;

    // This flag is set when an async dispatch has completed, for example because the connection dropped. A subsequent
    // call is no-op (succeeds silently).
    std::atomic_flag _completed = ATOMIC_FLAG_INIT;
};

}

#endif
