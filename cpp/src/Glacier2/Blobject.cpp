// Copyright (c) ZeroC, Inc.

#include "Blobject.h"
#include "Instrumentation.h"
#include "SessionRouterI.h"

using namespace std;
using namespace Ice;
using namespace Glacier2;
using namespace Glacier2::Instrumentation;

namespace
{
    constexpr string_view serverForwardContext = "Glacier2.Server.ForwardContext";
    constexpr string_view clientForwardContext = "Glacier2.Client.ForwardContext";
    constexpr string_view serverTraceRequest = "Glacier2.Server.Trace.Request";
    constexpr string_view clientTraceRequest = "Glacier2.Client.Trace.Request";
}

Glacier2::Blobject::Blobject(shared_ptr<Instance> instance, ConnectionPtr reverseConnection, Context context)
    : _instance(std::move(instance)),
      _reverseConnection(std::move(reverseConnection)),
      _forwardContext(
          _reverseConnection ? _instance->properties()->getIcePropertyAsInt(serverForwardContext) > 0
                             : _instance->properties()->getIcePropertyAsInt(clientForwardContext) > 0),
      _requestTraceLevel(
          _reverseConnection ? _instance->properties()->getIcePropertyAsInt(serverTraceRequest)
                             : _instance->properties()->getIcePropertyAsInt(clientTraceRequest)),
      _context(std::move(context))
{
}

void
Glacier2::Blobject::invoke(
    ObjectPrx& proxy,
    pair<const byte*, const byte*> inParams,
    function<void(bool, pair<const byte*, const byte*>)> response,
    function<void(exception_ptr)> exception, // NOLINT(performance-unnecessary-value-param)
    const Current& current)
{
    //
    // Set the correct facet on the proxy.
    //
    if (!current.facet.empty())
    {
        proxy = proxy->ice_facet(current.facet);
    }

    //
    // Modify the proxy according to the request id. This can
    // be overridden by the _fwd context.
    //
    if (current.requestId == 0)
    {
        proxy = proxy->ice_oneway();
    }
    else if (current.requestId > 0)
    {
        proxy = proxy->ice_twoway();
    }

    //
    // Modify the proxy according to the _fwd context field.
    //
    auto p = current.ctx.find("_fwd");
    if (p != current.ctx.end())
    {
        for (unsigned int i = 0; i < p->second.length(); ++i)
        {
            char option = p->second[i];
            switch (option)
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
                    out << "unknown forward option '" << option << "'";
                    break;
                }
            }
        }
    }

    if (_requestTraceLevel >= 1)
    {
        Trace out(_instance->logger(), "Glacier2");
        if (_reverseConnection)
        {
            out << "reverse ";
        }
        out << "routing";
        if (_reverseConnection)
        {
            out << "\nidentity = " << _instance->communicator()->identityToString(proxy->ice_getIdentity());
        }
        else
        {
            out << "\nproxy = " << proxy;
        }
        out << "\noperation = " << current.operation;
        out << "\ncontext = ";
        auto q = current.ctx.begin();
        while (q != current.ctx.end())
        {
            out << q->first << '/' << q->second;
            if (++q != current.ctx.end())
            {
                out << ", ";
            }
        }
    }

    try
    {
        function<void(bool, pair<const byte*, const byte*>)> amiResponse = nullptr;
        function<void(bool)> amiSent = nullptr;

        if (proxy->ice_isTwoway())
        {
            amiResponse = std::move(response);
        }
        else
        {
            // For oneway requests, we want the dispatch to complete only once the request has been forwarded (sent).
            // This ensures proper flow control / back pressure through the Glacier2 router.
            amiSent = [amdResponse = std::move(response)](bool) { amdResponse(true, {nullptr, nullptr}); };
        }

        // We can't move exception because we use it in the catch block below.
        if (_forwardContext)
        {
            if (_context.size() > 0)
            {
                Context ctx = current.ctx;
                ctx.insert(_context.begin(), _context.end());
                proxy->ice_invokeAsync(
                    current.operation,
                    current.mode,
                    inParams,
                    std::move(amiResponse),
                    exception,
                    std::move(amiSent),
                    ctx);
            }
            else
            {
                proxy->ice_invokeAsync(
                    current.operation,
                    current.mode,
                    inParams,
                    std::move(amiResponse),
                    exception,
                    std::move(amiSent),
                    current.ctx);
            }
        }
        else
        {
            if (_context.size() > 0)
            {
                proxy->ice_invokeAsync(
                    current.operation,
                    current.mode,
                    inParams,
                    std::move(amiResponse),
                    exception,
                    std::move(amiSent),
                    _context);
            }
            else
            {
                proxy->ice_invokeAsync(
                    current.operation,
                    current.mode,
                    inParams,
                    std::move(amiResponse),
                    exception,
                    std::move(amiSent));
            }
        }
    }
    catch (const LocalException&)
    {
        exception(current_exception());
    }
}
