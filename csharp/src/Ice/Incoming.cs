//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;

namespace IceInternal
{
    internal static class Incoming
    {
        internal static void ReportException(Ice.RemoteException exc,
                                             Ice.Instrumentation.IDispatchObserver? dispatchObserver,
                                             Ice.Current current)
        {
            bool unhandledException = exc is Ice.UnhandledException;

            if (unhandledException && current.Adapter.Communicator.GetPropertyAsInt("Ice.Warn.Dispatch") > 0)
            {
                Warning((Ice.UnhandledException)exc, current);
            }

            if (dispatchObserver != null)
            {
                if (unhandledException)
                {
                    System.Exception realEx = exc.InnerException ?? exc;
                    if (realEx is Ice.LocalException iceLocalEx)
                    {
                        dispatchObserver.Failed(iceLocalEx.ice_id()); // TODO: refactor
                    }
                    else
                    {
                        dispatchObserver.Failed(realEx.GetType().FullName);
                    }
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
