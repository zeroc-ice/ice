// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Incoming.h>
#include <Ice/IncomingAsync.h>
#include <Ice/IncomingRequest.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/ServantLocator.h>
#include <Ice/ServantManager.h>
#include <Ice/Object.h>
#include <Ice/ConnectionI.h>
#include <Ice/LocalException.h>
#include <Ice/Instance.h>
#include <Ice/Properties.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Protocol.h>
#include <Ice/ReplyStatus.h>
#include <Ice/ResponseHandler.h>
#include <Ice/StringUtil.h>
#include <typeinfo>

using namespace std;
using namespace Ice;
using namespace Ice::Instrumentation;
using namespace IceInternal;

namespace IceUtilInternal
{

extern bool printStackTraces;

}

#ifdef ICE_CPP11_MAPPING
Ice::MarshaledResult::MarshaledResult(const Ice::Current& current) :
    __os(make_shared<Ice::OutputStream>(current.adapter->getCommunicator(), Ice::currentProtocolEncoding))
{
    __os->writeBlob(replyHdr, sizeof(replyHdr));
    __os->write(current.requestId);
    __os->write(replyOK);
}
#endif

IceInternal::IncomingBase::IncomingBase(Instance* instance, ResponseHandler* responseHandler,
                                        Ice::Connection* connection, const ObjectAdapterPtr& adapter,
                                        bool response, Byte compress, Int requestId) :
    _response(response),
    _compress(compress),
    _format(Ice::DefaultFormat),
    _os(instance, Ice::currentProtocolEncoding),
    _responseHandler(responseHandler)
{
    _current.adapter = adapter;
#ifdef ICE_CPP11_MAPPING
    ::Ice::ConnectionI* conn = dynamic_cast<::Ice::ConnectionI*>(connection);
    _current.con = conn ? conn->shared_from_this() : nullptr;
#else
    _current.con = connection;
#endif
    _current.requestId = requestId;
}

IceInternal::IncomingBase::IncomingBase(IncomingBase& other) :
    _current(other._current),
    _servant(other._servant),
    _locator(other._locator),
    _cookie(other._cookie),
    _response(other._response),
    _compress(other._compress),
    _format(other._format),
    _os(other._os.instance(), Ice::currentProtocolEncoding),
    _responseHandler(other._responseHandler),
    _interceptorCBs(other._interceptorCBs)
{
    _observer.adopt(other._observer);
}

OutputStream*
IncomingBase::startWriteParams()
{
    if(!_response)
    {
        throw MarshalException(__FILE__, __LINE__, "can't marshal out parameters for oneway dispatch");
    }

    assert(_current.encoding >= Ice::Encoding_1_0); // Encoding for reply is known.

    _os.writeBlob(replyHdr, sizeof(replyHdr));
    _os.write(_current.requestId);
    _os.write(replyOK);
    _os.startEncapsulation(_current.encoding, _format);
    return &_os;
}

void
IncomingBase::endWriteParams()
{
    if(_response)
    {
        _os.endEncapsulation();
    }
}

void
IncomingBase::writeEmptyParams()
{
    if(_response)
    {
        assert(_current.encoding >= Ice::Encoding_1_0); // Encoding for reply is known.
        _os.writeBlob(replyHdr, sizeof(replyHdr));
        _os.write(_current.requestId);
        _os.write(replyOK);
        _os.writeEmptyEncapsulation(_current.encoding);
    }
}

void
IncomingBase::writeParamEncaps(const Byte* v, Ice::Int sz, bool ok)
{
    if(!ok)
    {
        _observer.userException();
    }

    if(_response)
    {
        assert(_current.encoding >= Ice::Encoding_1_0); // Encoding for reply is known.
        _os.writeBlob(replyHdr, sizeof(replyHdr));
        _os.write(_current.requestId);
        _os.write(ok ? replyOK : replyUserException);
        if(sz == 0)
        {
            _os.writeEmptyEncapsulation(_current.encoding);
        }
        else
        {
            _os.writeEncapsulation(v, sz);
        }
    }
}

