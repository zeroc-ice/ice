//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;

namespace Ice
{
    public interface IMarshaledReturnValue
    {
        OutputStream GetOutputStream(Current current);
    };
}

namespace IceInternal
{
    public class Incoming : Ice.IRequest
    {
        public Incoming(Ice.Communicator communicator, IResponseHandler handler, Ice.Connection? connection,
                        Ice.ObjectAdapter adapter, bool response, byte compress, int requestId)
        {
            _communicator = communicator;
            _responseHandler = handler;
            _response = response;
            _compress = compress;

            _adapter = adapter;
            _connection = connection;
            _requestId = requestId;
        }

        //
        // These functions allow this object to be reused, rather than reallocated.
        //
        public void Reset(Ice.Communicator communicator, IResponseHandler handler, Ice.Connection connection,
                          Ice.ObjectAdapter adapter, bool response, byte compress, int requestId)
        {
            _communicator = communicator;
            _responseHandler = handler;
            _response = response;
            _compress = compress;

            //
            // Don't recycle the Current object, because servants may keep a reference to it.
            //
            _current = null;
            _adapter = adapter;
            _connection = connection;
            _requestId = requestId;

            _inParamPos = -1;
        }

        public bool Reclaim()
        {
            if (_responseHandler != null) // Async dispatch not ready for being reclaimed!
            {
                return false;
            }

            _current = null;
            _servant = null;

            //_observer = null;
            Debug.Assert(_observer == null);

            if (_os != null)
            {
                _os.Reset(); // Reset the output stream to prepare it for re-use.
            }

            _is = null;

            //_responseHandler = null;
            Debug.Assert(_responseHandler == null);

            _inParamPos = -1;

            return true;
        }

        public Ice.Current? GetCurrent() => _current;

        public void Invoke(Ice.InputStream stream)
        {
            _is = stream;

            int start = _is.Pos;
            //
            // Read the current.
            //
            var id = new Ice.Identity(_is);

            //
            // For compatibility with the old FacetPath.
            //
            string[] facetPath = _is.ReadStringSeq();
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

            string operation = _is.ReadString();
            byte mode = _is.ReadByte();
            var context = new Dictionary<string, string>();
            int sz = _is.ReadSize();
            while (sz-- > 0)
            {
                string first = _is.ReadString();
                string second = _is.ReadString();
                context[first] = second;
            }
            _current = new Ice.Current(_adapter, id, facet, operation, (Ice.OperationMode)mode, context, _requestId, _connection);
            Ice.Instrumentation.ICommunicatorObserver? obsv = _communicator.Observer;
            if (obsv != null)
            {
                // Read the encapsulation size.
                int size = _is.ReadInt();
                _is.Pos -= 4;

                _observer = obsv.GetDispatchObserver(_current, _is.Pos - start + size);
                if (_observer != null)
                {
                    _observer.Attach();
                }
            }

            //
            // Don't put the code above into the try block below. Exceptions
            // in the code above are considered fatal, and must propagate to
            // the caller of this operation.
            //
            _servant = _adapter.Find(_current.Id, _current.Facet);

            if (_servant == null)
            {
                try
                {
                    throw new Ice.ObjectNotExistException(_current.Id, _current.Facet, _current.Operation);
                }
                catch (Exception ex)
                {
                    SkipReadParams(); // Required for batch requests
                    HandleException(ex, false);
                    return;
                }
            }

            try
            {
                Task<Ice.OutputStream?>? task = _servant.Dispatch(this, _current);
                if (task == null)
                {
                    Completed(null, false);
                }
                else
                {
                    if (task.IsCompleted)
                    {
                        _os = task.GetAwaiter().GetResult(); // Get the response
                        Completed(null, false);
                    }
                    else
                    {
                        task.ContinueWith((Task<Ice.OutputStream> t) =>
                        {
                            try
                            {
                                _os = t.GetAwaiter().GetResult();
                                Completed(null, true); // true = asynchronous
                            }
                            catch (Exception ex)
                            {
                                Completed(ex, true); // true = asynchronous
                            }
                        },
                        CancellationToken.None,
                        TaskContinuationOptions.ExecuteSynchronously,
                        scheduler: TaskScheduler.Current);
                    }
                }
            }
            catch (Exception ex)
            {
                Completed(ex, false);
            }
        }

        public Task<Ice.OutputStream?>? SetResult(Ice.OutputStream? os)
        {
            _os = os;
            return null; // Response is cached in the Incoming to not have to create unecessary Task
        }

