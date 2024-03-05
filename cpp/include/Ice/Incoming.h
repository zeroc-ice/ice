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
#include <Ice/IncomingAsyncF.h>
#include <Ice/ObserverHelper.h>
#include <Ice/ResponseHandlerF.h>

#include <deque>

namespace Ice
{

/**
 * Base class for marshaled result structures, which are generated for operations having the
 * marshaled-result metadata tag.
 * \headerfile Ice/Ice.h
 */
class ICE_API MarshaledResult
{
public:

    /**
     * The constructor requires the Current object that was passed to the servant.
     */
    MarshaledResult(const Current& current);

    /**
     * Obtains the output stream that is used to marshal the results.
     * @return The output stream.
     */
    std::shared_ptr<OutputStream> getOutputStream() const
    {
        return ostr;
    }

protected:

    /** The output stream used to marshal the results. */
    std::shared_ptr<OutputStream> ostr;
};

}

namespace Ice
{
    class ServantLocator;
}

namespace IceInternal
{

class ICE_API IncomingBase : public std::enable_shared_from_this<IncomingBase>
{
public:

    IncomingBase(Instance*, ResponseHandlerPtr, Ice::Connection*, const Ice::ObjectAdapterPtr&, bool, std::uint8_t, std::int32_t);

    Ice::OutputStream* startWriteParams();
    void endWriteParams();
    void writeEmptyParams();
    void writeParamEncaps(const std::uint8_t*, std::int32_t, bool);
    void setMarshaledResult(const Ice::MarshaledResult&);

    void response(bool);
    void exception(std::exception_ptr, bool);

    const Ice::Current& getCurrent()
    {
        return _current;
    }

    void setAsync(const IncomingAsyncPtr& in)
    {
        assert(!_inAsync);
        _inAsync = in;
    }

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

    std::function<void()> response()
    {
        return [self = shared_from_this()]
        {
            self->writeEmptyParams();
            self->completed();
        };
    }

    template<class T>
    std::function<void(const T&)> response()
    {
        return [self = shared_from_this()](const T& marshaledResult)
        {
            self->setMarshaledResult(marshaledResult);
            self->completed();
        };
    }

    std::function<void(std::exception_ptr)> exception()
    {
        return [self = shared_from_this()](std::exception_ptr ex) { self->completed(ex); };
    }

    void kill(IncomingBase&);

    void completed();

    void completed(std::exception_ptr);

protected:

    IncomingBase(IncomingBase&);
    IncomingBase(const IncomingBase&) = delete;

    friend class IncomingAsync;

private:

    void checkResponseSent();

    void warning(const Ice::Exception&) const;
    void warning(std::exception_ptr) const;

    bool servantLocatorFinished(bool);

    void handleException(std::exception_ptr, bool);

    Ice::Current _current;
    std::shared_ptr<Ice::Object> _servant;
    std::shared_ptr<Ice::ServantLocator> _locator;
    ::std::shared_ptr<void> _cookie;
    DispatchObserver _observer;
    bool _response;
    std::uint8_t _compress;
    Ice::FormatType _format;
    Ice::OutputStream _os;

    ResponseHandlerPtr _responseHandler;

    // _is points to an object allocated on the stack of the dispatch thread.
    Ice::InputStream* _is;
    IncomingAsyncPtr _inAsync;

    std::atomic_flag _responseSent = ATOMIC_FLAG_INIT;
};

}

#endif
