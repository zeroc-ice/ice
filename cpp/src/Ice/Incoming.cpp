// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
#include <IceUtil/StringUtil.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::IncomingBase::IncomingBase(Instance* instance, ConnectionI* connection, 
                                        const ObjectAdapterPtr& adapter,
                                        bool response, Byte compress, Int requestId) :
    _response(response),
    _compress(compress),
    _os(instance),
    _connection(connection)
{
    _current.adapter = adapter;
    _current.con = _connection;
    _current.requestId = requestId;
}

IceInternal::IncomingBase::IncomingBase(IncomingBase& in) :
    _current(in._current), // copy
    _os(in._os.instance()),
    _interceptorAsyncCallbackQueue(in._interceptorAsyncCallbackQueue) // copy
{
    adopt(in); // adopt everything else
}

void
IceInternal::IncomingBase::adopt(IncomingBase& other)
{
    _servant = other._servant;
    other._servant = 0;
    
    _locator = other._locator;
    other._locator = 0;
    
    _cookie = other._cookie;
    other._cookie = 0;
    
    _response = other._response;
    other._response = false;
    
    _compress = other._compress;
    other._compress = 0;
    
    _os.swap(other._os);
    
    _connection = other._connection;
    other._connection = 0;
}

void
IceInternal::IncomingBase::__warning(const Exception& ex) const
{
    ostringstream str;
    str << ex;
    __warning(str.str());
}

void
IceInternal::IncomingBase::__warning(const string& msg) const
{
    Warning out(_os.instance()->initializationData().logger);
    
    out << "dispatch exception: " << msg;
    out << "\nidentity: " << _os.instance()->identityToString(_current.id);
    out << "\nfacet: " << IceUtil::escapeString(_current.facet, "");
    out << "\noperation: " << _current.operation;
}