        public Task<Ice.OutputStream>? SetMarshaledResult<T>(T result) where T : struct, Ice.IMarshaledReturnValue
        {
            Debug.Assert(_current != null);
            _os = result.GetOutputStream(_current);
            return null; // Response is cached in the Incoming to not have to create unecessary Task
        }

        public Task<Ice.OutputStream?>? SetResultTask<R>(Task<R>? task, Action<Ice.OutputStream, R> write)
        {
            if (task == null)
            {
                //
                // Write default constructed response if no task is provided
                //
                Ice.OutputStream os = StartWriteParams();
                write(os, default);
                EndWriteParams(os);
                return SetResult(os);
            }
            else
            {
                Ice.OutputStream? cached = GetAndClearCachedOutputStream(); // If an output stream is cached, re-use it

                //
                // NOTE: it's important that the continuation doesn't mutate the Incoming state to
                // guarantee thread-safety. Multiple continuations can execute concurrently if the
                // user installed a dispatch interceptor and the dispatch is retried.
                //
                return task.ContinueWith((Task<R> t) =>
                {
                    R result = t.GetAwaiter().GetResult();
                    Ice.OutputStream os = StartWriteParams(cached);
                    write(os, result);
                    EndWriteParams(os);
                    return Task.FromResult<Ice.OutputStream?>(os);
                },
                CancellationToken.None,
                TaskContinuationOptions.ExecuteSynchronously,
                TaskScheduler.Current).Unwrap();
            }
        }

        public Task<Ice.OutputStream?>? SetResultTask(Task task)
        {
            if (task == null)
            {
                //
                // Write response if no task is provided
                //
                return SetResult(WriteEmptyParams());
            }
            else
            {
                Ice.OutputStream? cached = GetAndClearCachedOutputStream(); // If an output stream is cached, re-use it

                //
                // NOTE: it's important that the continuation doesn't mutate the Incoming state to
                // guarantee thread-safety. Multiple continuations can execute concurrently if the
                // user installed a dispatch interceptor and the dispatch is retried.
                //
                return task.ContinueWith((Task t) =>
                    {
                        t.GetAwaiter().GetResult();
                        return Task.FromResult(WriteEmptyParams(cached));
                    },
                    CancellationToken.None,
                    TaskContinuationOptions.ExecuteSynchronously,
                    TaskScheduler.Current).Unwrap();
            }
        }

        public Task<Ice.OutputStream?>? SetMarshaledResultTask<T>(Task<T> task) where T : struct, Ice.IMarshaledReturnValue
        {
            Debug.Assert(_current != null);
            if (task == null)
            {
                return SetResult(default(T).GetOutputStream(_current));
            }
            else
            {
                //
                // NOTE: it's important that the continuation doesn't mutate the Incoming state to
                // guarantee thread-safety. Multiple continuations can execute concurrently if the
                // user installed a dispatch interceptor and the dispatch is retried.
                //
                return task.ContinueWith((Task<T> t) =>
                    Task.FromResult<Ice.OutputStream?>(t.GetAwaiter().GetResult().GetOutputStream(_current)),
                        CancellationToken.None,
                        TaskContinuationOptions.ExecuteSynchronously,
                        TaskScheduler.Current).Unwrap();
            }
        }

        private void Completed(Exception? exc, bool amd)
        {
            Debug.Assert(_responseHandler != null, "null response handler");
            Debug.Assert(_current != null, "null current");
            try
            {
                if (exc != null)
                {
                    HandleException(exc, amd);
                }
                else if (_response)
                {
                    Debug.Assert(_os != null, "null output stream");
                    if (_observer != null)
                    {
                        _observer.Reply(_os.Size - Protocol.headerSize - 4);
                    }
                    _responseHandler.SendResponse(_current.RequestId, _os, _compress, amd);
                }
                else
                {
                    _responseHandler.SendNoResponse();
                }
            }
            catch (Ice.LocalException ex)
            {
                _responseHandler.InvokeException(_current.RequestId, ex, 1, amd);
            }
            finally
            {
                if (_observer != null)
                {
                    _observer.Detach();
                    _observer = null;
                }
                _responseHandler = null;
            }
        }

        public void StartOver()
        {
            Debug.Assert(_is != null);
            if (_inParamPos == -1)
            {
                //
                // That's the first startOver, so almost nothing to do
                //
                _inParamPos = _is.Pos;
            }
            else
            {
                //
                // Let's rewind _is, reset _os
                //
                _is.Pos = _inParamPos;
                if (_response && _os != null)
                {
                    _os.Reset();
                }
            }
        }

        public void SkipReadParams()
        {
            Debug.Assert(_is != null);
            Debug.Assert(_current != null);
            //
            // Remember the encoding used by the input parameters, we'll
            // encode the response parameters with the same encoding.
            //
            _current.Encoding = _is.SkipEncapsulation();
        }

