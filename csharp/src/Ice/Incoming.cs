// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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
    using System.IO;
    using System.Globalization;
    using System.Threading.Tasks;

    public class Incoming : Ice.Request
    {
        public Incoming(Instance instance, ResponseHandler handler, Ice.ConnectionI connection,
                        Ice.ObjectAdapter adapter, bool response, byte compress, int requestId)
        {
            _instance = instance;
            _responseHandler = handler;
            _response = response;
            _compress = compress;

            _current = new Ice.Current();
            _current.id = new Ice.Identity();
            _current.adapter = adapter;
            _current.con = connection;
            _current.requestId = requestId;

            _cookie = null;
        }

        //
        // These functions allow this object to be reused, rather than reallocated.
        //
        public void reset(Instance instance, ResponseHandler handler, Ice.ConnectionI connection,
                          Ice.ObjectAdapter adapter, bool response, byte compress, int requestId)
        {
            _instance = instance;
            _responseHandler = handler;
            _response = response;
            _compress = compress;

            //
            // Don't recycle the Current object, because servants may keep a reference to it.
            //
            _current = new Ice.Current();
            _current.id = new Ice.Identity();
            _current.adapter = adapter;
            _current.con = connection;
            _current.requestId = requestId;

            Debug.Assert(_cookie == null);

            _inParamPos = -1;
        }

        public bool reclaim()
        {
            if(_responseHandler != null) // Async dispatch not ready for being reclaimed!
            {
                return false;
            }

            _current = null;
            _servant = null;
            _locator = null;
            _cookie = null;

            //_observer = null;
            Debug.Assert(_observer == null);

            _os = null;
            _is = null;

            //_responseHandler = null;
            Debug.Assert(_responseHandler == null);

            _inParamPos = -1;

            return true;
        }

        public Ice.Current getCurrent()
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
            _current.id.ice_readMembers(_is);

            //
            // For compatibility with the old FacetPath.
            //
            string[] facetPath = _is.readStringSeq();
            if(facetPath.Length > 0)
            {
                if(facetPath.Length > 1)
                {
                    throw new Ice.MarshalException();
                }
                _current.facet = facetPath[0];
            }
            else
            {
                _current.facet = "";
            }

            _current.operation = _is.readString();
            _current.mode = (Ice.OperationMode)_is.readByte();
            _current.ctx = new Dictionary<string, string>();
            int sz = _is.readSize();
            while(sz-- > 0)
            {
                string first = _is.readString();
                string second = _is.readString();
                _current.ctx[first] = second;
            }

            Ice.Instrumentation.CommunicatorObserver obsv = _instance.initializationData().observer;
            if(obsv != null)
            {
                // Read the encapsulation size.
                int size = _is.readInt();
                _is.pos(_is.pos() - 4);

                _observer = obsv.getDispatchObserver(_current, _is.pos() - start + size);
                if(_observer != null)
                {
                    _observer.attach();
                }
            }

            //
            // Don't put the code above into the try block below. Exceptions
            // in the code above are considered fatal, and must propagate to
            // the caller of this operation.
            //

            if(servantManager != null)
            {
                _servant = servantManager.findServant(_current.id, _current.facet);
                if(_servant == null)
                {
                    _locator = servantManager.findServantLocator(_current.id.category);
                    if(_locator == null && _current.id.category.Length > 0)
                    {
                        _locator = servantManager.findServantLocator("");
                    }

                    if(_locator != null)
                    {
                        Debug.Assert(_locator != null);
                        try
                        {
                            _servant = _locator.locate(_current, out _cookie);
                        }
                        catch(Exception ex)
                        {
                            skipReadParams(); // Required for batch requests.
                            handleException(ex, false);
                            return;
                        }
                    }
                }
            }

            if(_servant == null)
            {
                try
                {
                    if(servantManager != null && servantManager.hasServant(_current.id))
                    {
                        throw new Ice.FacetNotExistException(_current.id, _current.facet, _current.operation);
                    }
                    else
                    {
                        throw new Ice.ObjectNotExistException(_current.id, _current.facet, _current.operation);
                    }
                }
                catch(Exception ex)
                {
                    skipReadParams(); // Required for batch requests
                    handleException(ex, false);
                    return;
                }
            }

            try
            {
                Task<Ice.OutputStream> task = _servant.iceDispatch(this, _current);
                if(task == null)
                {
                    completed(null, false);
                }
                else
                {
                    if(task.IsCompleted)
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
                            catch(Exception ex)
                            {
                                completed(ex, true); // true = asynchronous
                            }
                        }, TaskContinuationOptions.ExecuteSynchronously);
                    }
                }
            }
            catch(Exception ex)
            {
                completed(ex, false);
            }
        }

        public Task<Ice.OutputStream> setResult(Ice.OutputStream os)
        {
            _os = os;
            return null; // Response is cached in the Incoming to not have to create unecessary Task
        }

        public Task<Ice.OutputStream> setMarshaledResult<T>(T result) where T : Ice.MarshaledResult
        {
            if(result == null)
            {
                _os = default(T).getOutputStream(_current);
            }
            else
            {
                _os = result.getOutputStream(_current);
            }
            return null; // Response is cached in the Incoming to not have to create unecessary Task
        }

        public Task<Ice.OutputStream> setResultTask<R>(Task<R> task, Action<Ice.OutputStream, R> write)
        {
            if(task == null)
            {
                _os = startWriteParams();
                write(_os, default(R));
                endWriteParams(_os);
                return null; // Response is cached in the Incoming to not have to create unecessary Task
            }
            else
            {
                return task.ContinueWith((Task<R> t) =>
                {
                    var result = t.GetAwaiter().GetResult();
                    var os = startWriteParams();
                    write(os, result);
                    endWriteParams(os);
                    return Task.FromResult(os);
                }, TaskContinuationOptions.ExecuteSynchronously).Unwrap();
            }
        }

        public Task<Ice.OutputStream> setResultTask(Task task)
        {
            if(task == null)
            {
                _os = writeEmptyParams();
                return null;
            }
            else
            {
                return task.ContinueWith((Task t) =>
                {
                    t.GetAwaiter().GetResult();
                    return Task.FromResult(writeEmptyParams());
                }, TaskContinuationOptions.ExecuteSynchronously).Unwrap();
            }
        }

        public Task<Ice.OutputStream> setMarshaledResultTask<T>(Task<T> task) where T : Ice.MarshaledResult
        {
            if(task == null)
            {
                _os = default(T).getOutputStream(_current);
                return null; // Response is cached in the Incoming to not have to create unecessary Task
            }
            else
            {
                return task.ContinueWith((Task<T> t) =>
                {
                    return Task.FromResult(t.GetAwaiter().GetResult().getOutputStream(_current));
                }, TaskContinuationOptions.ExecuteSynchronously).Unwrap();
            }
        }

        public void completed(Exception exc, bool amd)
        {
            try
            {
                if(_locator != null)
                {
                    Debug.Assert(_locator != null && _servant != null);
                    try
                    {
                        _locator.finished(_current, _servant, _cookie);
                    }
                    catch(Exception ex)
                    {
                        handleException(ex, amd);
                        return;
                    }
                }

                Debug.Assert(_responseHandler != null);

                if(exc != null)
                {
                    handleException(exc, amd);
                }
                else if(_response)
                {
                    if(_observer != null)
                    {
                        _observer.reply(_os.size() - Protocol.headerSize - 4);
                    }
                    _responseHandler.sendResponse(_current.requestId, _os, _compress, amd);
                }
                else
                {
                    _responseHandler.sendNoResponse();
                }
            }
            catch(Ice.LocalException ex)
            {
                _responseHandler.invokeException(_current.requestId, ex, 1, amd);
            }
            finally
            {
                if(_observer != null)
                {
                    _observer.detach();
                    _observer = null;
                }
                _responseHandler = null;
            }
        }

        public void startOver()
        {
            if(_inParamPos == -1)
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
                _os = null;
            }
        }

        public void skipReadParams()
        {
            //
            // Remember the encoding used by the input parameters, we'll
            // encode the response parameters with the same encoding.
            //
            _current.encoding = _is.skipEncapsulation();
        }

        public Ice.InputStream startReadParams()
        {
            //
            // Remember the encoding used by the input parameters, we'll
            // encode the response parameters with the same encoding.
            //
            _current.encoding = _is.startEncapsulation();
            return _is;
        }

        public void endReadParams()
        {
            _is.endEncapsulation();
        }

        public void readEmptyParams()
        {
            _current.encoding = _is.skipEmptyEncapsulation();
        }

        public byte[] readParamEncaps()
        {
            return _is.readEncapsulation(out _current.encoding);
        }

        public void setFormat(Ice.FormatType format)
        {
            _format = format;
        }

        static public Ice.OutputStream createResponseOutputStream(Ice.Current current)
        {
            var os = new Ice.OutputStream(current.adapter.getCommunicator(), Ice.Util.currentProtocolEncoding);
            os.writeBlob(Protocol.replyHdr);
            os.writeInt(current.requestId);
            os.writeByte(ReplyStatus.replyOK);
            return os;
        }

        public Ice.OutputStream startWriteParams()
        {
            if(!_response)
            {
                throw new Ice.MarshalException("can't marshal out parameters for oneway dispatch");
            }

            var os = new Ice.OutputStream(_instance, Ice.Util.currentProtocolEncoding);
            os.writeBlob(Protocol.replyHdr);
            os.writeInt(_current.requestId);
            os.writeByte(ReplyStatus.replyOK);
            os.startEncapsulation(_current.encoding, _format);
            return os;
        }

        public void endWriteParams(Ice.OutputStream os)
        {
            if(_response)
            {
                os.endEncapsulation();
            }
        }

        public Ice.OutputStream writeEmptyParams()
        {
            if(_response)
            {
                var os = new Ice.OutputStream(_instance, Ice.Util.currentProtocolEncoding);
                os.writeBlob(Protocol.replyHdr);
                os.writeInt(_current.requestId);
                os.writeByte(ReplyStatus.replyOK);
                os.writeEmptyEncapsulation(_current.encoding);
                return os;
            }
            else
            {
                return null;
            }
        }

        public Ice.OutputStream writeParamEncaps(byte[] v, bool ok)
        {
            if(!ok && _observer != null)
            {
                _observer.userException();
            }

            if(_response)
            {
                var os = new Ice.OutputStream(_instance, Ice.Util.currentProtocolEncoding);
                os.writeBlob(Protocol.replyHdr);
                os.writeInt(_current.requestId);
                os.writeByte(ok ? ReplyStatus.replyOK : ReplyStatus.replyUserException);
                if(v == null || v.Length == 0)
                {
                    os.writeEmptyEncapsulation(_current.encoding);
                }
                else
                {
                    os.writeEncapsulation(v);
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
            Debug.Assert(_instance != null);

            using(StringWriter sw = new StringWriter(CultureInfo.CurrentCulture))
            {
                IceUtilInternal.OutputBase output = new IceUtilInternal.OutputBase(sw);
                Ice.ToStringMode toStringMode = _instance.toStringMode();
                output.setUseTab(false);
                output.print("dispatch exception:");
                output.print("\nidentity: " + Ice.Util.identityToString(_current.id, toStringMode));
                output.print("\nfacet: " + IceUtilInternal.StringUtil.escapeString(_current.facet, "", toStringMode));
                output.print("\noperation: " + _current.operation);
                if(_current.con != null)
                {
                    try
                    {
                        for(Ice.ConnectionInfo p = _current.con.getInfo(); p != null; p = p.underlying)
                        {
                            if(p is Ice.IPConnectionInfo)
                            {
                                Ice.IPConnectionInfo ipinfo = p as Ice.IPConnectionInfo;
                                output.print("\nremote host: " + ipinfo.remoteAddress + " remote port: " + ipinfo.remotePort);
                                break;
                            }
                        }
                    }
                    catch(Ice.LocalException)
                    {
                    }
                }
                output.print("\n");
                output.print(ex.ToString());
                _instance.initializationData().logger.warning(sw.ToString());
            }
        }

        private void handleException(Exception exc, bool amd)
        {
            Debug.Assert(_responseHandler != null);

            if(exc is Ice.SystemException)
            {
                if(_responseHandler.systemException(_current.requestId, (Ice.SystemException)exc, amd))
                {
                    return;
                }
            }

            try
            {
                throw exc;
            }
            catch(Ice.RequestFailedException ex)
            {
                if(ex.id == null || ex.id.name == null || ex.id.name.Length == 0)
                {
                    ex.id = _current.id;
                }

                if(ex.facet == null || ex.facet.Length == 0)
                {
                    ex.facet = _current.facet;
                }

                if(ex.operation == null || ex.operation.Length == 0)
                {
                    ex.operation = _current.operation;
                }

                if(_instance.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 1)
                {
                    warning(ex);
                }

                if(_observer != null)
                {
                    _observer.failed(ex.ice_id());
                }

                if(_response)
                {
                    _os = new Ice.OutputStream(_instance, Ice.Util.currentProtocolEncoding);
                    _os.writeBlob(Protocol.replyHdr);
                    _os.writeInt(_current.requestId);
                    if(ex is Ice.ObjectNotExistException)
                    {
                        _os.writeByte(ReplyStatus.replyObjectNotExist);
                    }
                    else if(ex is Ice.FacetNotExistException)
                    {
                        _os.writeByte(ReplyStatus.replyFacetNotExist);
                    }
                    else if(ex is Ice.OperationNotExistException)
                    {
                        _os.writeByte(ReplyStatus.replyOperationNotExist);
                    }
                    else
                    {
                        Debug.Assert(false);
                    }
                    ex.id.ice_writeMembers(_os);

                    //
                    // For compatibility with the old FacetPath.
                    //
                    if(ex.facet == null || ex.facet.Length == 0)
                    {
                        _os.writeStringSeq(null);
                    }
                    else
                    {
                        string[] facetPath2 = { ex.facet };
                        _os.writeStringSeq(facetPath2);
                    }

                    _os.writeString(ex.operation);

                    if(_observer != null)
                    {
                        _observer.reply(_os.size() - Protocol.headerSize - 4);
                    }
                    _responseHandler.sendResponse(_current.requestId, _os, _compress, amd);
                }
                else
                {
                    _responseHandler.sendNoResponse();
                }
            }
            catch(Ice.UnknownLocalException ex)
            {
                if(_instance.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
                {
                    warning(ex);
                }

                if(_observer != null)
                {
                    _observer.failed(ex.ice_id());
                }

                if(_response)
                {
                    _os = new Ice.OutputStream(_instance, Ice.Util.currentProtocolEncoding);
                    _os.writeBlob(Protocol.replyHdr);
                    _os.writeInt(_current.requestId);
                    _os.writeByte(ReplyStatus.replyUnknownLocalException);
                    _os.writeString(ex.unknown);
                    if(_observer != null)
                    {
                        _observer.reply(_os.size() - Protocol.headerSize - 4);
                    }
                    _responseHandler.sendResponse(_current.requestId, _os, _compress, amd);
                }
                else
                {
                    _responseHandler.sendNoResponse();
                }
            }
            catch(Ice.UnknownUserException ex)
            {
                if(_instance.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
                {
                    warning(ex);
                }

                if(_observer != null)
                {
                    _observer.failed(ex.ice_id());
                }

                if(_response)
                {
                    _os = new Ice.OutputStream(_instance, Ice.Util.currentProtocolEncoding);
                    _os.writeBlob(Protocol.replyHdr);
                    _os.writeInt(_current.requestId);
                    _os.writeByte(ReplyStatus.replyUnknownUserException);
                    _os.writeString(ex.unknown);
                    if(_observer != null)
                    {
                        _observer.reply(_os.size() - Protocol.headerSize - 4);
                    }
                    Debug.Assert(_responseHandler != null && _current != null);
                    _responseHandler.sendResponse(_current.requestId, _os, _compress, amd);
                }
                else
                {
                    _responseHandler.sendNoResponse();
                }
            }
            catch(Ice.UnknownException ex)
            {
                if(_instance.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
                {
                    warning(ex);
                }

                if(_observer != null)
                {
                    _observer.failed(ex.ice_id());
                }

                if(_response)
                {
                    _os = new Ice.OutputStream(_instance, Ice.Util.currentProtocolEncoding);
                    _os.writeBlob(Protocol.replyHdr);
                    _os.writeInt(_current.requestId);
                    _os.writeByte(ReplyStatus.replyUnknownException);
                    _os.writeString(ex.unknown);
                    if(_observer != null)
                    {
                        _observer.reply(_os.size() - Protocol.headerSize - 4);
                    }
                    _responseHandler.sendResponse(_current.requestId, _os, _compress, amd);
                }
                else
                {
                    _responseHandler.sendNoResponse();
                }
            }
            catch(Ice.UserException ex)
            {
                if(_observer != null)
                {
                    _observer.userException();
                }

                if(_response)
                {
                    _os = new Ice.OutputStream(_instance, Ice.Util.currentProtocolEncoding);
                    _os.writeBlob(Protocol.replyHdr);
                    _os.writeInt(_current.requestId);
                    _os.writeByte(ReplyStatus.replyUserException);
                    _os.startEncapsulation(_current.encoding, _format);
                    _os.writeException(ex);
                    _os.endEncapsulation();
                    if(_observer != null)
                    {
                        _observer.reply(_os.size() - Protocol.headerSize - 4);
                    }
                    _responseHandler.sendResponse(_current.requestId, _os, _compress, false);
                }
                else
                {
                    _responseHandler.sendNoResponse();
                }
            }
            catch(Ice.Exception ex)
            {
                if(_instance.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
                {
                    warning(ex);
                }

                if(_observer != null)
                {
                    _observer.failed(ex.ice_id());
                }

                if(_response)
                {
                    _os = new Ice.OutputStream(_instance, Ice.Util.currentProtocolEncoding);
                    _os.writeBlob(Protocol.replyHdr);
                    _os.writeInt(_current.requestId);
                    _os.writeByte(ReplyStatus.replyUnknownLocalException);
                    _os.writeString(ex.ice_id() + "\n" + ex.StackTrace);
                    if(_observer != null)
                    {
                        _observer.reply(_os.size() - Protocol.headerSize - 4);
                    }
                    _responseHandler.sendResponse(_current.requestId, _os, _compress, amd);
                }
                else
                {
                    _responseHandler.sendNoResponse();
                }
            }
            catch(Exception ex)
            {
                if(_instance.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
                {
                    warning(ex);
                }

                if(_observer != null)
                {
                    _observer.failed(ex.GetType().FullName);
                }

                if(_response)
                {
                    _os = new Ice.OutputStream(_instance, Ice.Util.currentProtocolEncoding);
                    _os.writeBlob(Protocol.replyHdr);
                    _os.writeInt(_current.requestId);
                    _os.writeByte(ReplyStatus.replyUnknownException);
                    _os.writeString(ex.ToString());
                    if(_observer != null)
                    {
                        _observer.reply(_os.size() - Protocol.headerSize - 4);
                    }
                    _responseHandler.sendResponse(_current.requestId, _os, _compress, amd);
                }
                else
                {
                    _responseHandler.sendNoResponse();
                }
            }

            if(_observer != null)
            {
                _observer.detach();
                _observer = null;
            }
            _responseHandler = null;
        }

        private Instance _instance;
        private Ice.Current _current;
        private Ice.Object _servant;
        private Ice.ServantLocator _locator;
        private object _cookie;
        private Ice.Instrumentation.DispatchObserver _observer;
        private ResponseHandler _responseHandler;

        private bool _response;
        private byte _compress;
        private Ice.FormatType _format = Ice.FormatType.DefaultFormat;

        private Ice.OutputStream _os;
        private Ice.InputStream _is;

        private int _inParamPos = -1;

        public Incoming next; // For use by Connection.
    }
}
