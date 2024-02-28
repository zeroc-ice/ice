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

class ICE_API IncomingBase
{
public:

    Ice::OutputStream* startWriteParams();
    void endWriteParams();
    void writeEmptyParams();
    void writeParamEncaps(const std::uint8_t*, std::int32_t, bool);
    void setMarshaledResult(const Ice::MarshaledResult&);

    void response(bool);
    void exception(std::exception_ptr, bool);

protected:

    IncomingBase(Instance*, ResponseHandler*, Ice::Connection*, const Ice::ObjectAdapterPtr&, bool, Ice::Byte, std::int32_t);
    IncomingBase(IncomingBase&);
    IncomingBase(const IncomingBase&) = delete;

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

    //
    // Optimization. The request handler may not be deleted while a
    // stack-allocated Incoming still holds it.
    //
    ResponseHandler* _responseHandler;
    using DispatchInterceptorCallbacks = std::deque<std::pair<std::function<bool()>,
                                                              std::function<bool(std::exception_ptr)>>>;
    DispatchInterceptorCallbacks _interceptorCBs;
};

class ICE_API Incoming final : public IncomingBase
{
public:

    Incoming(Instance*, ResponseHandler*, Ice::Connection*, const Ice::ObjectAdapterPtr&, bool, Ice::Byte, std::int32_t);

    const Ice::Current& getCurrent()
    {
        return _current;
    }

    void push(std::function<bool()>, std::function<bool(std::exception_ptr)>);
    void pop();

    void setAsync(const IncomingAsyncPtr& in)
    {
        assert(!_inAsync);
        _inAsync = in;
    }

    void startOver();

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

private:

    friend class IncomingAsync;

    Ice::InputStream* _is;
    std::uint8_t* _inParamPos;

    IncomingAsyncPtr _inAsync;
};

}

#endif