        public Ice.InputStream StartReadParams()
        {
            Debug.Assert(_is != null);
            Debug.Assert(_current != null);
            //
            // Remember the encoding used by the input parameters, we'll
            // encode the response parameters with the same encoding.
            //
            _current.Encoding = _is.StartEncapsulation();
            return _is;
        }

        public void EndReadParams()
        {
            Debug.Assert(_is != null);
            _is.EndEncapsulation();
        }

        public void ReadEmptyParams()
        {
            Debug.Assert(_is != null);
            Debug.Assert(_current != null);
            _current.Encoding = _is.SkipEmptyEncapsulation();
        }

        public byte[] ReadParamEncaps()
        {
            Debug.Assert(_is != null);
            Debug.Assert(_current != null);
            byte[] result = _is.ReadEncapsulation(out Ice.EncodingVersion encoding);
            _current.Encoding = encoding;
            return result;
        }

        public void SetFormat(Ice.FormatType format) => _format = format;

        public Ice.OutputStream? GetAndClearCachedOutputStream()
        {
            if (_response)
            {
                Ice.OutputStream? cached = _os;
                _os = null;
                return cached;
            }
            else
            {
                return null; // Don't consume unnecessarily the output stream if we are dispatching a oneway request
            }
        }

        public static Ice.OutputStream CreateResponseOutputStream(Ice.Current current)
        {
            var os = new Ice.OutputStream(current.Adapter!.Communicator, Ice.Util.CurrentProtocolEncoding);
            os.WriteBlob(Protocol.replyHdr);
            os.WriteInt(current.RequestId);
            os.WriteByte(ReplyStatus.replyOK);
            return os;
        }

        private Ice.OutputStream StartWriteParams(Ice.OutputStream? os)
        {
            if (!_response)
            {
                throw new Ice.MarshalException("can't marshal out parameters for oneway dispatch");
            }

            if (os == null) // Create the output stream if none is provided
            {
                os = new Ice.OutputStream(_communicator, Ice.Util.CurrentProtocolEncoding);
            }
            Debug.Assert(_current != null);
            Debug.Assert(os.Pos == 0);
            os.WriteBlob(Protocol.replyHdr);
            os.WriteInt(_current.RequestId);
            os.WriteByte(ReplyStatus.replyOK);
            os.StartEncapsulation(_current.Encoding, _format);
            return os;
        }

        public Ice.OutputStream StartWriteParams() => StartWriteParams(GetAndClearCachedOutputStream());

        public void EndWriteParams(Ice.OutputStream os)
        {
            if (_response)
            {
                os.EndEncapsulation();
            }
        }

        private Ice.OutputStream? WriteEmptyParams(Ice.OutputStream? os)
        {
            if (_response)
            {
                if (os == null) // Create the output stream if none is provided
                {
                    os = new Ice.OutputStream(_communicator, Ice.Util.CurrentProtocolEncoding);
                }
                Debug.Assert(_current != null);
                Debug.Assert(os.Pos == 0);
                os.WriteBlob(Protocol.replyHdr);
                os.WriteInt(_current.RequestId);
                os.WriteByte(ReplyStatus.replyOK);
                os.WriteEmptyEncapsulation(_current.Encoding);
                return os;
            }
            else
            {
                return null;
            }
        }

        public Ice.OutputStream? WriteEmptyParams() => WriteEmptyParams(GetAndClearCachedOutputStream());

        public Ice.OutputStream? WriteParamEncaps(Ice.OutputStream? os, byte[] v, bool ok)
        {
            if (!ok && _observer != null)
            {
                _observer.UserException();
            }

            if (_response)
            {
                if (os == null) // Create the output stream if none is provided
                {
                    os = new Ice.OutputStream(_communicator, Ice.Util.CurrentProtocolEncoding);
                }
                Debug.Assert(_current != null);
                Debug.Assert(os.Pos == 0);
                os.WriteBlob(Protocol.replyHdr);
                os.WriteInt(_current.RequestId);
                os.WriteByte(ok ? ReplyStatus.replyOK : ReplyStatus.replyUserException);
                if (v == null || v.Length == 0)
                {
                    os.WriteEmptyEncapsulation(_current.Encoding);
                }
                else
                {
                    os.WriteEncapsulation(v);
                }
                return os;
            }
            else
            {
                return null;
            }
        }

