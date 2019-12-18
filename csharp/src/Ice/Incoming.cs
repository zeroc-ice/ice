//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{

    public interface MarshaledResult
    {
        OutputStream getOutputStream(Current current);
    };

}

namespace IceInternal
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Threading;
    using System.Threading.Tasks;

    public class Incoming : Ice.Request
    {
        public Incoming(Ice.Communicator communicator, ResponseHandler handler, Ice.ConnectionI? connection,
                        Ice.ObjectAdapter adapter, bool response, byte compress, int requestId)
        {
            _communicator = communicator;
            _responseHandler = handler;
            _response = response;
            _compress = compress;

            _adapter = adapter;
            _connection = connection;
            _requestId = requestId;

            _cookie = null;
        }

        //
        // These functions allow this object to be reused, rather than reallocated.
        //
        public void reset(Ice.Communicator communicator, ResponseHandler handler, Ice.ConnectionI connection,
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

            Debug.Assert(_cookie == null);

            _inParamPos = -1;
        }

        public bool reclaim()
        {
            if (_responseHandler != null) // Async dispatch not ready for being reclaimed!
            {
                return false;
            }

            _current = null;
            _servant = null;
            _locator = null;
            _cookie = null;

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

        public Ice.Current? getCurrent()
        {
            return _current;
        }

        public void invoke(ServantManager servantManager, Ice.InputStream stream)
        {
            _is = stream;

            int start = _is.pos();
            //
            // Read the current.
            //
            var id = new Ice.Identity();
            id.ice_readMembers(_is);

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
            Dictionary<string, string> context = new Dictionary<string, string>();
            int sz = _is.ReadSize();
            while (sz-- > 0)
            {
                string first = _is.ReadString();
                string second = _is.ReadString();
                context[first] = second;
            }
            _current = new Ice.Current(_adapter, id, facet, operation, (Ice.OperationMode)mode, context, _requestId, _connection);
            Ice.Instrumentation.CommunicatorObserver? obsv = _communicator.Observer;
            if (obsv != null)
            {
                // Read the encapsulation size.
                int size = _is.ReadInt();
                _is.pos(_is.pos() - 4);

                _observer = obsv.getDispatchObserver(_current, _is.pos() - start + size);
                if (_observer != null)
                {
                    _observer.attach();
                }
            }

            //
            // Don't put the code above into the try block below. Exceptions
            // in the code above are considered fatal, and must propagate to
            // the caller of this operation.
            //

            if (servantManager != null)
            {
                _servant = servantManager.findServant(_current.Id, _current.Facet);
                if (_servant == null)
                {
                    _locator = servantManager.findServantLocator(_current.Id.category);
                    if (_locator == null && _current.Id.category.Length > 0)
                    {
                        _locator = servantManager.findServantLocator("");
                    }

                    if (_locator != null)
                    {
                        Debug.Assert(_locator != null);
                        try
                        {
                            _servant = _locator.locate(_current, out _cookie);
                        }
                        catch (Exception ex)
                        {
                            skipReadParams(); // Required for batch requests.
                            handleException(ex, false);
                            return;
                        }
                    }
                }
            }

            if (_servant == null)
            {
                try
                {
                    if (servantManager != null && servantManager.hasServant(_current.Id))
                    {
                        throw new Ice.FacetNotExistException(_current.Id, _current.Facet, _current.Operation);
                    }
                    else
                    {
                        throw new Ice.ObjectNotExistException(_current.Id, _current.Facet, _current.Operation);
                    }
                }
                catch (Exception ex)
                {
                    skipReadParams(); // Required for batch requests
                    handleException(ex, false);
                    return;
                }
            }

            try
            {
                Task<Ice.OutputStream>? task = _servant(this, _current);
                if (task == null)
                {
                    completed(null, false);
                }
                else
                {
                    if (task.IsCompleted)
                    {
                        _os = task.GetAwaiter().GetResult(); // Get the response
                        completed(null, false);
                    }
                    else
                    {
                        task.ContinueWith((Task<Ice.OutputStream> t) =>
                        {
                            try
                            {
                                _os = t.GetAwaiter().GetResult();
                                completed(null, true); // true = asynchronous
                            }
                            catch (Exception ex)
                            {
                                completed(ex, true); // true = asynchronous
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
                completed(ex, false);
            }
        }

        public Task<Ice.OutputStream>? setResult(Ice.OutputStream? os)
        {
            _os = os;
            return null; // Response is cached in the Incoming to not have to create unecessary Task
        }

        public Task<Ice.OutputStream>? setMarshaledResult<T>(T result) where T : struct, Ice.MarshaledResult
        {
            Debug.Assert(_current != null);
            _os = result.getOutputStream(_current);
            return null; // Response is cached in the Incoming to not have to create unecessary Task
        }

        public Task<Ice.OutputStream?>? setResultTask<R>(Task<R>? task, Action<Ice.OutputStream, R> write)
        {
            if (task == null)
            {
                //
                // Write default constructed response if no task is provided
                //
                var os = startWriteParams();
                write(os, default(R));
                endWriteParams(os);
                return setResult(os);
            }
            else
            {
                var cached = getAndClearCachedOutputStream(); // If an output stream is cached, re-use it

                //
                // NOTE: it's important that the continuation doesn't mutate the Incoming state to
                // guarantee thread-safety. Multiple continuations can execute concurrently if the
                // user installed a dispatch interceptor and the dispatch is retried.
                //
                return task.ContinueWith((Task<R> t) =>
                {
                    var result = t.GetAwaiter().GetResult();
                    var os = startWriteParams(cached);
                    write(os, result);
                    endWriteParams(os);
                    return Task.FromResult<Ice.OutputStream?>(os);
                },
                CancellationToken.None,
                TaskContinuationOptions.ExecuteSynchronously,
                TaskScheduler.Current).Unwrap();
            }
        }

        public Task<Ice.OutputStream?>? setResultTask(Task task)
        {
            if (task == null)
            {
                //
                // Write response if no task is provided
                //
                return setResult(writeEmptyParams());
            }
            else
            {
                var cached = getAndClearCachedOutputStream(); // If an output stream is cached, re-use it

                //
                // NOTE: it's important that the continuation doesn't mutate the Incoming state to
                // guarantee thread-safety. Multiple continuations can execute concurrently if the
                // user installed a dispatch interceptor and the dispatch is retried.
                //
                return task.ContinueWith((Task t) =>
                    {
                        t.GetAwaiter().GetResult();
                        return Task.FromResult(writeEmptyParams(cached));
                    },
                    CancellationToken.None,
                    TaskContinuationOptions.ExecuteSynchronously,
                    TaskScheduler.Current).Unwrap();
            }
        }

        public Task<Ice.OutputStream?>? setMarshaledResultTask<T>(Task<T> task) where T : struct, Ice.MarshaledResult
        {
            Debug.Assert(_current != null);
            if (task == null)
            {
                return setResult(default(T).getOutputStream(_current));
            }
            else
            {
                //
                // NOTE: it's important that the continuation doesn't mutate the Incoming state to
                // guarantee thread-safety. Multiple continuations can execute concurrently if the
                // user installed a dispatch interceptor and the dispatch is retried.
                //
                return task.ContinueWith((Task<T> t) =>
                    {
                        return Task.FromResult<Ice.OutputStream?>(t.GetAwaiter().GetResult().getOutputStream(_current));
                    },
                    CancellationToken.None,
                    TaskContinuationOptions.ExecuteSynchronously,
                    TaskScheduler.Current).Unwrap();
            }
        }

        private void completed(Exception? exc, bool amd)
        {
            Debug.Assert(_responseHandler != null, "null response handler");
            Debug.Assert(_current != null, "null current");
            try
            {
                if (_locator != null)
                {
                    Debug.Assert(_servant != null, "null servant");
                    try
                    {
                        _locator.finished(_current, _servant, _cookie);
                    }
                    catch (Exception ex)
                    {
                        handleException(ex, amd);
                        return;
                    }
                }

                if (exc != null)
                {
                    handleException(exc, amd);
                }
                else if (_response)
                {
                    Debug.Assert(_os != null, "null output stream");
                    if (_observer != null)
                    {
                        _observer.reply(_os.size() - Protocol.headerSize - 4);
                    }
                    _responseHandler.sendResponse(_current.RequestId, _os, _compress, amd);
                }
                else
                {
                    _responseHandler.sendNoResponse();
                }
            }
            catch (Ice.LocalException ex)
            {
                _responseHandler.invokeException(_current.RequestId, ex, 1, amd);
            }
            finally
            {
                if (_observer != null)
                {
                    _observer.detach();
                    _observer = null;
                }
                _responseHandler = null;
            }
        }

        public void startOver()
        {
            Debug.Assert(_is != null);
            if (_inParamPos == -1)
            {
                //
                // That's the first startOver, so almost nothing to do
                //
                _inParamPos = _is.pos();
            }
            else
            {
                //
                // Let's rewind _is, reset _os
                //
                _is.pos(_inParamPos);
                if (_response && _os != null)
                {
                    _os.Reset();
                }
            }
        }

        public void skipReadParams()
        {
            Debug.Assert(_is != null);
            Debug.Assert(_current != null);
            //
            // Remember the encoding used by the input parameters, we'll
            // encode the response parameters with the same encoding.
            //
            _current.Encoding = _is.SkipEncapsulation();
        }

        public Ice.InputStream startReadParams()
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

        public void endReadParams()
        {
            Debug.Assert(_is != null);
            _is.EndEncapsulation();
        }

        public void readEmptyParams()
        {
            Debug.Assert(_is != null);
            Debug.Assert(_current != null);
            _current.Encoding = _is.SkipEmptyEncapsulation();
        }

        public byte[] readParamEncaps()
        {
            Debug.Assert(_is != null);
            Debug.Assert(_current != null);
            Ice.EncodingVersion encoding;
            byte[] result = _is.ReadEncapsulation(out encoding);
            _current.Encoding = encoding;
            return result;
        }

        public void setFormat(Ice.FormatType format)
        {
            _format = format;
        }

        public Ice.OutputStream? getAndClearCachedOutputStream()
        {
            if (_response)
            {
                var cached = _os;
                _os = null;
                return cached;
            }
            else
            {
                return null; // Don't consume unnecessarily the output stream if we are dispatching a oneway request
            }
        }

        public static Ice.OutputStream createResponseOutputStream(Ice.Current current)
        {
            var os = new Ice.OutputStream(current.Adapter!.Communicator, Ice.Util.currentProtocolEncoding);
            os.WriteBlob(Protocol.replyHdr);
            os.WriteInt(current.RequestId);
            os.WriteByte(ReplyStatus.replyOK);
            return os;
        }

        private Ice.OutputStream startWriteParams(Ice.OutputStream? os)
        {
            if (!_response)
            {
                throw new Ice.MarshalException("can't marshal out parameters for oneway dispatch");
            }

            if (os == null) // Create the output stream if none is provided
            {
                os = new Ice.OutputStream(_communicator, Ice.Util.currentProtocolEncoding);
            }
            Debug.Assert(_current != null);
            Debug.Assert(os.pos() == 0);
            os.WriteBlob(Protocol.replyHdr);
            os.WriteInt(_current.RequestId);
            os.WriteByte(ReplyStatus.replyOK);
            os.StartEncapsulation(_current.Encoding, _format);
            return os;
        }

        public Ice.OutputStream startWriteParams()
        {
            return startWriteParams(getAndClearCachedOutputStream());
        }

        public void endWriteParams(Ice.OutputStream os)
        {
            if (_response)
            {
                os.EndEncapsulation();
            }
        }

        private Ice.OutputStream? writeEmptyParams(Ice.OutputStream? os)
        {
            if (_response)
            {
                if (os == null) // Create the output stream if none is provided
                {
                    os = new Ice.OutputStream(_communicator, Ice.Util.currentProtocolEncoding);
                }
                Debug.Assert(_current != null);
                Debug.Assert(os.pos() == 0);
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

        public Ice.OutputStream? writeEmptyParams()
        {
            return writeEmptyParams(getAndClearCachedOutputStream());
        }

        public Ice.OutputStream? writeParamEncaps(Ice.OutputStream os, byte[] v, bool ok)
        {
            if (!ok && _observer != null)
            {
                _observer.userException();
            }

            if (_response)
            {
                if (os == null) // Create the output stream if none is provided
                {
                    os = new Ice.OutputStream(_communicator, Ice.Util.currentProtocolEncoding);
                }
                Debug.Assert(_current != null);
                Debug.Assert(os.pos() == 0);
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

        private void warning(Exception ex)
        {
            Debug.Assert(_communicator != null);
            Debug.Assert(_current != null);

            using (StringWriter sw = new StringWriter(CultureInfo.CurrentCulture))
            {
                IceUtilInternal.OutputBase output = new IceUtilInternal.OutputBase(sw);
                output.setUseTab(false);
                output.print("dispatch exception:");
                output.print("\nidentity: " + _current.Id.ToString(_communicator.ToStringMode));
                output.print("\nfacet: " + IceUtilInternal.StringUtil.escapeString(_current.Facet, "", _communicator.ToStringMode));
                output.print("\noperation: " + _current.Operation);
                if (_current.Connection != null)
                {
                    try
                    {
                        for (Ice.ConnectionInfo? p = _current.Connection.getInfo(); p != null; p = p.underlying)
                        {
                            if (p is Ice.IPConnectionInfo)
                            {
                                Ice.IPConnectionInfo ipinfo = (Ice.IPConnectionInfo)p;
                                output.print($"\nremote host: {ipinfo.remoteAddress} remote port: {ipinfo.remotePort}");
                                break;
                            }
                        }
                    }
                    catch (Ice.LocalException)
                    {
                    }
                }
                output.print("\n");
                output.print(ex.ToString());
                _communicator.Logger.warning(sw.ToString());
            }
        }

        private void handleException(Exception exc, bool amd)
        {
            Debug.Assert(_current != null);
            Debug.Assert(_responseHandler != null);

            if (exc is Ice.SystemException)
            {
                if (_responseHandler.systemException(_requestId, (Ice.SystemException)exc, amd))
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
                    _os = new Ice.OutputStream(_communicator, Ice.Util.currentProtocolEncoding);
                }
            }

            try
            {
                throw exc;
            }
            catch (Ice.RequestFailedException ex)
            {
                if (ex.id.name == null || ex.id.name.Length == 0)
                {
                    ex.id = _current.Id;
                }

                if (ex.facet == null || ex.facet.Length == 0)
                {
                    ex.facet = _current.Facet;
                }

                if (ex.operation == null || ex.operation.Length == 0)
                {
                    ex.operation = _current.Operation;
                }

                if (_communicator.GetPropertyAsInt("Ice.Warn.Dispatch") > 1)
                {
                    warning(ex);
                }

                if (_observer != null)
                {
                    _observer.failed(ex.ice_id());
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
                    ex.id.ice_writeMembers(_os);

                    //
                    // For compatibility with the old FacetPath.
                    //
                    if (ex.facet == null || ex.facet.Length == 0)
                    {
                        _os.WriteStringSeq(null);
                    }
                    else
                    {
                        string[] facetPath2 = { ex.facet };
                        _os.WriteStringSeq(facetPath2);
                    }

                    _os.WriteString(ex.operation);

                    if (_observer != null)
                    {
                        _observer.reply(_os.size() - Protocol.headerSize - 4);
                    }
                    _responseHandler.sendResponse(_current.RequestId, _os, _compress, amd);
                }
                else
                {
                    _responseHandler.sendNoResponse();
                }
            }
            catch (Ice.UnknownLocalException ex)
            {
                if ((_communicator.GetPropertyAsInt("Ice.Warn.Dispatch") ?? 1) > 0)
                {
                    warning(ex);
                }

                if (_observer != null)
                {
                    _observer.failed(ex.ice_id());
                }

                if (_response)
                {
                    Debug.Assert(_os != null);
                    _os.WriteBlob(Protocol.replyHdr);
                    _os.WriteInt(_current.RequestId);
                    _os.WriteByte(ReplyStatus.replyUnknownLocalException);
                    _os.WriteString(ex.unknown);
                    if (_observer != null)
                    {
                        _observer.reply(_os.size() - Protocol.headerSize - 4);
                    }
                    _responseHandler.sendResponse(_current.RequestId, _os, _compress, amd);
                }
                else
                {
                    _responseHandler.sendNoResponse();
                }
            }
            catch (Ice.UnknownUserException ex)
            {
                if ((_communicator.GetPropertyAsInt("Ice.Warn.Dispatch") ?? 1) > 0)
                {
                    warning(ex);
                }

                if (_observer != null)
                {
                    _observer.failed(ex.ice_id());
                }

                if (_response)
                {
                    Debug.Assert(_os != null);
                    _os.WriteBlob(Protocol.replyHdr);
                    _os.WriteInt(_current.RequestId);
                    _os.WriteByte(ReplyStatus.replyUnknownUserException);
                    _os.WriteString(ex.unknown);
                    if (_observer != null)
                    {
                        _observer.reply(_os.size() - Protocol.headerSize - 4);
                    }
                    Debug.Assert(_responseHandler != null && _current != null);
                    _responseHandler.sendResponse(_current.RequestId, _os, _compress, amd);
                }
                else
                {
                    _responseHandler.sendNoResponse();
                }
            }
            catch (Ice.UnknownException ex)
            {
                if ((_communicator.GetPropertyAsInt("Ice.Warn.Dispatch") ?? 1) > 0)
                {
                    warning(ex);
                }

                if (_observer != null)
                {
                    _observer.failed(ex.ice_id());
                }

                if (_response)
                {
                    Debug.Assert(_os != null);
                    _os.WriteBlob(Protocol.replyHdr);
                    _os.WriteInt(_current.RequestId);
                    _os.WriteByte(ReplyStatus.replyUnknownException);
                    _os.WriteString(ex.unknown);
                    if (_observer != null)
                    {
                        _observer.reply(_os.size() - Protocol.headerSize - 4);
                    }
                    _responseHandler.sendResponse(_current.RequestId, _os, _compress, amd);
                }
                else
                {
                    _responseHandler.sendNoResponse();
                }
            }
            catch (Ice.UserException ex)
            {
                if (_observer != null)
                {
                    _observer.userException();
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
                        _observer.reply(_os.size() - Protocol.headerSize - 4);
                    }
                    _responseHandler.sendResponse(_current.RequestId, _os, _compress, false);
                }
                else
                {
                    _responseHandler.sendNoResponse();
                }
            }
            catch (Ice.Exception ex)
            {
                if ((_communicator.GetPropertyAsInt("Ice.Warn.Dispatch") ?? 1) > 0)
                {
                    warning(ex);
                }

                if (_observer != null)
                {
                    _observer.failed(ex.ice_id());
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
                        _observer.reply(_os.size() - Protocol.headerSize - 4);
                    }
                    _responseHandler.sendResponse(_current.RequestId, _os, _compress, amd);
                }
                else
                {
                    _responseHandler.sendNoResponse();
                }
            }
            catch (Exception ex)
            {
                if ((_communicator.GetPropertyAsInt("Ice.Warn.Dispatch") ?? 1) > 0)
                {
                    warning(ex);
                }

                if (_observer != null)
                {
                    _observer.failed(ex.GetType().FullName);
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
                        _observer.reply(_os.size() - Protocol.headerSize - 4);
                    }
                    _responseHandler.sendResponse(_current.RequestId, _os, _compress, amd);
                }
                else
                {
                    _responseHandler.sendNoResponse();
                }
            }

            if (_observer != null)
            {
                _observer.detach();
                _observer = null;
            }
            _responseHandler = null;
        }

        private Ice.Communicator _communicator;
        private Ice.Current? _current;
        private Ice.Disp? _servant;
        private Ice.ServantLocator? _locator;
        private object? _cookie;
        private Ice.Instrumentation.DispatchObserver? _observer;
        private ResponseHandler? _responseHandler;

        private bool _response;
        private byte _compress;
        private Ice.ObjectAdapter _adapter;
        private Ice.Connection? _connection;
        private int _requestId;
        private Ice.FormatType _format = Ice.FormatType.DefaultFormat;

        private Ice.OutputStream? _os;
        private Ice.InputStream? _is;

        private int _inParamPos = -1;

        public Incoming? next; // For use by Connection.
    }
}
