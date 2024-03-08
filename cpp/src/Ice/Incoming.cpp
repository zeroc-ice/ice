//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Incoming.h>
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

using namespace std;
using namespace Ice;
using namespace Ice::Instrumentation;
using namespace IceInternal;

namespace IceUtilInternal
{
    extern bool printStackTraces;
}

Incoming::Incoming(
    Instance* instance,
    ResponseHandlerPtr responseHandler,
    Ice::ConnectionPtr connection,
    ObjectAdapterPtr adapter,
    bool twoWay,
    uint8_t compress,
    int32_t requestId)
    : _isTwoWay(twoWay),
      _compress(compress),
      _format(Ice::FormatType::DefaultFormat),
      _os(instance, Ice::currentProtocolEncoding),
      _responseHandler(std::move(responseHandler)),
      _is(nullptr)
{
    _current.adapter = std::move(adapter);
    _current.con = std::move(connection);
    _current.requestId = requestId;
    _current.encoding.major = 0;
    _current.encoding.minor = 0;
}

Incoming::Incoming(Incoming&& other)
    : _current(std::move(other._current)),
      _servant(std::move(other._servant)),
      _locator(std::move(other._locator)),
      _cookie(std::move(other._cookie)),
      _isTwoWay(std::move(other._isTwoWay)),
      _compress(std::move(other._compress)),
      _format(std::move(other._format)),
      _os(other._os.instance(), Ice::currentProtocolEncoding), // TODO: make movable
      _responseHandler(std::move(other._responseHandler)),
      _is(std::move(other._is)) // just copies the pointer
{
    _observer.adopt(other._observer); // TODO: make movable
}

