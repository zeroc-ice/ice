//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;

namespace IceInternal
{
    internal static class Incoming
    {
        internal static void ReportException(Ice.RemoteException ex,
                                             Ice.Instrumentation.IDispatchObserver? dispatchObserver,
                                             Ice.Current current)
        {
            bool unhandledException = ex is Ice.UnhandledException;

            if (unhandledException && (current.Adapter.Communicator.GetPropertyAsBool("Ice.Warn.Dispatch") ?? false))
            {
                Warning((Ice.UnhandledException)ex, current);
            }

            if (dispatchObserver != null)
            {
                if (unhandledException)
                {
                    dispatchObserver.Failed((ex.InnerException ?? ex).GetType().FullName ?? "System.Exception");
                }
                else
                {
                    dispatchObserver.RemoteException();
                }
            }
        }

        private static void Warning(Ice.UnhandledException ex, Ice.Current current)
        {
            Debug.Assert(current.Adapter.Communicator != null);

            var output = new System.Text.StringBuilder();

            output.Append("dispatch exception:");
            output.Append("\nidentity: ").Append(current.Identity.ToString(current.Adapter.Communicator.ToStringMode));
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
                catch (System.Exception)
                {
                }
            }
            output.Append("\n");
            output.Append(ex.ToString());
            current.Adapter.Communicator.Logger.Warning(output.ToString());
        }
    }
}
