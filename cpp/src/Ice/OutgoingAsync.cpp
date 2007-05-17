// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/OutgoingAsync.h>
#include <Ice/Object.h>
#include <Ice/ConnectionI.h>
#include <Ice/Reference.h>
#include <Ice/Instance.h>
#include <Ice/LocalException.h>
#include <Ice/Properties.h>
#include <Ice/LoggerUtil.h>
#include <Ice/LocatorInfo.h>
#include <Ice/ProxyFactory.h>
#include <Ice/RouterInfo.h>
#include <Ice/Outgoing.h> // For LocalExceptionWrapper.
#include <Ice/Protocol.h>
#include <Ice/ReplyStatus.h>
#include <Ice/ImplicitContextI.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(OutgoingAsync* p) { return p; }
IceUtil::Shared* IceInternal::upCast(AMI_Object_ice_invoke* p) { return p; }
IceUtil::Shared* IceInternal::upCast(AMI_Array_Object_ice_invoke* p) { return p; }

IceInternal::OutgoingAsync::OutgoingAsync() :
    __is(0),
    __os(0)
{
}

IceInternal::OutgoingAsync::~OutgoingAsync()
{
    assert(!__is);
    assert(!__os);
}

void
IceInternal::OutgoingAsync::__finished(BasicStream& is)
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(_monitor);

    Ice::Byte replyStatus;
    
    try
    {
        __is->swap(is);  
        __is->read(replyStatus);
     
        
        switch(replyStatus)
        {
            case replyOK:
            case replyUserException:
            {
                __is->startReadEncaps();
                break;
            }
            
            case replyObjectNotExist:
            case replyFacetNotExist:
            case replyOperationNotExist:
            {
                Identity ident;
                ident.__read(__is);
                
                //
                // For compatibility with the old FacetPath.
                //
                vector<string> facetPath;
                __is->read(facetPath);
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
                __is->read(operation, false);
                
                auto_ptr<RequestFailedException> ex;
                switch(replyStatus)
                {
                    case replyObjectNotExist:
                    {
                        ex.reset(new ObjectNotExistException(__FILE__, __LINE__));
                        break;
                    }
                    
                    case replyFacetNotExist:
                    {
                        ex.reset(new FacetNotExistException(__FILE__, __LINE__));
                        break;
                    }
                    
                    case replyOperationNotExist:
                    {
                        ex.reset(new OperationNotExistException(__FILE__, __LINE__));
                        break;
                    }
                    
                    default:
                    {
                        assert(false);
                        break;
                    }
                }

                ex->id = ident;
                ex->facet = facet;
                ex->operation = operation;
                ex->ice_throw();
            }
            
            case replyUnknownException:
            case replyUnknownLocalException:
            case replyUnknownUserException:
            {
                string unknown;
                __is->read(unknown, false);
                
                auto_ptr<UnknownException> ex;
                switch(replyStatus)
                {
                    case replyUnknownException:
                    {
                        ex.reset(new UnknownException(__FILE__, __LINE__));
                        break;
                    }
                    
                    case replyUnknownLocalException:
                    {
                        ex.reset(new UnknownLocalException(__FILE__, __LINE__));
                        break;
                    }
                    
                    case replyUnknownUserException:
                    {
                        ex.reset(new UnknownUserException(__FILE__, __LINE__));
                        break;
                    }
                    
                    default:
                    {
                        assert(false);
                        break;
                    }
                }
                
                ex->unknown = unknown;
                ex->ice_throw();
            }
            
            default:
            {
                throw UnknownReplyStatusException(__FILE__, __LINE__);
            }
        }
    }
    catch(const LocalException& ex)
    {
        __finished(ex);
        return;
    }

    assert(replyStatus == replyOK || replyStatus == replyUserException);

    try
    {
        __response(replyStatus == replyOK);
    }
    catch(const Exception& ex)
    {
        warning(ex);
    }
    catch(const std::exception& ex)
    {
        warning(ex);
    }
    catch(...)
    {
        warning();
    }

    cleanup();
}

void
IceInternal::OutgoingAsync::__finished(const LocalException& exc)
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(_monitor);

    if(__os) // Don't retry if cleanup() was already called.
    {
        //
        // A CloseConnectionException indicates graceful server
        // shutdown, and is therefore always repeatable without
        // violating "at-most-once". That's because by sending a close
        // connection message, the server guarantees that all
        // outstanding requests can safely be repeated. Otherwise, we
        // can also retry if the operation mode is Nonmutating or
        // Idempotent.
        //
        // An ObjectNotExistException can always be retried as
        // well without violating "at-most-once".
        //
        if(_mode == Nonmutating || _mode == Idempotent || dynamic_cast<const CloseConnectionException*>(&exc) ||
           dynamic_cast<const ObjectNotExistException*>(&exc))
        {
            try
            {
                _proxy->__handleException(_delegate, exc, _cnt);
                __send();
                return;
            }
            catch(const LocalException&)
            {
            }
        }
    }
    
    try
    {
        ice_exception(exc);
    }
    catch(const Exception& ex)
    {
        warning(ex);
    }
    catch(const std::exception& ex)
    {
        warning(ex);
    }
    catch(...)
    {
        warning();
    }

    cleanup();
}

