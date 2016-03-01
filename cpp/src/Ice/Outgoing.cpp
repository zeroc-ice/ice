// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Outgoing.h>
#include <Ice/ConnectionI.h>
#include <Ice/CollocatedRequestHandler.h>
#include <Ice/Reference.h>
#include <Ice/Instance.h>
#include <Ice/LocalException.h>
#include <Ice/ReplyStatus.h>
#include <Ice/ImplicitContextI.h>

using namespace std;
using namespace IceUtil;
using namespace Ice;
using namespace Ice::Instrumentation;
using namespace IceInternal;

OutgoingBase::OutgoingBase(Instance* instance) : _os(instance, Ice::currentProtocolEncoding), _sent(false)
{
}

ProxyOutgoingBase::ProxyOutgoingBase(IceProxy::Ice::Object* proxy, OperationMode mode) :
    OutgoingBase(proxy->__reference()->getInstance().get()),
    _proxy(proxy),
    _mode(mode),
    _state(StateUnsent)
{
    int invocationTimeout = _proxy->__reference()->getInvocationTimeout();
    if(invocationTimeout > 0)
    {
        _invocationTimeoutDeadline = Time::now(Time::Monotonic) + Time::milliSeconds(invocationTimeout);
    }
}

ProxyOutgoingBase::~ProxyOutgoingBase()
{
}

void
ProxyOutgoingBase::sent()
{
    Monitor<Mutex>::Lock sync(_monitor);
    if(_proxy->__reference()->getMode() != Reference::ModeTwoway)
    {
        _childObserver.detach();
        _state = StateOK;
    }
    _sent = true;
    _monitor.notify();

    //
    // NOTE: At this point the stack allocated ProxyOutgoingBase object can be destroyed
    // since the notify() on the monitor will release the thread waiting on the
    // synchronous Ice call.
    //
}

void
ProxyOutgoingBase::completed(const Ice::Exception& ex)
{
    Monitor<Mutex>::Lock sync(_monitor);
    //assert(_state <= StateInProgress);
    if(_state > StateInProgress)
    {
        //
        // Response was already received but message
        // didn't get removed first from the connection
        // send message queue so it's possible we can be
        // notified of failures. In this case, ignore the
        // failure and assume the outgoing has been sent.
        //
        assert(_state != StateFailed);
        _sent = true;
        _monitor.notify();
        return;
    }

    _childObserver.failed(ex.ice_name());
    _childObserver.detach();

    _state = StateFailed;
    _exception.reset(ex.ice_clone());
    _monitor.notify();
}

void
ProxyOutgoingBase::completed(BasicStream& is)
{
    assert(false); // Must be overriden
}

void
ProxyOutgoingBase::retryException(const Ice::Exception&)
{
    Monitor<Mutex>::Lock sync(_monitor);
    assert(_state <= StateInProgress);
    _state = StateRetry;
    _monitor.notify();
}