#ifdef ICE_CPP11_MAPPING
void
IceInternal::IncomingBase::setMarshaledResult(const Ice::MarshaledResult& result)
{
    result.getOutputStream()->swap(_os);
}
#endif

void
IceInternal::IncomingBase::response(bool amd)
{
    try
    {
        if(_locator && !servantLocatorFinished(amd))
        {
            return;
        }

        assert(_responseHandler);
        if(_response)
        {
            _observer.reply(static_cast<Int>(_os.b.size() - headerSize - 4));
            _responseHandler->sendResponse(_current.requestId, &_os, _compress, amd);
        }
        else
        {
            _responseHandler->sendNoResponse();
        }
    }
    catch(const LocalException& ex)
    {
        _responseHandler->invokeException(_current.requestId, ex, 1, amd); // Fatal invocation exception
    }

    _observer.detach();
    _responseHandler = 0;
}

void
IceInternal::IncomingBase::exception(const std::exception& exc, bool amd)
{
    try
    {
        if(_locator && !servantLocatorFinished(amd))
        {
            return;
        }
        handleException(exc, amd);
    }
    catch(const LocalException& ex)
    {
        _responseHandler->invokeException(_current.requestId, ex, 1, amd);  // Fatal invocation exception
    }
}

void
IceInternal::IncomingBase::exception(const string& msg, bool amd)
{
    try
    {
        if(_locator && !servantLocatorFinished(amd))
        {
            return;
        }
        handleException(msg, amd);
    }
    catch(const LocalException& ex)
    {
        _responseHandler->invokeException(_current.requestId, ex, 1, amd);  // Fatal invocation exception
    }
}

void
IceInternal::IncomingBase::warning(const Exception& ex) const
{
    Warning out(_os.instance()->initializationData().logger);

    ToStringMode toStringMode = _os.instance()->toStringMode();

    out << "dispatch exception: " << ex;
    out << "\nidentity: " << identityToString(_current.id, toStringMode);
    out << "\nfacet: ";
    out << escapeString(_current.facet, "", toStringMode);
    out << "\noperation: " << _current.operation;

    if(_current.con)
    {
        try
        {
            for(Ice::ConnectionInfoPtr connInfo = _current.con->getInfo(); connInfo; connInfo = connInfo->underlying)
            {
                Ice::IPConnectionInfoPtr ipConnInfo = ICE_DYNAMIC_CAST(Ice::IPConnectionInfo, connInfo);
                if(ipConnInfo)
                {
                    out << "\nremote host: " << ipConnInfo->remoteAddress << " remote port: " << ipConnInfo->remotePort;
                    break;
                }
            }
        }
        catch(const Ice::LocalException&)
        {
            // Ignore.
        }
    }
}

void
IceInternal::IncomingBase::warning(const string& msg) const
{
    Warning out(_os.instance()->initializationData().logger);
    ToStringMode toStringMode = _os.instance()->toStringMode();

    out << "dispatch exception: " << msg;
    out << "\nidentity: " << identityToString(_current.id, toStringMode);
    out << "\nfacet: " << escapeString(_current.facet, "", toStringMode);
    out << "\noperation: " << _current.operation;

    if(_current.con)
    {
        for(Ice::ConnectionInfoPtr connInfo = _current.con->getInfo(); connInfo; connInfo = connInfo->underlying)
        {
            Ice::IPConnectionInfoPtr ipConnInfo = ICE_DYNAMIC_CAST(Ice::IPConnectionInfo, connInfo);
            if(ipConnInfo)
            {
                out << "\nremote host: " << ipConnInfo->remoteAddress << " remote port: " << ipConnInfo->remotePort;
                break;
            }
        }
    }
}

bool
IceInternal::IncomingBase::servantLocatorFinished(bool amd)
{
    assert(_locator && _servant);
    try
    {
        _locator->finished(_current, _servant, _cookie);
        return true;
    }
    catch(const std::exception& ex)
    {
        handleException(ex, amd);
    }
    catch(...)
    {
        handleException("unknown c++ exception", amd);
    }
    return false;
}