void
IceInternal::OutgoingAsync::__prepare(const ObjectPrx& prx, const string& operation, OperationMode mode,
                                      const Context* context)
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(_monitor);

    try
    {
        //
        // We must first wait for other requests to finish.
        //
        while(__os)
        {
            _monitor.wait();
        }
        
        //
        // Can't call async via a oneway proxy.
        //
        prx->__checkTwowayOnly(operation);

        _proxy = prx;
        _delegate = 0;
        _cnt = 0;
        _mode = mode;

        ReferencePtr ref = _proxy->__reference();
        assert(!__is);
        __is = new BasicStream(ref->getInstance().get());
        assert(!__os);
        __os = new BasicStream(ref->getInstance().get());
        
        __os->writeBlob(requestHdr, sizeof(requestHdr));

        ref->getIdentity().__write(__os);

        //
        // For compatibility with the old FacetPath.
        //
        if(ref->getFacet().empty())
        {
            __os->write(static_cast<string*>(0), static_cast<string*>(0));
        }
        else
        {
            string facet = ref->getFacet();
            __os->write(&facet, &facet + 1);
        }

        __os->write(operation, false);

        __os->write(static_cast<Byte>(_mode));

        if(context != 0)
        {
            //
            // Explicit context
            //
            __writeContext(__os, *context);
        }
        else
        {
            //
            // Implicit context
            //
            const ImplicitContextIPtr& implicitContext =
                ref->getInstance()->getImplicitContext();
            
            const Context& prxContext = ref->getContext()->getValue();

            if(implicitContext == 0)
            {
                __writeContext(__os, prxContext);
            }
            else
            {
                implicitContext->write(prxContext, __os);
            }
        }
        
        __os->startWriteEncaps();
    }
    catch(const LocalException& ex)
    {
        cleanup();
        ex.ice_throw();
    }
}

void
IceInternal::OutgoingAsync::__send()
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(_monitor);
    
    try
    {
        while(true)
        {
            bool compress;
            _delegate = _proxy->__getDelegate();
            Ice::ConnectionIPtr connection = _delegate->__getConnection(compress);
            try
            {
                connection->sendAsyncRequest(__os, this, compress);
                
                //
                // Don't do anything after sendAsyncRequest() returned
                // without an exception.  I such case, there will be
                // callbacks, i.e., calls to the __finished()
                // functions. Since there is no mutex protection, we
                // cannot modify state here and in such callbacks.
                //
                return;
            }
            catch(const LocalExceptionWrapper& ex)
            {
                _proxy->__handleExceptionWrapper(_delegate, ex);
            }
            catch(const LocalException& ex)
            {
                _proxy->__handleException(_delegate, ex, _cnt);
            }
        }
    }
    catch(const LocalException& ex)
    {
        __finished(ex);
    }
}

void
IceInternal::OutgoingAsync::warning(const Exception& ex) const
{
    if(__os) // Don't print anything if cleanup() was already called.
    {
        ReferencePtr ref = _proxy->__reference();
        if(ref->getInstance()->initializationData().properties->
                getPropertyAsIntWithDefault("Ice.Warn.AMICallback", 1) > 0)
        {
            Warning out(ref->getInstance()->initializationData().logger);
            out << "Ice::Exception raised by AMI callback:\n" << ex;
        }
    }
}

void
IceInternal::OutgoingAsync::warning(const std::exception& ex) const
{
    if(__os) // Don't print anything if cleanup() was already called.
    {
        ReferencePtr ref = _proxy->__reference();
        if(ref->getInstance()->initializationData().properties->
                getPropertyAsIntWithDefault("Ice.Warn.AMICallback", 1) > 0)
        {
            Warning out(ref->getInstance()->initializationData().logger);
            out << "std::exception raised by AMI callback:\n" << ex.what();
        }
    }
}

void
IceInternal::OutgoingAsync::warning() const
{
    if(__os) // Don't print anything if cleanup() was already called.
    {
        ReferencePtr ref = _proxy->__reference();
        if(ref->getInstance()->initializationData().properties->
                getPropertyAsIntWithDefault("Ice.Warn.AMICallback", 1) > 0)
        {
            Warning out(ref->getInstance()->initializationData().logger);
            out << "unknown exception raised by AMI callback";
        }
    }
}

void
IceInternal::OutgoingAsync::cleanup()
{
    delete __is;
    __is = 0;
    delete __os;
    __os = 0;

    _monitor.notify();
}

void
Ice::AMI_Object_ice_invoke::__invoke(const ObjectPrx& prx, const string& operation, OperationMode mode,
                                     const vector<Byte>& inParams, const Context* context)
{
    try
    {
        __prepare(prx, operation, mode, context);
        __os->writeBlob(inParams);
        __os->endWriteEncaps();
    }
    catch(const LocalException& ex)
    {
        __finished(ex);
        return;
    }
    __send();
}

void
Ice::AMI_Object_ice_invoke::__response(bool ok) // ok == true means no user exception.
{
    vector<Byte> outParams;
    try
    {
        Int sz = __is->getReadEncapsSize();
        __is->readBlob(outParams, sz);
    }
    catch(const LocalException& ex)
    {
        __finished(ex);
        return;
    }
    ice_response(ok, outParams);
}

void
Ice::AMI_Array_Object_ice_invoke::__invoke(const ObjectPrx& prx, const string& operation, OperationMode mode,
                                           const pair<const Byte*, const Byte*>& inParams, const Context* context)
{
    try
    {
        __prepare(prx, operation, mode, context);
        __os->writeBlob(inParams.first, static_cast<Int>(inParams.second - inParams.first));
        __os->endWriteEncaps();
    }
    catch(const LocalException& ex)
    {
        __finished(ex);
        return;
    }
    __send();
}

void
Ice::AMI_Array_Object_ice_invoke::__response(bool ok) // ok == true means no user exception.
{
    pair<const Byte*, const Byte*> outParams;
    try
    {
        Int sz = __is->getReadEncapsSize();
        __is->readBlob(outParams.first, sz);
        outParams.second = outParams.first + sz;
    }
    catch(const LocalException& ex)
    {
        __finished(ex);
        return;
    }
    ice_response(ok, outParams);
}