bool
ProxyOutgoingBase::invokeImpl()
{
    assert(_state == StateUnsent);

    const int invocationTimeout = _proxy->__reference()->getInvocationTimeout();
    int cnt = 0;
    while(true)
    {
        try
        {
            if(invocationTimeout > 0 && _invocationTimeoutDeadline <= Time::now(Time::Monotonic))
            {
                throw Ice::InvocationTimeoutException(__FILE__, __LINE__);
            }

            _state = StateInProgress;
            _exception.reset(0);
            _sent = false;

            _handler = _proxy->__getRequestHandler();

            if(_handler->sendRequest(this)) // Request sent and no response expected, we're done.
            {
                return true;
            }

            if(invocationTimeout == -2) // Use the connection timeout
            {
                try
                {
                    _invocationTimeoutDeadline = Time(); // Reset any previously set value

                    int timeout = _handler->waitForConnection()->timeout();
                    if(timeout > 0)
                    {
                        _invocationTimeoutDeadline = Time::now(Time::Monotonic) + Time::milliSeconds(timeout);
                    }
                }
                catch(const Ice::LocalException&)
                {
                }
            }

            bool timedOut = false;
            {
                Monitor<Mutex>::Lock sync(_monitor);
                //
                // If the handler says it's not finished, we wait until we're done.
                //
                if(_invocationTimeoutDeadline != Time())
                {
                    Time now = Time::now(Time::Monotonic);
                    timedOut = now >= _invocationTimeoutDeadline;
                    while((_state == StateInProgress || !_sent) && _state != StateFailed && _state != StateRetry)
                    {
                        if(timedOut)
                        {
                            break;
                        }
                        _monitor.timedWait(_invocationTimeoutDeadline - now);

                        if((_state == StateInProgress || !_sent) && _state != StateFailed)
                        {
                            now = Time::now(Time::Monotonic);
                            timedOut = now >= _invocationTimeoutDeadline;
                        }
                    }
                }
                else
                {
                    while((_state == StateInProgress || !_sent) && _state != StateFailed && _state != StateRetry)
                    {
                        _monitor.wait();
                    }
                }
            }

            if(timedOut)
            {
                if(invocationTimeout == -2)
                {
                    _handler->requestCanceled(this, ConnectionTimeoutException(__FILE__, __LINE__));
                }
                else
                {
                    _handler->requestCanceled(this, InvocationTimeoutException(__FILE__, __LINE__));
                }

                //
                // Wait for the exception to propagate. It's possible the request handler ignores
                // the timeout if there was a failure shortly before requestCanceled got called.
                // In this case, the exception should be set on the ProxyOutgoingBase.
                //
                Monitor<Mutex>::Lock sync(_monitor);
                while(_state == StateInProgress)
                {
                    _monitor.wait();
                }
            }

            if(_exception.get())
            {
                _exception->ice_throw();
            }
            else if(_state == StateRetry)
            {
                _proxy->__updateRequestHandler(_handler, 0); // Clear request handler and retry.
                continue;
            }
            else
            {
                assert(_state != StateInProgress);
                return _state == StateOK;
            }
        }
        catch(const RetryException&)
        {
            _proxy->__updateRequestHandler(_handler, 0); // Clear request handler and retry.
        }
        catch(const Ice::Exception& ex)
        {
            try
            {
                Time interval;
                interval = Time::milliSeconds(_proxy->__handleException(ex, _handler, _mode, _sent, cnt));
                if(interval > Time())
                {
                    if(invocationTimeout > 0)
                    {
                        IceUtil::Time now = Time::now(Time::Monotonic);
                        IceUtil::Time retryDeadline = now + interval;

                        //
                        // Wait until either the retry and invocation timeout deadline is reached.
                        // Note that we're using a loop here because sleep() precision isn't as
                        // good as the motonic clock and it can return few hundred micro-seconds
                        // earlier which breaks the check for the invocation timeout.
                        //
                        while(retryDeadline > now && _invocationTimeoutDeadline > now)
                        {
                            if(retryDeadline < _invocationTimeoutDeadline)
                            {
                                ThreadControl::sleep(retryDeadline - now);
                            }
                            else if(_invocationTimeoutDeadline > now)
                            {
                                ThreadControl::sleep(_invocationTimeoutDeadline - now);
                            }
                            now = Time::now(Time::Monotonic);
                        }
                        if(now >= _invocationTimeoutDeadline)
                        {
                            throw Ice::InvocationTimeoutException(__FILE__, __LINE__);
                        }
                    }
                    else
                    {
                        ThreadControl::sleep(interval);
                    }
                }
                _observer.retried();
            }
            catch(const Ice::Exception& ex)
            {
                _observer.failed(ex.ice_name());
                throw;
            }
        }
    }

    assert(false);
    return false;
}

