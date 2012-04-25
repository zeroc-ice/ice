// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Glacier2/Blobject.h>
#include <Glacier2/SessionRouterI.h>

using namespace std;
using namespace Ice;
using namespace Glacier2;

namespace
{

const string serverForwardContext = "Glacier2.Server.ForwardContext";
const string clientForwardContext = "Glacier2.Client.ForwardContext";
const string serverAlwaysBatch = "Glacier2.Server.AlwaysBatch";
const string clientAlwaysBatch = "Glacier2.Client.AlwaysBatch";
const string serverTraceRequest = "Glacier2.Server.Trace.Request";
const string clientTraceRequest = "Glacier2.Client.Trace.Request";
const string serverTraceOverride = "Glacier2.Server.Trace.Override";
const string clientTraceOverride = "Glacier2.Client.Trace.Override";

class AMI_Array_Object_ice_invokeTwowayI : public AMI_Array_Object_ice_invoke
{
public:

    AMI_Array_Object_ice_invokeTwowayI(const AMD_Object_ice_invokePtr& amdCB,
                                       const InstancePtr& instance,
                                       const ConnectionPtr& connection) :
        _amdCB(amdCB),
        _instance(instance),
        _connection(connection)
    {
    }

    virtual void
    ice_response(bool ok, const pair<const Byte*, const Byte*>& outParams)
    {
        _amdCB->ice_response(ok, outParams);
    }

    virtual void
    ice_exception(const Exception& ex)
    {
        //
        // If the connection has been lost, destroy the session.
        //
        if(_connection)
        {
            if(dynamic_cast<const Ice::SocketException*>(&ex) ||
               dynamic_cast<const Ice::TimeoutException*>(&ex) ||
               dynamic_cast<const Ice::ProtocolException*>(&ex))
            {
                try
                {
                    _instance->sessionRouter()->destroySession(_connection);
                }
                catch(const Exception&)
                {
                }
            }
        }

        _amdCB->ice_exception(ex);
    }

private:

    const AMD_Object_ice_invokePtr _amdCB;
    const InstancePtr _instance;
    const ConnectionPtr _connection;
};

class AMI_Array_Object_ice_invokeOnewayI : public AMI_Array_Object_ice_invoke, public Ice::AMISentCallback
{
public:

    AMI_Array_Object_ice_invokeOnewayI(const AMD_Object_ice_invokePtr& amdCB,
                                       const InstancePtr& instance,
                                       const ConnectionPtr& connection) :
        _amdCB(amdCB),
        _instance(instance),
        _connection(connection)
    {
    }

    virtual void
    ice_response(bool, const pair<const Byte*, const Byte*>&)
    {
        assert(false);
    }

    virtual void
    ice_sent()
    {
#if (defined(_MSC_VER) && (_MSC_VER >= 1600))
        _amdCB->ice_response(true, pair<const Byte*, const Byte*>(nullptr, nullptr));
#else
        _amdCB->ice_response(true, pair<const Byte*, const Byte*>(0, 0));
#endif
    }

    virtual void
    ice_exception(const Exception& ex)
    {
        //
        // If the connection has been lost, destroy the session.
        //
        if(_connection)
        {
            if(dynamic_cast<const Ice::SocketException*>(&ex) ||
               dynamic_cast<const Ice::TimeoutException*>(&ex) ||
               dynamic_cast<const Ice::ProtocolException*>(&ex))
            {
                try
                {
                    _instance->sessionRouter()->destroySession(_connection);
                }
                catch(const Exception&)
                {
                }
            }
        }

        _amdCB->ice_exception(ex);
    }

private:

    const AMD_Object_ice_invokePtr _amdCB;
    const InstancePtr _instance;
    const ConnectionPtr _connection;
};

}