void
IceInternal::IncomingBase::handleException(const std::exception& exc, bool amd)
{
    assert(_responseHandler);

    // Reset the stream, it's possible the marshalling of the response failed and left
    // the stream in an unknown state.
    _os.clear();
    _os.b.clear();

    if(const SystemException* ex = dynamic_cast<const SystemException*>(&exc))
    {
        if(_responseHandler->systemException(_current.requestId, *ex, amd))
        {
            return;
        }
    }

    if(dynamic_cast<const RequestFailedException*>(&exc))
    {
        RequestFailedException* rfe =
            const_cast<RequestFailedException*>(dynamic_cast<const RequestFailedException*>(&exc));

        if(rfe->id.name.empty())
        {
            rfe->id = _current.id;
        }

        if(rfe->facet.empty() && !_current.facet.empty())
        {
            rfe->facet = _current.facet;
        }

        if(rfe->operation.empty() && !_current.operation.empty())
        {
            rfe->operation = _current.operation;
        }

        if(_os.instance()->initializationData().properties->getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 1)
        {
            warning(*rfe);
        }

        if(_observer)
        {
            _observer.failed(rfe->ice_id());
        }

        if(_response)
        {
            _os.writeBlob(replyHdr, sizeof(replyHdr));
            _os.write(_current.requestId);
            if(dynamic_cast<ObjectNotExistException*>(rfe))
            {
                _os.write(replyObjectNotExist);
            }
            else if(dynamic_cast<FacetNotExistException*>(rfe))
            {
                _os.write(replyFacetNotExist);
            }
            else if(dynamic_cast<OperationNotExistException*>(rfe))
            {
                _os.write(replyOperationNotExist);
            }
            else
            {
                assert(false);
            }

            _os.write(rfe->id);

            //
            // For compatibility with the old FacetPath.
            //
            if(rfe->facet.empty())
            {
                _os.write(static_cast<string*>(0), static_cast<string*>(0));
            }
            else
            {
                _os.write(&rfe->facet, &rfe->facet + 1);
            }

            _os.write(rfe->operation, false);

            _observer.reply(static_cast<Int>(_os.b.size() - headerSize - 4));
            _responseHandler->sendResponse(_current.requestId, &_os, _compress, amd);
        }
        else
        {
            _responseHandler->sendNoResponse();
        }
    }
    else if(const UserException* ex = dynamic_cast<const UserException*>(&exc))
    {
        _observer.userException();

        //
        // The operation may have already marshaled a reply; we must overwrite that reply.
        //
        if(_response)
        {
            _os.writeBlob(replyHdr, sizeof(replyHdr));
            _os.write(_current.requestId);
            _os.write(replyUserException);
            _os.startEncapsulation(_current.encoding, _format);
            _os.write(*ex);
            _os.endEncapsulation();
            _observer.reply(static_cast<Int>(_os.b.size() - headerSize - 4));
            _responseHandler->sendResponse(_current.requestId, &_os, _compress, amd);
        }
        else
        {
            _responseHandler->sendNoResponse();
        }
    }
    else if(const Exception* ex = dynamic_cast<const Exception*>(&exc))
    {
        if(_os.instance()->initializationData().properties->getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
        {
            warning(*ex);
        }

        if(_observer)
        {
            _observer.failed(ex->ice_id());
        }

        if(_response)
        {
            _os.writeBlob(replyHdr, sizeof(replyHdr));
            _os.write(_current.requestId);
            if(const UnknownLocalException* ule = dynamic_cast<const UnknownLocalException*>(&exc))
            {
                _os.write(replyUnknownLocalException);
                _os.write(ule->unknown, false);
            }
            else if(const UnknownUserException* uue = dynamic_cast<const UnknownUserException*>(&exc))
            {
                _os.write(replyUnknownUserException);
                _os.write(uue->unknown, false);
            }
            else if(const UnknownException* ue = dynamic_cast<const UnknownException*>(&exc))
            {
                _os.write(replyUnknownException);
                _os.write(ue->unknown, false);
            }
            else if(const LocalException* le = dynamic_cast<const LocalException*>(&exc))
            {
                _os.write(replyUnknownLocalException);
                ostringstream str;
                str << *le;
                if(IceUtilInternal::printStackTraces)
                {
                    str <<  '\n' << ex->ice_stackTrace();
                }
                _os.write(str.str(), false);
            }
            else if(const UserException* ue = dynamic_cast<const UserException*>(&exc))
            {
                _os.write(replyUnknownUserException);
                ostringstream str;
                str << *ue;
                if(IceUtilInternal::printStackTraces)
                {
                    str <<  '\n' << ex->ice_stackTrace();
                }
                _os.write(str.str(), false);
            }
            else
            {
                _os.write(replyUnknownException);
                ostringstream str;
                str << *ex;
                if(IceUtilInternal::printStackTraces)
                {
                    str <<  '\n' << ex->ice_stackTrace();
                }
                _os.write(str.str(), false);
            }

            _observer.reply(static_cast<Int>(_os.b.size() - headerSize - 4));
            _responseHandler->sendResponse(_current.requestId, &_os, _compress, amd);
        }
        else
        {
            _responseHandler->sendNoResponse();
        }
    }
    else
    {
        if(_os.instance()->initializationData().properties->getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
        {
            warning(string("std::exception: ") + exc.what());
        }

        if(_observer)
        {
            _observer.failed(typeid(exc).name());
        }

        if(_response)
        {
            _os.writeBlob(replyHdr, sizeof(replyHdr));
            _os.write(_current.requestId);
            _os.write(replyUnknownException);
            ostringstream str;
            str << "std::exception: " << exc.what();
            _os.write(str.str(), false);

            _observer.reply(static_cast<Int>(_os.b.size() - headerSize - 4));
            _responseHandler->sendResponse(_current.requestId, &_os, _compress, amd);
        }
        else
        {
            _responseHandler->sendNoResponse();
        }
    }

    _observer.detach();
    _responseHandler = 0;
}

void
IceInternal::IncomingBase::handleException(const string& msg, bool amd)
{
    if(_os.instance()->initializationData().properties->getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
    {
        warning(msg);
    }

    assert(_responseHandler);

    // Reset the stream, it's possible the marshalling of the response failed and left
    // the stream in an unknown state.
    _os.clear();
    _os.b.clear();

    if(_observer)
    {
        _observer.failed("unknown");
    }

    if(_response)
    {
        _os.writeBlob(replyHdr, sizeof(replyHdr));
        _os.write(_current.requestId);
        _os.write(replyUnknownException);
        string reason = msg;
        _os.write(reason, false);
        _observer.reply(static_cast<Int>(_os.b.size() - headerSize - 4));
        _responseHandler->sendResponse(_current.requestId, &_os, _compress, amd);
    }
    else
    {
        _responseHandler->sendNoResponse();
    }

    _observer.detach();
    _responseHandler = 0;
}


IceInternal::Incoming::Incoming(Instance* instance, ResponseHandler* responseHandler, Ice::Connection* connection,
                                const ObjectAdapterPtr& adapter, bool response, Byte compress, Int requestId) :
    IncomingBase(instance, responseHandler, connection, adapter, response, compress, requestId),
    _inParamPos(0)
{
}

#ifdef ICE_CPP11_MAPPING
void
IceInternal::Incoming::push(function<bool()> response, function<bool(exception_ptr)> exception)
{
    _interceptorCBs.push_front(make_pair(response, exception));
}
#else
void
IceInternal::Incoming::push(const Ice::DispatchInterceptorAsyncCallbackPtr& cb)
{
    _interceptorCBs.push_front(cb);
}
#endif

void
IceInternal::Incoming::pop()
{
    _interceptorCBs.pop_front();
}

void
IceInternal::Incoming::startOver()
{
    if(_inParamPos == 0)
    {
        //
        // That's the first startOver, so almost nothing to do
        //
        _inParamPos = _is->i;
    }
    else
    {
        // Reset input stream's position and clear response
        if(_inAsync)
        {
            _inAsync->kill(*this);
            _inAsync = 0;
        }
        _os.clear();
        _os.b.clear();

        _is->i = _inParamPos;
    }
}

void
IceInternal::Incoming::invoke(const ServantManagerPtr& servantManager, InputStream* stream)
{
    _is = stream;

    InputStream::Container::iterator start = _is->i;

    //
    // Read the current.
    //
    _is->read(_current.id);

    //
    // For compatibility with the old FacetPath.
    //
    string facet;
    {
        vector<string> facetPath;
        _is->read(facetPath);
        if(!facetPath.empty())
        {
            if(facetPath.size() > 1)
            {
                throw MarshalException(__FILE__, __LINE__);
            }
            facet.swap(facetPath[0]);
        }
    }
    _current.facet.swap(facet);

    _is->read(_current.operation, false);

    Byte b;
    _is->read(b);
    _current.mode = static_cast<OperationMode>(b);

    Int sz = _is->readSize();
    while(sz--)
    {
        pair<const string, string> pr;
        _is->read(const_cast<string&>(pr.first));
        _is->read(pr.second);
        _current.ctx.insert(_current.ctx.end(), pr);
    }

    const CommunicatorObserverPtr& obsv = _is->instance()->initializationData().observer;
    if(obsv)
    {
        // Read the parameter encapsulation size.
        Ice::Int sz;
        _is->read(sz);
        _is->i -= 4;

        _observer.attach(obsv->getDispatchObserver(_current, static_cast<Int>(_is->i - start + sz)));
    }

    //
    // Don't put the code above into the try block below. Exceptions
    // in the code above are considered fatal, and must propagate to
    // the caller of this operation.
    //

    if(servantManager)
    {
        _servant = servantManager->findServant(_current.id, _current.facet);
        if(!_servant)
        {
            _locator = servantManager->findServantLocator(_current.id.category);
            if(!_locator && !_current.id.category.empty())
            {
                _locator = servantManager->findServantLocator("");
            }

            if(_locator)
            {
                try
                {
                    _servant = _locator->locate(_current, _cookie);
                }
                catch(const std::exception& ex)
                {
                    skipReadParams(); // Required for batch requests.
                    handleException(ex, false);
                    return;
                }
                catch(...)
                {
                    skipReadParams(); // Required for batch requests.
                    handleException("unknown c++ exception", false);
                    return;
                }
            }
        }
    }

    if(!_servant)
    {
        try
        {
            if(servantManager && servantManager->hasServant(_current.id))
            {
                throw FacetNotExistException(__FILE__, __LINE__, _current.id, _current.facet, _current.operation);
            }
            else
            {
                throw ObjectNotExistException(__FILE__, __LINE__, _current.id, _current.facet, _current.operation);
            }
        }
        catch(const Ice::LocalException& ex)
        {
            skipReadParams(); // Required for batch requests
            handleException(ex, false);
            return;
        }
    }

    try
    {
        //
        // Dispatch in the incoming call
        //
        _servant->__dispatch(*this, _current);

        //
        // If the request was not dispatched asynchronously, send the response.
        //
        if(!_inAsync)
        {
            response(false);
        }
    }
    catch(const std::exception& ex)
    {
        if(_inAsync)
        {
            try
            {
                _inAsync->kill(*this);
            }
            catch(const Ice::ResponseSentException&)
            {
                const Ice::PropertiesPtr properties = _os.instance()->initializationData().properties;
                if(properties->getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
                {
                    if(const IceUtil::Exception* exc = dynamic_cast<const IceUtil::Exception*>(&ex))
                    {
                        warning(*exc);
                    }
                    else
                    {
                        warning(string("std::exception: ") + ex.what());
                    }
                }
                return;
            }
        }
        exception(ex, false);
    }
    catch(...)
    {
        if(_inAsync)
        {
            try
            {
                _inAsync->kill(*this);
            }
            catch(const Ice::ResponseSentException&)
            {
                const Ice::PropertiesPtr properties = _os.instance()->initializationData().properties;
                if(properties->getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
                {
                    warning("unknown c++ exception");
                }
                return;
            }
        }
        exception("unknown c++ exception", false);
    }
}

const Current&
IceInternal::IncomingRequest::getCurrent()
{
    return _in.getCurrent();
}