Outgoing::Outgoing(IceProxy::Ice::Object* proxy, const string& operation, OperationMode mode, const Context* context) :
    ProxyOutgoingBase(proxy, mode),
    _encoding(getCompatibleEncoding(proxy->__reference()->getEncoding())),
    _is(proxy->__reference()->getInstance().get(), Ice::currentProtocolEncoding),
    _operation(operation)
{
    checkSupportedProtocol(getCompatibleProtocol(proxy->__reference()->getProtocol()));
    _observer.attach(proxy, operation, context);

    switch(_proxy->__reference()->getMode())
    {
        case Reference::ModeTwoway:
        case Reference::ModeOneway:
        case Reference::ModeDatagram:
        {
            _os.writeBlob(requestHdr, sizeof(requestHdr));
            break;
        }

        case Reference::ModeBatchOneway:
        case Reference::ModeBatchDatagram:
        {
            _proxy->__getBatchRequestQueue()->prepareBatchRequest(&_os);
            break;
        }
    }

    try
    {
        _os.write(_proxy->__reference()->getIdentity());

        //
        // For compatibility with the old FacetPath.
        //
        if(_proxy->__reference()->getFacet().empty())
        {
            _os.write(static_cast<string*>(0), static_cast<string*>(0));
        }
        else
        {
            string facet = _proxy->__reference()->getFacet();
            _os.write(&facet, &facet + 1);
        }

        _os.write(operation, false);

        _os.write(static_cast<Ice::Byte>(mode));

        if(context != 0)
        {
            //
            // Explicit context
            //
            _os.write(*context);
        }
        else
        {
            //
            // Implicit context
            //
            const ImplicitContextIPtr& implicitContext = _proxy->__reference()->getInstance()->getImplicitContext();
            const Context& prxContext = _proxy->__reference()->getContext()->getValue();
            if(implicitContext == 0)
            {
                _os.write(prxContext);
            }
            else
            {
                implicitContext->write(prxContext, &_os);
            }
        }
    }
    catch(const LocalException& ex)
    {
        abort(ex);
    }
}

Outgoing::~Outgoing()
{
}

bool
Outgoing::invokeRemote(const Ice::ConnectionIPtr& connection, bool compress, bool response)
{
    return connection->sendRequest(this, compress, response, 0);
}

void
Outgoing::invokeCollocated(CollocatedRequestHandler* handler)
{
    handler->invokeRequest(this, 0);
}

bool
Outgoing::invoke()
{
    const Reference::Mode mode = _proxy->__reference()->getMode();
    if(mode == Reference::ModeBatchOneway || mode == Reference::ModeBatchDatagram)
    {
        _state = StateInProgress;
        _proxy->__getBatchRequestQueue()->finishBatchRequest(&_os, _proxy, _operation);
        return true;
    }
    return invokeImpl();
}

void
Outgoing::abort(const LocalException& ex)
{
    assert(_state == StateUnsent);

    //
    // If we didn't finish a batch oneway or datagram request, we must
    // notify the connection about that we give up ownership of the
    // batch stream.
    //
    const Reference::Mode mode = _proxy->__reference()->getMode();
    if(mode == Reference::ModeBatchOneway || mode == Reference::ModeBatchDatagram)
    {
        _proxy->__getBatchRequestQueue()->abortBatchRequest(&_os);
    }

    ex.ice_throw();
}

void
Outgoing::completed(BasicStream& is)
{
    Monitor<Mutex>::Lock sync(_monitor);

    assert(_proxy->__reference()->getMode() == Reference::ModeTwoway); // Can only be called for twoways.

    assert(_state <= StateInProgress);
    if(_childObserver)
    {
        _childObserver->reply(static_cast<Int>(is.b.size() - headerSize - 4));
    }
    _childObserver.detach();

    _is.swap(is);

    Ice::Byte replyStatus;
    _is.read(replyStatus);

    switch(replyStatus)
    {
        case replyOK:
        {
            _state = StateOK; // The state must be set last, in case there is an exception.
            break;
        }

        case replyUserException:
        {
            _observer.userException();
            _state = StateUserException; // The state must be set last, in case there is an exception.
            break;
        }

        case replyObjectNotExist:
        case replyFacetNotExist:
        case replyOperationNotExist:
        {
            //
            // Don't read the exception members directly into the
            // exception. Otherwise if reading fails and raises an
            // exception, you will have a memory leak.
            //
            Identity ident;
            _is.read(ident);

            //
            // For compatibility with the old FacetPath.
            //
            vector<string> facetPath;
            _is.read(facetPath);
            string facet;
            if(!facetPath.empty())
            {
                if(facetPath.size() > 1)
                {
                    throw MarshalException(__FILE__, __LINE__);
                }
                facet.swap(facetPath[0]);
            }

            string operation;
            _is.read(operation, false);

            RequestFailedException* ex;
            switch(replyStatus)
            {
                case replyObjectNotExist:
                {
                    ex = new ObjectNotExistException(__FILE__, __LINE__);
                    break;
                }

                case replyFacetNotExist:
                {
                    ex = new FacetNotExistException(__FILE__, __LINE__);
                    break;
                }

                case replyOperationNotExist:
                {
                    ex = new OperationNotExistException(__FILE__, __LINE__);
                    break;
                }

                default:
                {
                    ex = 0; // To keep the compiler from complaining.
                    assert(false);
                    break;
                }
            }

            ex->id = ident;
            ex->facet = facet;
            ex->operation = operation;
            _exception.reset(ex);

            _state = StateLocalException; // The state must be set last, in case there is an exception.
            break;
        }

        case replyUnknownException:
        case replyUnknownLocalException:
        case replyUnknownUserException:
        {
            //
            // Don't read the exception members directly into the
            // exception. Otherwise if reading fails and raises an
            // exception, you will have a memory leak.
            //
            string unknown;
            _is.read(unknown, false);

            UnknownException* ex;
            switch(replyStatus)
            {
                case replyUnknownException:
                {
                    ex = new UnknownException(__FILE__, __LINE__);
                    break;
                }

                case replyUnknownLocalException:
                {
                    ex = new UnknownLocalException(__FILE__, __LINE__);
                    break;
                }

                case replyUnknownUserException:
                {
                    ex = new UnknownUserException(__FILE__, __LINE__);
                    break;
                }

                default:
                {
                    ex = 0; // To keep the compiler from complaining.
                    assert(false);
                    break;
                }
            }

            ex->unknown = unknown;
            _exception.reset(ex);

            _state = StateLocalException; // The state must be set last, in case there is an exception.
            break;
        }

        default:
        {
            _exception.reset(new UnknownReplyStatusException(__FILE__, __LINE__));
            _state = StateLocalException;
            break;
        }
    }

    _monitor.notify();
}

