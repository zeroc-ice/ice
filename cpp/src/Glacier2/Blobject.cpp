//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Glacier2/Blobject.h>
#include <Glacier2/SessionRouterI.h>
#include <Glacier2/Instrumentation.h>

using namespace std;
using namespace Ice;
using namespace Glacier2;
using namespace Glacier2::Instrumentation;

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

}

Glacier2::Blobject::Blobject(const InstancePtr& instance, const ConnectionPtr& reverseConnection,
                             const Context& context) :
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
Glacier2::Blobject::updateObserver(const Glacier2::Instrumentation::SessionObserverPtr& observer)
{
    if(_requestQueue)
    {
        _requestQueue->updateObserver(observer);
    }
}

void
Glacier2::Blobject::invokeResponse(bool ok, const pair<const Byte*, const Byte*>& outParams,
                                   const AMD_Object_ice_invokePtr& amdCB)
{
    amdCB->ice_response(ok, outParams);
}

void
Glacier2::Blobject::invokeSent(bool, const AMD_Object_ice_invokePtr& amdCB)
{
#if (defined(_MSC_VER) && (_MSC_VER >= 1600))
    amdCB->ice_response(true, pair<const Byte*, const Byte*>(static_cast<const Byte*>(nullptr),
                                                             static_cast<const Byte*>(nullptr)));
#else
    amdCB->ice_response(true, pair<const Byte*, const Byte*>(0, 0));
#endif
}

void
Glacier2::Blobject::invokeException(const Exception& ex, const AMD_Object_ice_invokePtr& amdCB)
{
    amdCB->ice_exception(ex);
}

void
Glacier2::Blobject::invoke(ObjectPrx& proxy, const AMD_Object_ice_invokePtr& amdCB,
                           const std::pair<const Byte*, const Byte*>& inParams, const Current& current)
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
            Callback_Object_ice_invokePtr amiCB;
            if(proxy->ice_isTwoway())
            {
                amiCB = newCallback_Object_ice_invoke(this, &Blobject::invokeResponse, &Blobject::invokeException);
            }
            else
            {
                amiCB = newCallback_Object_ice_invoke(this, &Blobject::invokeException, &Blobject::invokeSent);
            }

            if(_forwardContext)
            {
                if(_context.size() > 0)
                {
                    Context ctx = current.ctx;
                    ctx.insert(_context.begin(), _context.end());
                    proxy->begin_ice_invoke(current.operation, current.mode, inParams, ctx, amiCB, amdCB);
                }
                else
                {
                    proxy->begin_ice_invoke(current.operation, current.mode, inParams, current.ctx, amiCB, amdCB);
                }
            }
            else
            {
                if(_context.size() > 0)
                {
                    proxy->begin_ice_invoke(current.operation, current.mode, inParams, _context, amiCB, amdCB);
                }
                else
                {
                    proxy->begin_ice_invoke(current.operation, current.mode, inParams, amiCB, amdCB);
                }
            }
        }
        catch(const LocalException& ex)
        {
            amdCB->ice_exception(ex);
        }
    }
}