void
IceInternal::IncomingBase::__handleException(const Ice::Exception& ex)
{
    try
    {
        ex.ice_throw();
    }
    catch(RequestFailedException& ex)
    {
        if(ex.id.name.empty())
        {
            ex.id = _current.id;
        }
        
        if(ex.facet.empty() && !_current.facet.empty())
        {
            ex.facet = _current.facet;
        }
        
        if(ex.operation.empty() && !_current.operation.empty())
        {
            ex.operation = _current.operation;
        }

        if(_os.instance()->initializationData().properties->getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 1)
        {
            __warning(ex);
        }

        if(_response)
        {
            _os.endWriteEncaps();
            _os.b.resize(headerSize + 4); // Reply status position.
            if(dynamic_cast<ObjectNotExistException*>(&ex))
            {
                _os.write(replyObjectNotExist);
            }
            else if(dynamic_cast<FacetNotExistException*>(&ex))
            {
                _os.write(replyFacetNotExist);
            }
            else if(dynamic_cast<OperationNotExistException*>(&ex))
            {
                _os.write(replyOperationNotExist);
            }
            else
            {
                assert(false);
            }

            ex.id.__write(&_os);

            //
            // For compatibility with the old FacetPath.
            //
            if(ex.facet.empty())
            {
                _os.write(static_cast<string*>(0), static_cast<string*>(0));
            }
            else
            {
                _os.write(&ex.facet, &ex.facet + 1);
            }

            _os.write(ex.operation, false);
            
            _connection->sendResponse(&_os, _compress);
        }
        else
        {
            _connection->sendNoResponse();
        }
    }
    catch(const UnknownLocalException& ex)
    {
        if(_os.instance()->initializationData().properties->getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
        {
            __warning(ex);
        }

        if(_response)
        {
            _os.endWriteEncaps();
            _os.b.resize(headerSize + 4); // Reply status position.
            _os.write(replyUnknownLocalException);
            _os.write(ex.unknown, false);
            _connection->sendResponse(&_os, _compress);
        }
        else
        {
            _connection->sendNoResponse();
        }
    }
    catch(const UnknownUserException& ex)
    {
        if(_os.instance()->initializationData().properties->getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
        {
            __warning(ex);
        }

        if(_response)
        {
            _os.endWriteEncaps();
            _os.b.resize(headerSize + 4); // Reply status position.
            _os.write(replyUnknownUserException);
            _os.write(ex.unknown, false);
            _connection->sendResponse(&_os, _compress);
        }
        else
        {
            _connection->sendNoResponse();
        }
    }
    catch(const UnknownException& ex)
    {
        if(_os.instance()->initializationData().properties->getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
        {
            __warning(ex);
        }

        if(_response)
        {
            _os.endWriteEncaps();
            _os.b.resize(headerSize + 4); // Reply status position.
            _os.write(replyUnknownException);
            _os.write(ex.unknown, false);
            _connection->sendResponse(&_os, _compress);
        }
        else
        {
            _connection->sendNoResponse();
        }
    }
    catch(const LocalException& ex)
    {
        if(_os.instance()->initializationData().properties->getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
        {
            __warning(ex);
        }

        if(_response)
        {
            _os.endWriteEncaps();
            _os.b.resize(headerSize + 4); // Reply status position.
            _os.write(replyUnknownLocalException);
            ostringstream str;
            str << ex;
            _os.write(str.str(), false);
            _connection->sendResponse(&_os, _compress);
        }
        else
        {
            _connection->sendNoResponse();
        }
    }
    catch(const UserException& ex)
    {
        if(_os.instance()->initializationData().properties->getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
        {
            __warning(ex);
        }

        if(_response)
        {
            _os.endWriteEncaps();
            _os.b.resize(headerSize + 4); // Reply status position.
            _os.write(replyUnknownUserException);
            ostringstream str;
            str << ex;
            _os.write(str.str(), false);
            _connection->sendResponse(&_os, _compress);
        }
        else
        {
            _connection->sendNoResponse();
        }
    }
    catch(const Exception& ex)
    {
        if(_os.instance()->initializationData().properties->getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
        {
            __warning(ex);
        }

        if(_response)
        {
            _os.endWriteEncaps();
            _os.b.resize(headerSize + 4); // Reply status position.
            _os.write(replyUnknownException);
            ostringstream str;
            str << ex;
            _os.write(str.str(), false);
            _connection->sendResponse(&_os, _compress);
        }
        else
        {
            _connection->sendNoResponse();
        }
    }
}

void
IceInternal::IncomingBase::__handleException(const std::exception& ex)
{
    if(_os.instance()->initializationData().properties->getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
    {
        __warning(string("std::exception: ") + ex.what());
    }
    
    if(_response)
    {
        _os.endWriteEncaps();
        _os.b.resize(headerSize + 4); // Reply status position.
        _os.write(replyUnknownException);
        ostringstream str;
        str << "std::exception: " << ex.what();
        _os.write(str.str(), false);
        _connection->sendResponse(&_os, _compress);
    }
    else
    {
        _connection->sendNoResponse();
    }    
}

void
IceInternal::IncomingBase::__handleException()
{
    if(_os.instance()->initializationData().properties->getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
    {
        __warning("unknown c++ exception");
    }
    
    if(_response)
    {
        _os.endWriteEncaps();
        _os.b.resize(headerSize + 4); // Reply status position.
        _os.write(replyUnknownException);
        string reason = "unknown c++ exception";
        _os.write(reason, false);
        _connection->sendResponse(&_os, _compress);
    }
    else
    {
        _connection->sendNoResponse();
    }
}


IceInternal::Incoming::Incoming(Instance* instance, ConnectionI* connection, 
                                const ObjectAdapterPtr& adapter,
                                bool response, Byte compress, Int requestId) :
    IncomingBase(instance, connection, adapter, response, compress, requestId),
    _is(instance),
    _inParamPos(0)
{
}


void 
IceInternal::Incoming::push(const Ice::DispatchInterceptorAsyncCallbackPtr& cb)
{
    _interceptorAsyncCallbackQueue.push_front(cb);
}

void 
IceInternal::Incoming::pop()
{
    _interceptorAsyncCallbackQueue.pop_front();
}

void 
IceInternal::Incoming::startOver()
{
    if(_inParamPos == 0)
    {
        //
        // That's the first startOver, so almost nothing to do
        //
        _inParamPos = _is.i - 6; // 6 bytes for the start of the encaps
    }
    else
    {
        killAsync();
        
        //
        // Let's rewind _is and clean-up _os
        //
        _is.endReadEncaps();
        _is.i = _inParamPos;
        _is.startReadEncaps();
        
        if(_response)
        {
            _os.endWriteEncaps();
            _os.b.resize(headerSize + 4); 
            _os.write(static_cast<Byte>(0));
            _os.startWriteEncaps();
        }
    }
}

void 
IceInternal::Incoming::killAsync()
{
    //
    // Always runs in the dispatch thread
    //
    if(_cb != 0)
    {
        //
        // May raise ResponseSentException
        //
        _cb->__deactivate(*this);
        _cb = 0;
    }
}

void
IceInternal::Incoming::setActive(IncomingAsync& cb)
{
    assert(_cb == 0);
    _cb = &cb; // acquires a ref-count
}

void
IceInternal::Incoming::invoke(const ServantManagerPtr& servantManager)
{
    //
    // Read the current.
    //
    _current.id.__read(&_is);

    //
    // For compatibility with the old FacetPath.
    //
    string facet;
    {
        vector<string> facetPath;
        _is.read(facetPath);
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

    _is.read(_current.operation, false);

    Byte b;
    _is.read(b);
    _current.mode = static_cast<OperationMode>(b);

    Int sz;
    _is.readSize(sz);
    while(sz--)
    {
        pair<const string, string> pr;
        _is.read(const_cast<string&>(pr.first));
        _is.read(pr.second);
        _current.ctx.insert(_current.ctx.end(), pr);
    }

    _is.startReadEncaps();

    if(_response)
    {
        assert(_os.b.size() == headerSize + 4); // Reply status position.
        _os.write(static_cast<Byte>(0));
        _os.startWriteEncaps();
    }

    // Initialize status to some value, to keep the compiler happy.
    Ice::Byte replyStatus = replyOK;

    DispatchStatus dispatchStatus = DispatchOK;

    //
    // Don't put the code above into the try block below. Exceptions
    // in the code above are considered fatal, and must propagate to
    // the caller of this operation.
    //

    try
    {
        try
        {
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
                        _servant = _locator->locate(_current, _cookie);
                    }
                }
            }
            if(!_servant)
            {
                if(servantManager && servantManager->hasServant(_current.id))
                {
                    replyStatus = replyFacetNotExist;
                }
                else
                {
                    replyStatus = replyObjectNotExist;
                }
            }
            else
            {
                dispatchStatus = _servant->__dispatch(*this, _current);
                if(dispatchStatus == DispatchUserException)
                {
                    replyStatus = replyUserException;
                }
            }
        }
        catch(...)
        {
            if(_locator && _servant && dispatchStatus != DispatchAsync)
            {
                _locator->finished(_current, _servant, _cookie);
            }

            throw;
        }
        
        if(_locator && _servant && dispatchStatus != DispatchAsync)
        {
            _locator->finished(_current, _servant, _cookie);
        }
    }
    catch(const Exception& ex)
    {
        _is.endReadEncaps();
        __handleException(ex);
        return;
    }
    catch(const std::exception& ex)
    {
        _is.endReadEncaps();
        __handleException(ex);
        return;
    }
    catch(...)
    {
        _is.endReadEncaps();
        __handleException();
        return;
    }

    //
    // Don't put the code below into the try block above. Exceptions
    // in the code below are considered fatal, and must propagate to
    // the caller of this operation.
    //
    
    _is.endReadEncaps();

    //
    // DispatchAsync is "pseudo dispatch status", used internally only
    // to indicate async dispatch.
    //
    if(dispatchStatus == DispatchAsync)
    {
        //
        // If this was an asynchronous dispatch, we're done here.
        //
        return;
    }

    if(_response)
    {
        _os.endWriteEncaps();
        
        if(replyStatus != replyOK && replyStatus != replyUserException)
        {
            assert(replyStatus == replyObjectNotExist ||
                   replyStatus == replyFacetNotExist);
            
            _os.b.resize(headerSize + 4); // Reply status position.
            _os.write(replyStatus);
            
            _current.id.__write(&_os);

            //
            // For compatibility with the old FacetPath.
            //
            if(_current.facet.empty())
            {
                _os.write(static_cast<string*>(0), static_cast<string*>(0));
            }
            else
            {
                _os.write(&_current.facet, &_current.facet + 1);
            }

            _os.write(_current.operation, false);
        }
        else
        {
            *(_os.b.begin() + headerSize + 4) = replyStatus; // Reply status position.
        }

        _connection->sendResponse(&_os, _compress);
    }
    else
    {
        _connection->sendNoResponse();
    }
}


bool
IceInternal::IncomingRequest::isCollocated()
{
    return false;
}

const Current&
IceInternal::IncomingRequest::getCurrent()
{
    return _in.getCurrent();
}