void
Outgoing::throwUserException()
{
    try
    {
        _is.startReadEncaps();
        _is.throwException();
    }
    catch(const Ice::UserException&)
    {
        _is.endReadEncaps();
        throw;
    }
}

ProxyFlushBatch::ProxyFlushBatch(IceProxy::Ice::Object* proxy, const string& operation) :
    ProxyOutgoingBase(proxy, Ice::Normal)
{
    checkSupportedProtocol(getCompatibleProtocol(proxy->__reference()->getProtocol()));
    _observer.attach(proxy, operation, 0);

    _batchRequestNum = proxy->__getBatchRequestQueue()->swap(&_os);
}

bool
ProxyFlushBatch::invokeRemote(const Ice::ConnectionIPtr& connection, bool compress, bool response)
{
    return connection->sendRequest(this, compress, response, _batchRequestNum);
}

void
ProxyFlushBatch::invokeCollocated(CollocatedRequestHandler* handler)
{
    handler->invokeRequest(this, _batchRequestNum);
}

void
ProxyFlushBatch::invoke()
{
    if(_batchRequestNum == 0)
    {
        sent();
    }
    else
    {
        invokeImpl();
    }
}

ConnectionFlushBatch::ConnectionFlushBatch(ConnectionI* connection, Instance* instance, const string& operation) :
    OutgoingBase(instance), _connection(connection)
{
    _observer.attach(instance, operation);
}

void
ConnectionFlushBatch::invoke()
{
    int batchRequestNum = _connection->getBatchRequestQueue()->swap(&_os);

    try
    {
        if(batchRequestNum == 0)
        {
            sent();
        }
        else if(!_connection->sendRequest(this, false, false, batchRequestNum))
        {
            Monitor<Mutex>::Lock sync(_monitor);
            while(!_exception.get() && !_sent)
            {
                _monitor.wait();
            }
            if(_exception.get())
            {
                _exception->ice_throw();
            }
        }
    }
    catch(const RetryException& ex)
    {
        ex.get()->ice_throw();
    }
}

void
ConnectionFlushBatch::sent()
{
    Monitor<Mutex>::Lock sync(_monitor);
    _childObserver.detach();

    _sent = true;
    _monitor.notify();

    //
    // NOTE: At this point the stack allocated ConnectionFlushBatch
    // object can be destroyed since the notify() on the monitor will
    // release the thread waiting on the synchronous Ice call.
    //
}

void
ConnectionFlushBatch::completed(const Ice::Exception& ex)
{
    Monitor<Mutex>::Lock sync(_monitor);
    _childObserver.failed(ex.ice_name());
    _childObserver.detach();
    _exception.reset(ex.ice_clone());
    _monitor.notify();
}

void
ConnectionFlushBatch::completed(BasicStream& is)
{
    assert(false);
}

void
ConnectionFlushBatch::retryException(const Ice::Exception& ex)
{
    completed(ex);
}
