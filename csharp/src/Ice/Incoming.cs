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
        internal static async ValueTask InvokeAsync(Ice.Communicator communicator, IResponseHandler responseHandler,
            Ice.Connection? connection, Ice.ObjectAdapter adapter, byte compress, int requestId, Ice.InputStream istr)
        {
            int start = istr.Pos;
            var identity = new Ice.Identity(istr);

            // For compatibility with the old FacetPath.
            string[] facetPath = istr.ReadStringArray();
            string facet;
            if (facetPath.Length > 0)
            {
                if (facetPath.Length > 1)
                {
                    throw new Ice.MarshalException();
                }
                facet = facetPath[0];
            }
            else
            {
                facet = "";
            }

            string operation = istr.ReadString();
            byte mode = istr.ReadByte();
            var context = new Dictionary<string, string>();
            int sz = istr.ReadSize();
            while (sz-- > 0)
            {
                string first = istr.ReadString();
                string second = istr.ReadString();
                context[first] = second;
            }

            var current = new Ice.Current(adapter, identity, facet, operation, (Ice.OperationMode)mode, context,
                requestId, connection, istr.StartEncapsulation(), responseHandler, compress);

            Ice.Instrumentation.ICommunicatorObserver? obsv = communicator.Observer;
            if (obsv != null)
            {
                int encapsSize = istr.GetEncapsulationSize();

                Ice.Instrumentation.IDispatchObserver? dispatchObserver =
                    obsv.GetDispatchObserver(current, istr.Pos - start + encapsSize);
                if (dispatchObserver != null)
                {
                    dispatchObserver.Attach();
                    current.DispatchObserver = dispatchObserver;
                }
            }

            Ice.IObject? servant = current.Adapter.Find(current.Id, current.Facet);

            if (servant == null)
            {
                try
                {
                    throw new Ice.ObjectNotExistException(current.Id, current.Facet, current.Operation);
                }
                catch (Exception ex)
                {
                    istr.SkipCurrentEncapsulation(); // Required for batch requests, and incoming batch requests
                                                     // are still supported in Ice 4.x.
                    HandleException(ex, false, current);
                    return;
                }
            }

            bool amd = true;
            try
            {
                ValueTask<Ice.OutputStream> vt = servant.DispatchAsync(istr, current);
                if (vt.IsCompleted)
                {
                    amd = false;
                    Completed(vt.Result, null, amd, current);
                }
                else
                {
                    Ice.OutputStream ostr = await vt.ConfigureAwait(false);
                    Completed(vt.Result, null, amd, current);
                }
            }
            catch (System.Exception ex)
            {
                Completed(null, ex, amd, current);
            }
        }

        private static void Completed(Ice.OutputStream? ostr, System.Exception? exc, bool amd, Ice.Current current)
        {
            Debug.Assert(current.ResponseHandler != null, "null response handler");
            try
            {
                if (exc != null)
                {
                    HandleException(exc, amd, current);
                }
                else if (!current.IsOneway)
                {
                    Debug.Assert(ostr != null, "null output stream");
                    current.DispatchObserver?.Reply(ostr.Size - Protocol.headerSize - 4);
                    current.ResponseHandler.SendResponse(current.RequestId, ostr, current.CompressionStatus, amd);
                }
                else
                {
                    current.ResponseHandler.SendNoResponse();
                }
            }
            catch (Ice.LocalException ex)
            {
                current.ResponseHandler.InvokeException(current.RequestId, ex, 1, amd);
            }
            finally
            {
                current.DispatchObserver?.Detach();
                current.DispatchObserver = null;
            }
        }

        private static void HandleException(System.Exception exc, bool amd, Ice.Current current)
        {
            Debug.Assert(current.ResponseHandler != null);

            if (exc is Ice.SystemException)
            {
                if (current.ResponseHandler.SystemException(current.RequestId, (Ice.SystemException)exc, amd))
                {
                    return;
                }
            }

            bool userException = exc is Ice.UserException;

            if (!userException && current.Adapter.Communicator.GetPropertyAsInt("Ice.Warn.Dispatch") > 0)
            {
                Warning(exc, current);
            }

            if (userException)
            {
                current.DispatchObserver?.UserException();
            }
            else
            {
                if (current.DispatchObserver != null)
                {
                    // TODO: temporary, should just use ToString all the time
                    if (exc is Ice.Exception ex)
                    {
                        current.DispatchObserver.Failed(ex.ice_id());
                    }
                    else
                    {
                        current.DispatchObserver.Failed(exc.GetType().FullName);
                    }
                }
            }

            if (current.IsOneway)
            {
                current.ResponseHandler.SendNoResponse();
            }
            else
            {
                Ice.OutputStream ostr = Protocol.CreateFailureResponseFrame(exc, current);
                current.DispatchObserver?.Reply(ostr.Size - Protocol.headerSize - 4);
                current.ResponseHandler.SendResponse(current.RequestId, ostr, current.CompressionStatus, amd);
            }

            current.DispatchObserver?.Detach();
            current.DispatchObserver = null;
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
