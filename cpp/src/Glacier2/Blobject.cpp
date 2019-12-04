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
const string serverTraceRequest = "Glacier2.Server.Trace.Request";
const string clientTraceRequest = "Glacier2.Client.Trace.Request";
const string serverTraceOverride = "Glacier2.Server.Trace.Override";
const string clientTraceOverride = "Glacier2.Client.Trace.Override";

}

Glacier2::Blobject::Blobject(shared_ptr<Instance> instance, shared_ptr<Connection> reverseConnection,
                             const Context& context) :
    _instance(move(instance)),
    _reverseConnection(move(reverseConnection)),
    _forwardContext(_reverseConnection ?
                    _instance->properties()->getPropertyAsInt(serverForwardContext) > 0 :
                    _instance->properties()->getPropertyAsInt(clientForwardContext) > 0),
    _requestTraceLevel(_reverseConnection ?
                       _instance->properties()->getPropertyAsInt(serverTraceRequest) :
                       _instance->properties()->getPropertyAsInt(clientTraceRequest)),
    _overrideTraceLevel(reverseConnection ?
                        _instance->properties()->getPropertyAsInt(serverTraceOverride) :
                        _instance->properties()->getPropertyAsInt(clientTraceOverride)),
    _context(context)
{
    auto t = _reverseConnection ? _instance->serverRequestQueueThread() : _instance->clientRequestQueueThread();
    if(t)
    {
        const_cast<shared_ptr<RequestQueue>&>(_requestQueue) = make_shared<RequestQueue>(t,
                                                                                         _instance,
                                                                                         _reverseConnection);
    }
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
Glacier2::Blobject::updateObserver(const shared_ptr<Glacier2::Instrumentation::SessionObserver>& observer)
{
    if(_requestQueue)
    {
        _requestQueue->updateObserver(observer);
    }
}

void
Glacier2::Blobject::invoke(shared_ptr<ObjectPrx>& proxy,
                           const std::pair<const Byte*, const Byte*>& inParams,
                           function<void(bool, const pair<const Byte*, const Byte*>&)> response,
                           function<void(exception_ptr)> exception,
                           const Current& current)
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
        proxy = proxy->ice_oneway();
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
                    proxy = proxy->ice_oneway();
                    break;
                }

                case 'd':
                {
                    proxy = proxy->ice_datagram();
                    break;
                }

                case 'O':
                {
                    // Batch support has been removed. These requests will be forwarded as oneway
                    proxy = proxy->ice_oneway();
                    break;
                }

                case 'D':
                {
                    // Batch support has been removed. These requests will be forwarded as datagram
                    proxy = proxy->ice_datagram();
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
            override = _requestQueue->addRequest(make_shared<Request>(proxy, inParams, current, _forwardContext,
                                                                      _context, move(response), exception));
        }
        catch(const ObjectNotExistException&)
        {
            exception(current_exception());
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
        // If we are in not in buffered mode, we send the request directly.
        //

        try
        {
            function<void(bool, pair<const Byte*, const Byte*>)> amiResponse = nullptr;
            function<void(bool)> amiSent = nullptr;

            function<void(exception_ptr)> amiException = [self = shared_from_this(),
                                                          amdException = exception](exception_ptr ex)
                {
                    //
                    // If the connection has been lost, destroy the session.
                    //
                    if(self->_reverseConnection)
                    {
                        try
                        {
                            rethrow_exception(ex);
                        }
                        catch(const Ice::LocalException& e)
                        {
                            if(dynamic_cast<const SocketException*>(&e) ||
                            dynamic_cast<const TimeoutException*>(&e) ||
                            dynamic_cast<const ProtocolException*>(&e))
                            {
                                try
                                {
                                    self->_instance->sessionRouter()->destroySession(self->_reverseConnection);
                                }
                                catch(const Exception&)
                                {
                                }
                            }
                        }
                    }

                    amdException(ex);
                };

            if(proxy->ice_isTwoway())
            {
                amiResponse = move(response);
            }
            else
            {
                amiSent = [amdResponse = move(response)](bool)
                    {
                        amdResponse(true, {nullptr, nullptr});
                    };
            }

            if(_forwardContext)
            {
                if(_context.size() > 0)
                {
                    Context ctx = current.ctx;
                    ctx.insert(_context.begin(), _context.end());
                    proxy->ice_invokeAsync(current.operation, current.mode, inParams,
                                           move(amiResponse), move(amiException), move(amiSent), ctx);
                }
                else
                {
                    proxy->ice_invokeAsync(current.operation, current.mode, inParams,
                                           move(amiResponse), move(amiException), move(amiSent), current.ctx);
                }
            }
            else
            {
                if(_context.size() > 0)
                {
                    proxy->ice_invokeAsync(current.operation, current.mode, inParams,
                                           move(amiResponse), move(amiException), move(amiSent), _context);
                }
                else
                {
                    proxy->ice_invokeAsync(current.operation, current.mode, inParams,
                                           move(amiResponse), move(amiException), move(amiSent));
                }
            }
        }
        catch(const LocalException&)
        {
            exception(current_exception());
        }
    }
}
