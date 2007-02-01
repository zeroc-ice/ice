// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Glacier2/Blobject.h>

using namespace std;
using namespace Ice;
using namespace Glacier2;

static const string serverForwardContext = "Glacier2.Server.ForwardContext";
static const string clientForwardContext = "Glacier2.Client.ForwardContext";
static const string serverBuffered = "Glacier2.Server.Buffered";
static const string clientBuffered = "Glacier2.Client.Buffered";
static const string serverAlwaysBatch = "Glacier2.Server.AlwaysBatch";
static const string clientAlwaysBatch = "Glacier2.Client.AlwaysBatch";
static const string serverTraceRequest = "Glacier2.Server.Trace.Request";
static const string clientTraceRequest = "Glacier2.Client.Trace.Request";
static const string serverTraceOverride = "Glacier2.Server.Trace.Override";
static const string clientTraceOverride = "Glacier2.Client.Trace.Override";
static const string serverSleepTime = "Glacier2.Server.SleepTime";
static const string clientSleepTime = "Glacier2.Client.SleepTime";

Glacier2::Blobject::Blobject(const CommunicatorPtr& communicator, bool reverse, const Ice::Context& sslContext) :
    _communicator(communicator),
    _properties(_communicator->getProperties()),
    _logger(_communicator->getLogger()),
    _reverse(reverse),
    _forwardContext(_reverse ?
                    _properties->getPropertyAsInt(serverForwardContext) > 0 :
                    _properties->getPropertyAsInt(clientForwardContext) > 0),
    _buffered(_reverse ?
                _properties->getPropertyAsIntWithDefault(serverBuffered, 1) > 0 :
                _properties->getPropertyAsIntWithDefault(clientBuffered, 1) > 0),
    _alwaysBatch(_reverse ?
                 _properties->getPropertyAsInt(serverAlwaysBatch) > 0 :
                 _properties->getPropertyAsInt(clientAlwaysBatch) > 0),
    _requestTraceLevel(_reverse ?
                       _properties->getPropertyAsInt(serverTraceRequest) :
                       _properties->getPropertyAsInt(clientTraceRequest)),
    _overrideTraceLevel(reverse ?
                        _properties->getPropertyAsInt(serverTraceOverride) :
                        _properties->getPropertyAsInt(clientTraceOverride)),
    _sslContext(sslContext)
{

    if(_buffered)
    {
        try
        {
            IceUtil::Time sleepTime = _reverse ?
                IceUtil::Time::milliSeconds(_properties->getPropertyAsInt(serverSleepTime)) :
                IceUtil::Time::milliSeconds(_properties->getPropertyAsInt(clientSleepTime));

            const_cast<RequestQueuePtr&>(_requestQueue) = new RequestQueue(sleepTime);
            
            Int threadStackSize = _properties->getPropertyAsInt("Ice.ThreadPerConnection.StackSize");
            
            _requestQueue->start(static_cast<size_t>(threadStackSize));

            //
            // See the comment in Glacier2::RequestQueue::destroy()
            // for why we detach the thread.
            //
            _requestQueue->getThreadControl().detach();
        }
        catch(const IceUtil::Exception& ex)
        {
            {
                Error out(_logger);
                out << "cannot create thread for request queue:\n" << ex;
            }

            if(_requestQueue)
            {
                _requestQueue->destroy();
            }
            
            throw;
        }
    }
}

Glacier2::Blobject::~Blobject()
{
}

void
Glacier2::Blobject::destroy()
{
    _requestQueue->destroy();
}

void
Glacier2::Blobject::invoke(ObjectPrx& proxy, const AMD_Array_Object_ice_invokePtr& amdCB, 
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
        if(_alwaysBatch && _buffered)
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
                    if(_alwaysBatch && _buffered)
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
                    if(_alwaysBatch && _buffered)
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
                    if(_buffered)
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
                    if(_buffered)
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
                    Warning out(_logger);
                    out << "unknown forward option `" << option << "'";
                    break;
                }
            }
        }
    }
    
    if(_requestTraceLevel >= 1)
    {
        Trace out(_logger, "Glacier2");
        if(_reverse)
        {
            out << "reverse ";
        }
        out << "routing";
        if(_buffered)
        {
            out << " (buffered)";
        }
        else
        {
            out << " (not buffered)";
        }
        if(_reverse)
        {
            out << "\nidentity = " << _communicator->identityToString(proxy->ice_getIdentity());
        }
        else
        {
            out << "\nproxy = " << _communicator->proxyToString(proxy);
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

    if(_buffered)
    {    
        //
        // If we are in buffered mode, we create a new request and add
        // it to the request queue. If the request is twoway, we use
        // AMI.
        //

        bool override;
        try
        {
            override = 
                _requestQueue->addRequest(new Request(proxy, inParams, current, _forwardContext, _sslContext, amdCB));
        }
        catch(const ObjectNotExistException& ex)
        {
            amdCB->ice_exception(ex);
            return;
        }

        if(override && _overrideTraceLevel >= 1)
        {
            Trace out(_logger, "Glacier2");
            if(_reverse)
            {
                out << "reverse ";
            }
            out << "routing override";
            if(_reverse)
            {
                out << "\nidentity = " << _communicator->identityToString(proxy->ice_getIdentity());
            }
            else
            {
                out << "\nproxy = " << _communicator->proxyToString(proxy);
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

        bool ok;
        ByteSeq outParams;

        try
        {
            if(_forwardContext)
            {
                if(_sslContext.size() > 0)
                {
                    Ice::Context ctx = current.ctx;
                    ctx.insert(_sslContext.begin(), _sslContext.end());
                    ok = proxy->ice_invoke(current.operation, current.mode, inParams, outParams, ctx);
                }
                else
                {
                    ok = proxy->ice_invoke(current.operation, current.mode, inParams, outParams, current.ctx);
                }
            }
            else
            {
                if(_sslContext.size() > 0)
                {
                    ok = proxy->ice_invoke(current.operation, current.mode, inParams, outParams, _sslContext);
                }
                else
                {
                    ok = proxy->ice_invoke(current.operation, current.mode, inParams, outParams);
                }
            }

            pair<const Byte*, const Byte*> outPair;
            if(outParams.size() == 0)
            {
                outPair.first = outPair.second = 0;
            }
            else
            {
                outPair.first = &outParams[0];
                outPair.second = outPair.first + outParams.size();
            }
            amdCB->ice_response(ok, outPair);
        }
        catch(const LocalException& ex)
        {
            amdCB->ice_exception(ex);
        }
    }
}