Glacier2::Blobject::Blobject(const InstancePtr& instance, const ConnectionPtr& reverseConnection,
                             const Ice::Context& context) :
    _instance(instance),
    _reverseConnection(reverseConnection),
    _forwardContext(_reverseConnection ?
                    _instance->properties()->getPropertyAsInt(serverForwardContext) > 0 :
                    _instance->properties()->getPropertyAsInt(clientForwardContext) > 0),
    _alwaysBatch(_reverseConnection ?
                 _instance->properties()->getPropertyAsInt(serverAlwaysBatch) > 0 :
                 _instance->properties()->getPropertyAsInt(clientAlwaysBatch) > 0),
    _requestTraceLevel(_reverseConnection ?
                       _instance->properties()->getPropertyAsInt(serverTraceRequest) :
                       _instance->properties()->getPropertyAsInt(clientTraceRequest)),
    _overrideTraceLevel(reverseConnection ?
                        _instance->properties()->getPropertyAsInt(serverTraceOverride) :
                        _instance->properties()->getPropertyAsInt(clientTraceOverride)),
    _context(context)
{
    RequestQueueThreadPtr t = _reverseConnection ? _instance->serverRequestQueueThread() : 
                                                   _instance->clientRequestQueueThread();
    if(t)
    {
        const_cast<RequestQueuePtr&>(_requestQueue) = new RequestQueue(t, _instance, _reverseConnection);
    }
}

Glacier2::Blobject::~Blobject()
{
}

void
Glacier2::Blobject::destroy()
{
    if(_requestQueue)
    {
        _requestQueue->destroy();
    }
}

