//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading.Tasks;

namespace IceInternal
{
    internal static class Incoming
    {
        internal static void ReportException(System.Exception exc,
            Ice.Instrumentation.IDispatchObserver? dispatchObserver, Ice.Current current)
        {
            bool userException = exc is Ice.UserException;

            if (!userException && current.Adapter.Communicator.GetPropertyAsInt("Ice.Warn.Dispatch") > 0)
            {
                Warning(exc, current);
            }

            if (userException)
            {
                dispatchObserver?.UserException();
            }
            else
            {
                if (dispatchObserver != null)
                {
                    // TODO: temporary, should just use ToString all the time
                    if (exc is Ice.Exception ex)
                    {
                        dispatchObserver.Failed(ex.ice_id());
                    }
                    else
                    {
                        dispatchObserver.Failed(exc.GetType().FullName);
                    }
                }
            }
        }

        private static void Warning(System.Exception ex, Ice.Current current)
        {
            Debug.Assert(current.Adapter.Communicator != null);

            var output = new System.Text.StringBuilder();

            output.Append("dispatch exception:");
            output.Append("\nidentity: ").Append(current.Id.ToString(current.Adapter.Communicator.ToStringMode));
            output.Append("\nfacet: ").Append(IceUtilInternal.StringUtil.EscapeString(current.Facet, "",
                current.Adapter.Communicator.ToStringMode));
            output.Append("\noperation: ").Append(current.Operation);
            if (current.Connection != null)
            {
                try
                {
                    for (Ice.ConnectionInfo? p = current.Connection.GetConnectionInfo(); p != null; p = p.Underlying)
                    {
                        if (p is Ice.IPConnectionInfo ipinfo)
                        {
                            output.Append("\nremote host: ").Append(ipinfo.RemoteAddress)
                                  .Append(" remote port: ")
                                  .Append(ipinfo.RemotePort);
                            break;
                        }
                    }
                }
                catch (Ice.LocalException)
                {
                }
            }
            output.Append("\n");
            output.Append(ex.ToString());
            current.Adapter.Communicator.Logger.Warning(output.ToString());
        }
    }
}