        private void Warning(Exception ex)
        {
            Debug.Assert(_communicator != null);
            Debug.Assert(_current != null);

            var output = new System.Text.StringBuilder();

            output.Append("dispatch exception:");
            output.Append("\nidentity: ").Append(_current.Id.ToString(_communicator.ToStringMode));
            output.Append("\nfacet: ").Append(IceUtilInternal.StringUtil.escapeString(_current.Facet, "", _communicator.ToStringMode));
            output.Append("\noperation: ").Append(_current.Operation);
            if (_current.Connection != null)
            {
                try
                {
                    for (Ice.ConnectionInfo? p = _current.Connection.GetConnectionInfo(); p != null; p = p.Underlying)
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
            _communicator.Logger.Warning(output.ToString());
        }

        private void HandleException(Exception exc, bool amd)
        {
            Debug.Assert(_current != null);
            Debug.Assert(_responseHandler != null);

            if (exc is Ice.SystemException)
            {
                if (_responseHandler.SystemException(_requestId, (Ice.SystemException)exc, amd))
                {
                    return;
                }
            }

            if (_response)
            {
                //
                // If there's already a response output stream, reset it to re-use it
                //
                if (_os != null)
                {
                    _os.Reset();
                }
                else
                {
                    _os = new Ice.OutputStream(_communicator, Ice.Util.CurrentProtocolEncoding);
                }
            }

            try
            {
                throw exc;
            }
            catch (Ice.RequestFailedException ex)
            {
                if (ex.Id.Name == null || ex.Id.Name.Length == 0)
                {
                    ex.Id = _current.Id;
                }

                if (ex.Facet == null || ex.Facet.Length == 0)
                {
                    ex.Facet = _current.Facet;
                }

                if (ex.Operation == null || ex.Operation.Length == 0)
                {
                    ex.Operation = _current.Operation;
                }

                if (_communicator.GetPropertyAsInt("Ice.Warn.Dispatch") > 1)
                {
                    Warning(ex);
                }

                if (_observer != null)
                {
                    _observer.Failed(ex.ice_id());
                }

                if (_response)
                {
                    Debug.Assert(_os != null);
                    _os.WriteBlob(Protocol.replyHdr);
                    _os.WriteInt(_current.RequestId);
                    if (ex is Ice.ObjectNotExistException)
                    {
                        _os.WriteByte(ReplyStatus.replyObjectNotExist);
                    }
                    else if (ex is Ice.FacetNotExistException)
                    {
                        _os.WriteByte(ReplyStatus.replyFacetNotExist);
                    }
                    else if (ex is Ice.OperationNotExistException)
                    {
                        _os.WriteByte(ReplyStatus.replyOperationNotExist);
                    }
                    else
                    {
                        Debug.Assert(false);
                    }
                    ex.Id.IceWrite(_os);

                    //
                    // For compatibility with the old FacetPath.
                    //
                    if (ex.Facet == null || ex.Facet.Length == 0)
                    {
                        _os.WriteStringSeq(null);
                    }
                    else
                    {
                        string[] facetPath2 = { ex.Facet };
                        _os.WriteStringSeq(facetPath2);
                    }

                    _os.WriteString(ex.Operation);

                    if (_observer != null)
                    {
                        _observer.Reply(_os.Size - Protocol.headerSize - 4);
                    }
                    _responseHandler.SendResponse(_current.RequestId, _os, _compress, amd);
                }
                else
                {
                    _responseHandler.SendNoResponse();
                }
            }
            catch (Ice.UnknownLocalException ex)
            {
                if ((_communicator.GetPropertyAsInt("Ice.Warn.Dispatch") ?? 1) > 0)
                {
                    Warning(ex);
                }

                if (_observer != null)
                {
                    _observer.Failed(ex.ice_id());
                }

                if (_response)
                {
                    Debug.Assert(_os != null);
                    _os.WriteBlob(Protocol.replyHdr);
                    _os.WriteInt(_current.RequestId);
                    _os.WriteByte(ReplyStatus.replyUnknownLocalException);
                    _os.WriteString(ex.Unknown);
                    if (_observer != null)
                    {
                        _observer.Reply(_os.Size - Protocol.headerSize - 4);
                    }
                    _responseHandler.SendResponse(_current.RequestId, _os, _compress, amd);
                }
                else
                {
                    _responseHandler.SendNoResponse();
                }
            }
            catch (Ice.UnknownUserException ex)
            {
                if ((_communicator.GetPropertyAsInt("Ice.Warn.Dispatch") ?? 1) > 0)
                {
                    Warning(ex);
                }

                if (_observer != null)
                {
                    _observer.Failed(ex.ice_id());
                }

                if (_response)
                {
                    Debug.Assert(_os != null);
                    _os.WriteBlob(Protocol.replyHdr);
                    _os.WriteInt(_current.RequestId);
                    _os.WriteByte(ReplyStatus.replyUnknownUserException);
                    _os.WriteString(ex.Unknown);
                    if (_observer != null)
                    {
                        _observer.Reply(_os.Size - Protocol.headerSize - 4);
                    }
                    Debug.Assert(_responseHandler != null && _current != null);
                    _responseHandler.SendResponse(_current.RequestId, _os, _compress, amd);
                }
                else
                {
                    _responseHandler.SendNoResponse();
                }
            }
            catch (Ice.UnknownException ex)
            {
                if ((_communicator.GetPropertyAsInt("Ice.Warn.Dispatch") ?? 1) > 0)
                {
                    Warning(ex);
                }

                if (_observer != null)
                {
                    _observer.Failed(ex.ice_id());
                }

                if (_response)
                {
                    Debug.Assert(_os != null);
                    _os.WriteBlob(Protocol.replyHdr);
                    _os.WriteInt(_current.RequestId);
                    _os.WriteByte(ReplyStatus.replyUnknownException);
                    _os.WriteString(ex.Unknown);
                    if (_observer != null)
                    {
                        _observer.Reply(_os.Size - Protocol.headerSize - 4);
                    }
                    _responseHandler.SendResponse(_current.RequestId, _os, _compress, amd);
                }
                else
                {
                    _responseHandler.SendNoResponse();
                }
            }
            catch (Ice.UserException ex)
            {
                if (_observer != null)
                {
                    _observer.UserException();
                }

                if (_response)
                {
                    Debug.Assert(_os != null);
                    _os.WriteBlob(Protocol.replyHdr);
                    _os.WriteInt(_current.RequestId);
                    _os.WriteByte(ReplyStatus.replyUserException);
                    _os.StartEncapsulation(_current.Encoding, _format);
                    _os.WriteException(ex);
                    _os.EndEncapsulation();
                    if (_observer != null)
                    {
                        _observer.Reply(_os.Size - Protocol.headerSize - 4);
                    }
                    _responseHandler.SendResponse(_current.RequestId, _os, _compress, false);
                }
                else
                {
                    _responseHandler.SendNoResponse();
                }
            }
            catch (Ice.Exception ex)
            {
                if ((_communicator.GetPropertyAsInt("Ice.Warn.Dispatch") ?? 1) > 0)
                {
                    Warning(ex);
                }

                if (_observer != null)
                {
                    _observer.Failed(ex.ice_id());
                }

                if (_response)
                {
                    Debug.Assert(_os != null);
                    _os.WriteBlob(Protocol.replyHdr);
                    _os.WriteInt(_current.RequestId);
                    _os.WriteByte(ReplyStatus.replyUnknownLocalException);
                    _os.WriteString(ex.ice_id() + "\n" + ex.StackTrace);
                    if (_observer != null)
                    {
                        _observer.Reply(_os.Size - Protocol.headerSize - 4);
                    }
                    _responseHandler.SendResponse(_current.RequestId, _os, _compress, amd);
                }
                else
                {
                    _responseHandler.SendNoResponse();
                }
            }
            catch (Exception ex)
            {
                if ((_communicator.GetPropertyAsInt("Ice.Warn.Dispatch") ?? 1) > 0)
                {
                    Warning(ex);
                }

                if (_observer != null)
                {
                    _observer.Failed(ex.GetType().FullName);
                }

                if (_response)
                {
                    Debug.Assert(_os != null);
                    _os.WriteBlob(Protocol.replyHdr);
                    _os.WriteInt(_current.RequestId);
                    _os.WriteByte(ReplyStatus.replyUnknownException);
                    _os.WriteString(ex.ToString());
                    if (_observer != null)
                    {
                        _observer.Reply(_os.Size - Protocol.headerSize - 4);
                    }
                    _responseHandler.SendResponse(_current.RequestId, _os, _compress, amd);
                }
                else
                {
                    _responseHandler.SendNoResponse();
                }
            }

            if (_observer != null)
            {
                _observer.Detach();
                _observer = null;
            }
            _responseHandler = null;
        }

        private Ice.Communicator _communicator;
        private Ice.Current? _current;
        private Ice.IObject? _servant;

        private Ice.Instrumentation.IDispatchObserver? _observer;
        private IResponseHandler? _responseHandler;

        private bool _response;
        private byte _compress;
        private Ice.ObjectAdapter _adapter;
        private Ice.Connection? _connection;
        private int _requestId;
        private Ice.FormatType? _format;

        private Ice.OutputStream? _os;
        private Ice.InputStream? _is;

        private int _inParamPos = -1;

        public Incoming? Next; // For use by Connection.
    }
}