OutputStream*
Incoming::startWriteParams()
{
    if (!_isTwoWay)
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
Incoming::endWriteParams()
{
    if (_isTwoWay)
    {
        _os.endEncapsulation();
    }
}

void
Incoming::writeEmptyParams()
{
    if (_isTwoWay)
    {
        assert(_current.encoding >= Ice::Encoding_1_0); // Encoding for reply is known.
        _os.writeBlob(replyHdr, sizeof(replyHdr));
        _os.write(_current.requestId);
        _os.write(replyOK);
        _os.writeEmptyEncapsulation(_current.encoding);
    }
}

void
Incoming::writeParamEncaps(const uint8_t* v, int32_t sz, bool ok)
{
    if (!ok)
    {
        _observer.userException();
    }

    if (_isTwoWay)
    {
        assert(_current.encoding >= Ice::Encoding_1_0); // Encoding for reply is known.
        _os.writeBlob(replyHdr, sizeof(replyHdr));
        _os.write(_current.requestId);
        _os.write(ok ? replyOK : replyUserException);
        if (sz == 0)
        {
            _os.writeEmptyEncapsulation(_current.encoding);
        }
        else
        {
            _os.writeEncapsulation(v, sz);
        }
    }
}

void
Incoming::setMarshaledResult(Ice::MarshaledResult&& result)
{
    result.swap(_os);
}

void
Incoming::response(bool amd)
{
    try
    {
        if (_locator && !servantLocatorFinished(amd))
        {
            return;
        }

        assert(_responseHandler);
        if (_isTwoWay)
        {
            _observer.reply(static_cast<int32_t>(_os.b.size() - headerSize - 4));
            _responseHandler->sendResponse(_current.requestId, &_os, _compress, amd);
        }
        else
        {
            _responseHandler->sendNoResponse();
        }
    }
    catch (const LocalException&)
    {
        _responseHandler->invokeException(
            _current.requestId, current_exception(), 1, amd); // Fatal invocation exception
    }

    _observer.detach();
    _responseHandler = nullptr;
}

void
Incoming::exception(std::exception_ptr exc, bool amd)
{
    try
    {
        if (_locator && !servantLocatorFinished(amd))
        {
            return;
        }
        handleException(exc, amd);
    }
    catch (const LocalException&)
    {
        _responseHandler->invokeException(
            _current.requestId, current_exception(), 1, amd); // Fatal invocation exception
    }
}

void
Incoming::response()
{
    writeEmptyParams();
    completed();
}

void
Incoming::response(MarshaledResult&& result)
{
    setMarshaledResult(std::move(result));
    completed();
}

void
Incoming::warning(const Exception& ex) const
{
    Warning out(_os.instance()->initializationData().logger);

    ToStringMode toStringMode = _os.instance()->toStringMode();

    out << "dispatch exception: " << ex;
    out << "\nidentity: " << identityToString(_current.id, toStringMode);
    out << "\nfacet: ";
    out << escapeString(_current.facet, "", toStringMode);
    out << "\noperation: " << _current.operation;

    if (_current.con)
    {
        try
        {
            for (Ice::ConnectionInfoPtr connInfo = _current.con->getInfo(); connInfo; connInfo = connInfo->underlying)
            {
                Ice::IPConnectionInfoPtr ipConnInfo = dynamic_pointer_cast<Ice::IPConnectionInfo>(connInfo);
                if (ipConnInfo)
                {
                    out << "\nremote host: " << ipConnInfo->remoteAddress << " remote port: " << ipConnInfo->remotePort;
                    break;
                }
            }
        }
        catch (const Ice::LocalException&)
        {
            // Ignore.
        }
    }
}

void
Incoming::warning(std::exception_ptr ex) const
{
    Warning out(_os.instance()->initializationData().logger);
    ToStringMode toStringMode = _os.instance()->toStringMode();

    out << "dispatch exception: ";

    try
    {
        rethrow_exception(ex);
    }
    catch (const std::exception& e)
    {
        out << "std::exception: " << e.what();
    }
    catch (...)
    {
        out << "unknown c++ exception";
    }
    out << "\nidentity: " << identityToString(_current.id, toStringMode);
    out << "\nfacet: " << escapeString(_current.facet, "", toStringMode);
    out << "\noperation: " << _current.operation;

    if (_current.con)
    {
        for (Ice::ConnectionInfoPtr connInfo = _current.con->getInfo(); connInfo; connInfo = connInfo->underlying)
        {
            Ice::IPConnectionInfoPtr ipConnInfo = dynamic_pointer_cast<Ice::IPConnectionInfo>(connInfo);
            if (ipConnInfo)
            {
                out << "\nremote host: " << ipConnInfo->remoteAddress << " remote port: " << ipConnInfo->remotePort;
                break;
            }
        }
    }
}

bool
Incoming::servantLocatorFinished(bool amd)
{
    assert(_locator && _servant);
    try
    {
        _locator->finished(_current, _servant, _cookie);
        return true;
    }
    catch (...)
    {
        handleException(current_exception(), amd);
    }
    return false;
}

void
Incoming::handleException(std::exception_ptr exc, bool amd)
{
    assert(_responseHandler);

    // Reset the stream, it's possible the marshalling of the response failed and left
    // the stream in an unknown state.
    _os.clear();
    _os.b.clear();

    try
    {
        rethrow_exception(exc);
    }
    catch (RequestFailedException& rfe)
    {
        if (rfe.id.name.empty())
        {
            rfe.id = _current.id;
        }

        if (rfe.facet.empty() && !_current.facet.empty())
        {
            rfe.facet = _current.facet;
        }

        if (rfe.operation.empty() && !_current.operation.empty())
        {
            rfe.operation = _current.operation;
        }

        if (_os.instance()->initializationData().properties->getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 1)
        {
            warning(rfe);
        }

        if (_observer)
        {
            _observer.failed(rfe.ice_id());
        }

        if (_isTwoWay)
        {
            _os.writeBlob(replyHdr, sizeof(replyHdr));
            _os.write(_current.requestId);
            if (dynamic_cast<ObjectNotExistException*>(&rfe))
            {
                _os.write(replyObjectNotExist);
            }
            else if (dynamic_cast<FacetNotExistException*>(&rfe))
            {
                _os.write(replyFacetNotExist);
            }
            else if (dynamic_cast<OperationNotExistException*>(&rfe))
            {
                _os.write(replyOperationNotExist);
            }
            else
            {
                assert(false);
            }

            _os.write(rfe.id);

            //
            // For compatibility with the old FacetPath.
            //
            if (rfe.facet.empty())
            {
                _os.write(static_cast<string*>(0), static_cast<string*>(0));
            }
            else
            {
                _os.write(&rfe.facet, &rfe.facet + 1);
            }

            _os.write(rfe.operation, false);

            _observer.reply(static_cast<int32_t>(_os.b.size() - headerSize - 4));
            _responseHandler->sendResponse(_current.requestId, &_os, _compress, amd);
        }
        else
        {
            _responseHandler->sendNoResponse();
        }
    }
    catch (const UserException& uex)
    {
        _observer.userException();

        //
        // The operation may have already marshaled a reply; we must overwrite that reply.
        //
        if (_isTwoWay)
        {
            _os.writeBlob(replyHdr, sizeof(replyHdr));
            _os.write(_current.requestId);
            _os.write(replyUserException);
            _os.startEncapsulation(_current.encoding, _format);
            _os.write(uex);
            _os.endEncapsulation();
            _observer.reply(static_cast<int32_t>(_os.b.size() - headerSize - 4));
            _responseHandler->sendResponse(_current.requestId, &_os, _compress, amd);
        }
        else
        {
            _responseHandler->sendNoResponse();
        }
    }
    catch (const Exception& ex)
    {
        if (_os.instance()->initializationData().properties->getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
        {
            warning(ex);
        }

        if (_observer)
        {
            _observer.failed(ex.ice_id());
        }

        if (_isTwoWay)
        {
            _os.writeBlob(replyHdr, sizeof(replyHdr));
            _os.write(_current.requestId);
            if (const UnknownLocalException* ule = dynamic_cast<const UnknownLocalException*>(&ex))
            {
                _os.write(replyUnknownLocalException);
                _os.write(ule->unknown, false);
            }
            else if (const UnknownUserException* uue = dynamic_cast<const UnknownUserException*>(&ex))
            {
                _os.write(replyUnknownUserException);
                _os.write(uue->unknown, false);
            }
            else if (const UnknownException* ue = dynamic_cast<const UnknownException*>(&ex))
            {
                _os.write(replyUnknownException);
                _os.write(ue->unknown, false);
            }
            else if (const LocalException* le = dynamic_cast<const LocalException*>(&ex))
            {
                _os.write(replyUnknownLocalException);
                ostringstream str;
                str << *le;
                if (IceUtilInternal::printStackTraces)
                {
                    str << '\n' << ex.ice_stackTrace();
                }
                _os.write(str.str(), false);
            }
            else if (const UserException* use = dynamic_cast<const UserException*>(&ex))
            {
                _os.write(replyUnknownUserException);
                ostringstream str;
                str << *use;
                if (IceUtilInternal::printStackTraces)
                {
                    str << '\n' << ex.ice_stackTrace();
                }
                _os.write(str.str(), false);
            }
            else
            {
                _os.write(replyUnknownException);
                ostringstream str;
                str << ex;
                if (IceUtilInternal::printStackTraces)
                {
                    str << '\n' << ex.ice_stackTrace();
                }
                _os.write(str.str(), false);
            }

            _observer.reply(static_cast<int32_t>(_os.b.size() - headerSize - 4));
            _responseHandler->sendResponse(_current.requestId, &_os, _compress, amd);
        }
        else
        {
            _responseHandler->sendNoResponse();
        }
    }
    catch (...)
    {
        string exceptionId = getExceptionId(exc);

        if (_os.instance()->initializationData().properties->getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
        {
            warning(exc);
        }

        if (_observer)
        {
            _observer.failed(exceptionId);
        }

        if (_isTwoWay)
        {
            _os.writeBlob(replyHdr, sizeof(replyHdr));
            _os.write(_current.requestId);
            _os.write(replyUnknownException);
            ostringstream str;
            str << "c++ exception: " << exceptionId;
            _os.write(str.str(), false);

            _observer.reply(static_cast<int32_t>(_os.b.size() - headerSize - 4));
            _responseHandler->sendResponse(_current.requestId, &_os, _compress, amd);
        }
        else
        {
            _responseHandler->sendNoResponse();
        }
    }

    _observer.detach();
    _responseHandler = nullptr;
}

void
Incoming::invoke(const ServantManagerPtr& servantManager, InputStream* stream)
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
        if (!facetPath.empty())
        {
            if (facetPath.size() > 1)
            {
                throw MarshalException(__FILE__, __LINE__);
            }
            facet.swap(facetPath[0]);
        }
    }
    _current.facet.swap(facet);

    _is->read(_current.operation, false);

    uint8_t b;
    _is->read(b);
    _current.mode = static_cast<OperationMode>(b);

    int32_t sz = _is->readSize();
    while (sz--)
    {
        pair<const string, string> pr;
        _is->read(const_cast<string&>(pr.first));
        _is->read(pr.second);
        _current.ctx.insert(_current.ctx.end(), pr);
    }

    const CommunicatorObserverPtr& obsv = _is->instance()->initializationData().observer;
    if (obsv)
    {
        // Read the parameter encapsulation size.
        int32_t encapsSize;
        _is->read(encapsSize);
        _is->i -= 4;

        _observer.attach(obsv->getDispatchObserver(_current, static_cast<int32_t>(_is->i - start + encapsSize)));
    }

    //
    // Don't put the code above into the try block below. Exceptions
    // in the code above are considered fatal, and must propagate to
    // the caller of this operation.
    //

    if (servantManager)
    {
        _servant = servantManager->findServant(_current.id, _current.facet);
        if (!_servant)
        {
            _locator = servantManager->findServantLocator(_current.id.category);
            if (!_locator && !_current.id.category.empty())
            {
                _locator = servantManager->findServantLocator("");
            }

            if (_locator)
            {
                try
                {
                    _servant = _locator->locate(_current, _cookie);
                }
                catch (...)
                {
                    skipReadParams(); // Required for batch requests.
                    handleException(current_exception(), false);
                    return;
                }
            }
        }
    }

    if (!_servant)
    {
        try
        {
            if (servantManager && servantManager->hasServant(_current.id))
            {
                throw FacetNotExistException(__FILE__, __LINE__, _current.id, _current.facet, _current.operation);
            }
            else
            {
                throw ObjectNotExistException(__FILE__, __LINE__, _current.id, _current.facet, _current.operation);
            }
        }
        catch (...)
        {
            skipReadParams(); // Required for batch requests
            handleException(current_exception(), false);
            return;
        }
    }

    try
    {
        // Dispatch the request to the servant.
        if (_servant->_iceDispatch(*this))
        {
            // If the request was dispatched synchronously, send the response.
            response(false); // amd: false
        }
    }
    catch (...)
    {
        // An async dispatch is not allowed to throw any exception because it moves "this" memory into a new Incoming
        // object.
        exception(current_exception(), false); // amd: false
    }
}

void
Incoming::completed()
{
    setResponseSent();
    response(true); // amd: true
}

void
Incoming::completed(exception_ptr ex)
{
    setResponseSent();
    exception(ex, true); // amd: true
}

void
Incoming::failed(exception_ptr ex) noexcept
{
    try
    {
        completed(ex);
    }
    catch (...)
    {
        // Ignore all exceptions. The caller in the dispatch thread can't handle any exception because its memory
        // was moved to a new "async" incoming object.
    }
}

void
Incoming::setResponseSent()
{
    if (_responseSent)
    {
        // The application must not call the callbacks twice, or call a callback after throwing an exception from the
        // dispatch thread.
        throw ResponseSentException(__FILE__, __LINE__);
    }
    else
    {
        _responseSent = true;
    }
}