void
Glacier2::Blobject::invoke(ObjectPrx& proxy, const AMD_Object_ice_invokePtr& amdCB, 
                           const std::pair<const Ice::Byte*, const Ice::Byte*>& inParams, const Current& current)
{
    //
    // Set the correct facet on the proxy.
    //
    if(!current.facet.empty())
    {
        proxy = proxy->ice_facet(current.facet);
    }

    //
    // Modify the proxy according to the request id. This can
    // be overridden by the _fwd context.
    //
    if(current.requestId == 0)
    {
        if(_alwaysBatch && _requestQueue)
        {
            proxy = proxy->ice_batchOneway();
        }
        else
        {
            proxy = proxy->ice_oneway();
        }
    }
    else if(current.requestId > 0)
    {
        proxy = proxy->ice_twoway();
    }

    //
    // Modify the proxy according to the _fwd context field.
    //
    Context::const_iterator p = current.ctx.find("_fwd");
    if(p != current.ctx.end())
    {
        for(unsigned int i = 0; i < p->second.length(); ++i)
        {
            char option = p->second[i];
            switch(option)
            {
                case 't':
                {
                    proxy = proxy->ice_twoway();
                    break;
                }
                
                case 'o':
                {
                    if(_alwaysBatch && _requestQueue)
                    {
                        proxy = proxy->ice_batchOneway();
                    }
                    else
                    {
                        proxy = proxy->ice_oneway();
                    }
                    break;
                }
                
                case 'd':
                {
                    if(_alwaysBatch && _requestQueue)
                    {
                        proxy = proxy->ice_batchDatagram();
                    }
                    else
                    {
                        proxy = proxy->ice_datagram();
                    }
                    break;
                }
                
                case 'O':
                {
                    if(_requestQueue)
                    {
                        proxy = proxy->ice_batchOneway();
                    }
                    else
                    {
                        proxy = proxy->ice_oneway();
                    }
                    break;
                }
                
                case 'D':
                {
                    if(_requestQueue)
                    {
                        proxy = proxy->ice_batchDatagram();
                    }
                    else
                    {
                        proxy = proxy->ice_datagram();
                    }
                    break;
                }
                
                case 's':
                {
                    proxy = proxy->ice_secure(true);
                    break;
                }
                
                case 'z':
                {
                    proxy = proxy->ice_compress(true);
                    break;
                }
                
                default:
                {
                    Warning out(_instance->logger());
                    out << "unknown forward option `" << option << "'";
                    break;
                }
            }
        }
    }
    
    if(_requestTraceLevel >= 1)
    {
        Trace out(_instance->logger(), "Glacier2");
        if(_reverseConnection)
        {
            out << "reverse ";
        }
        out << "routing";
        if(_requestQueue)
        {
            out << " (buffered)";
        }
        else
        {
            out << " (not buffered)";
        }
        if(_reverseConnection)
        {
            out << "\nidentity = " << _instance->communicator()->identityToString(proxy->ice_getIdentity());
        }
        else
        {
            out << "\nproxy = " << _instance->communicator()->proxyToString(proxy);
        }
        out << "\noperation = " << current.operation;
        out << "\ncontext = ";
        Context::const_iterator q = current.ctx.begin();
        while(q != current.ctx.end())
        {
            out << q->first << '/' << q->second;
            if(++q != current.ctx.end())
            {
                out << ", ";
            }
        }
    }

    if(_requestQueue)
    {    
        //
        // If we are in buffered mode, we create a new request and add
        // it to the request queue. If the request is twoway, we use
        // AMI.
        //

        bool override;
        try
        {
            override = _requestQueue->addRequest(new Request(proxy, inParams, current, _forwardContext, _context,
                                                             amdCB));
        }
        catch(const ObjectNotExistException& ex)
        {
            amdCB->ice_exception(ex);
            return;
        }

        if(override && _overrideTraceLevel >= 1)
        {
            Trace out(_instance->logger(), "Glacier2");
            if(_reverseConnection)
            {
                out << "reverse ";
            }
            out << "routing override";
            if(_reverseConnection)
            {
                out << "\nidentity = " << _instance->communicator()->identityToString(proxy->ice_getIdentity());
            }
            else
            {
                out << "\nproxy = " << _instance->communicator()->proxyToString(proxy);
            }
            out << "\noperation = " << current.operation;
            out << "\ncontext = ";
            Context::const_iterator q = current.ctx.begin();
            while(q != current.ctx.end())
            {
                out << q->first << '/' << q->second;
                if(++q != current.ctx.end())
                {
                    out << ", ";
                }
            }
        }
    }
    else
    {
        //
        // If we are in not in buffered mode, we send the request
        // directly.
        //
        assert(!proxy->ice_isBatchOneway() && !proxy->ice_isBatchDatagram());

        try
        {
            AMI_Array_Object_ice_invokePtr amiCB;
            Ice::AMISentCallback* sentCB = 0;
            if(proxy->ice_isTwoway())
            {
                amiCB = new AMI_Array_Object_ice_invokeTwowayI(amdCB, _instance, _reverseConnection);
            }
            else
            {
                AMI_Array_Object_ice_invokeOnewayI* cb = 
                    new AMI_Array_Object_ice_invokeOnewayI(amdCB, _instance, _reverseConnection);
                amiCB = cb;
                sentCB = cb;
            }

            bool sent;
            if(_forwardContext)
            {
                if(_context.size() > 0)
                {
                    Ice::Context ctx = current.ctx;
                    ctx.insert(_context.begin(), _context.end());
                    sent = proxy->ice_invoke_async(amiCB, current.operation, current.mode, inParams, ctx);
                }
                else
                {
                    sent = proxy->ice_invoke_async(amiCB, current.operation, current.mode, inParams, current.ctx);
                }
            }
            else
            {
                if(_context.size() > 0)
                {
                    sent = proxy->ice_invoke_async(amiCB, current.operation, current.mode, inParams, _context);
                }
                else
                {
                    sent = proxy->ice_invoke_async(amiCB, current.operation, current.mode, inParams);
                }
            }

            if(sent && sentCB)
            {
                sentCB->ice_sent();
            }
        }
        catch(const LocalException& ex)
        {
            amdCB->ice_exception(ex);
        }
    }
}
