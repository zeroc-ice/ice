//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceInternal;

import java.util.concurrent.CompletionStage;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutionException;

import com.zeroc.Ice.ConnectionInfo;
import com.zeroc.Ice.Current;
import com.zeroc.Ice.FormatType;
import com.zeroc.Ice.Instrumentation.CommunicatorObserver;
import com.zeroc.Ice.Instrumentation.DispatchObserver;
import com.zeroc.Ice.InputStream;
import com.zeroc.Ice.IPConnectionInfo;
import com.zeroc.Ice.MarshaledResult;
import com.zeroc.Ice.OutputStream;
import com.zeroc.Ice.ServantLocator;
import com.zeroc.Ice.Util;

final public class Incoming implements com.zeroc.Ice.Request
{
    public Incoming(Instance instance, ResponseHandler responseHandler, com.zeroc.Ice.ConnectionI connection,
                    com.zeroc.Ice.ObjectAdapter adapter, boolean response, byte compress, int requestId)
    {
        _instance = instance;
        _responseHandler = responseHandler;
        _response = response;
        _compress = compress;

        _current = new Current();
        _current.id = new com.zeroc.Ice.Identity();
        _current.adapter = adapter;
        _current.con = connection;
        _current.requestId = requestId;

        _cookie = null;
    }

    //
    // These functions allow this object to be reused, rather than reallocated.
    //
    public void reset(Instance instance, ResponseHandler handler, com.zeroc.Ice.ConnectionI connection,
                      com.zeroc.Ice.ObjectAdapter adapter, boolean response, byte compress, int requestId)
    {
        _instance = instance;
        _responseHandler = handler;
        _response = response;
        _compress = compress;

        //
        // Don't recycle the Current object, because servants may keep a reference to it.
        //
        _current = new Current();
        _current.id = new com.zeroc.Ice.Identity();
        _current.adapter = adapter;
        _current.con = connection;
        _current.requestId = requestId;

        assert(_cookie == null);

        _inParamPos = -1;
    }

    public boolean reclaim()
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
        assert(_observer == null);

        if(_os != null)
        {
            _os.reset();
        }

        _is = null;

        //_responseHandler = null;
        assert(_responseHandler == null);

        _inParamPos = -1;

        return true;
    }

    @Override
    public Current getCurrent()
    {
        return _current;
    }

    public void invoke(ServantManager servantManager, InputStream stream)
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
        String[] facetPath = _is.readStringSeq();
        if(facetPath.length > 0)
        {
            if(facetPath.length > 1)
            {
                throw new com.zeroc.Ice.MarshalException();
            }
            _current.facet = facetPath[0];
        }
        else
        {
            _current.facet = "";
        }

        _current.operation = _is.readString();
        _current.mode = com.zeroc.Ice.OperationMode.values()[_is.readByte()];
        _current.ctx = new java.util.HashMap<>();
        int sz = _is.readSize();
        while(sz-- > 0)
        {
            String first = _is.readString();
            String second = _is.readString();
            _current.ctx.put(first, second);
        }

        CommunicatorObserver obsv = _instance.initializationData().observer;
        if(obsv != null)
        {
            // Read the parameter encapsulation size.
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
                if(_locator == null && _current.id.category.length() > 0)
                {
                    _locator = servantManager.findServantLocator("");
                }

                if(_locator != null)
                {
                    try
                    {
                        ServantLocator.LocateResult r = _locator.locate(_current);
                        _servant = r.returnValue;
                        _cookie = r.cookie;
                    }
                    catch(Throwable ex)
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
                    throw new com.zeroc.Ice.FacetNotExistException(_current.id, _current.facet, _current.operation);
                }
                else
                {
                    throw new com.zeroc.Ice.ObjectNotExistException(_current.id, _current.facet, _current.operation);
                }
            }
            catch(Throwable ex)
            {
                skipReadParams(); // Required for batch requests
                handleException(ex, false);
                return;
            }
        }

        try
        {
            if(_instance.useApplicationClassLoader())
            {
                Thread.currentThread().setContextClassLoader(_servant.getClass().getClassLoader());
            }

            try
            {
                CompletionStage<OutputStream> f = _servant._iceDispatch(this, _current);
                if(f == null)
                {
                    completed(null, false);
                }
                else
                {
                    f.whenComplete((result, ex) ->
                        {
                            if(ex != null)
                            {
                                completed(ex, true); // true = asynchronous
                            }
                            else
                            {
                                _os = result;
                                completed(null, true); // true = asynchronous
                            }
                        });
                }
            }
            finally
            {
                if(_instance.useApplicationClassLoader())
                {
                    Thread.currentThread().setContextClassLoader(null);
                }
            }
        }
        catch(ServantError ex)
        {
            throw ex;
        }
        catch(Throwable ex)
        {
            completed(ex, false);
        }
    }

    public CompletionStage<OutputStream> setResult(OutputStream os)
    {
        _os = os;
        return null; // Response is cached in the Incoming to not have to create unnecessary future
    }

    @FunctionalInterface
    public static interface Write<T>
    {
        void write(OutputStream os, T v);
    }

    public <T> CompletionStage<OutputStream> setResultFuture(CompletionStage<T> f, Write<T> write)
    {
        final CompletableFuture<OutputStream> r = new CompletableFuture<OutputStream>();
        f.whenComplete((result, ex) ->
            {
                if(ex != null)
                {
                    r.completeExceptionally(ex);
                }
                else
                {
                    OutputStream os = startWriteParams();
                    write.write(os, result);
                    endWriteParams(os);
                    r.complete(os);
                }
            });
        return r;
    }

    public CompletionStage<OutputStream> setResultFuture(CompletionStage<Void> f)
    {
        final CompletableFuture<OutputStream> r = new CompletableFuture<OutputStream>();
        f.whenComplete((result, ex) ->
            {
                if(ex != null)
                {
                    r.completeExceptionally(ex);
                }
                else
                {
                    r.complete(writeEmptyParams());
                }
            });
        return r;
    }

    public CompletionStage<OutputStream> setMarshaledResult(MarshaledResult result)
    {
        _os = result.getOutputStream();
        return null; // Response is cached in the Incoming to not have to create unnecessary future
    }

    public <T extends MarshaledResult> CompletionStage<OutputStream>
    setMarshaledResultFuture(CompletionStage<T> f)
    {
        final CompletableFuture<OutputStream> r = new CompletableFuture<OutputStream>();
        f.whenComplete((result, ex) ->
            {
                if(ex != null)
                {
                    r.completeExceptionally(ex);
                }
                else
                {
                    r.complete(result.getOutputStream());
                }
            });
        return r;
    }

    public void completed(Throwable exc, boolean amd)
    {
        try
        {
            if(_locator != null)
            {
                assert(_locator != null && _servant != null);
                try
                {
                    _locator.finished(_current, _servant, _cookie);
                }
                catch(Throwable ex)
                {
                    handleException(ex, amd);
                    return;
                }
            }

            assert(_responseHandler != null);

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
        catch(com.zeroc.Ice.LocalException ex)
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

    public final void startOver()
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
            // Let's rewind _is and reset _os
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

    public InputStream startReadParams()
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
        _current.encoding = new com.zeroc.Ice.EncodingVersion();
        return _is.readEncapsulation(_current.encoding);
    }

    public void setFormat(FormatType format)
    {
        if(format == null)
        {
            format = FormatType.DefaultFormat;
        }
        _format = format;
    }

    static public OutputStream createResponseOutputStream(Current current)
    {
        OutputStream os = new OutputStream(current.adapter.getCommunicator(), Protocol.currentProtocolEncoding);
        os.writeBlob(Protocol.replyHdr);
        os.writeInt(current.requestId);
        os.writeByte(ReplyStatus.replyOK);
        return os;
    }

    public com.zeroc.Ice.OutputStream startWriteParams()
    {
        if(!_response)
        {
            throw new com.zeroc.Ice.MarshalException("can't marshal out parameters for oneway dispatch");
        }

        OutputStream os = new OutputStream(_instance, Protocol.currentProtocolEncoding);
        os.writeBlob(Protocol.replyHdr);
        os.writeInt(_current.requestId);
        os.writeByte(ReplyStatus.replyOK);
        os.startEncapsulation(_current.encoding, _format);
        return os;
    }

    public void endWriteParams(OutputStream os)
    {
        if(_response)
        {
            os.endEncapsulation();
        }
    }

    public OutputStream writeEmptyParams()
    {
        if(_response)
        {
            OutputStream os = new OutputStream(_instance, Protocol.currentProtocolEncoding);
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

    public OutputStream writeParamEncaps(byte[] v, boolean ok)
    {
        if(!ok && _observer != null)
        {
            _observer.userException();
        }

        if(_response)
        {
            OutputStream os = new OutputStream(_instance, Protocol.currentProtocolEncoding);
            os.writeBlob(Protocol.replyHdr);
            os.writeInt(_current.requestId);
            os.writeByte(ok ? ReplyStatus.replyOK : ReplyStatus.replyUserException);
            if(v == null || v.length == 0)
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

    private void warning(Throwable ex)
    {
        assert(_instance != null);

        java.io.StringWriter sw = new java.io.StringWriter();
        java.io.PrintWriter pw = new java.io.PrintWriter(sw);
        com.zeroc.IceUtilInternal.OutputBase out = new com.zeroc.IceUtilInternal.OutputBase(pw);
        out.setUseTab(false);
        out.print("dispatch exception:");
        out.print("\nidentity: " + Util.identityToString(_current.id, _instance.toStringMode()));
        out.print("\nfacet: " + com.zeroc.IceUtilInternal.StringUtil.escapeString(_current.facet, "", _instance.toStringMode()));
        out.print("\noperation: " + _current.operation);
        if(_current.con != null)
        {
            try
            {
                for(ConnectionInfo connInfo = _current.con.getInfo(); connInfo != null; connInfo = connInfo.underlying)
                {
                    if(connInfo instanceof IPConnectionInfo)
                    {
                        IPConnectionInfo ipConnInfo = (IPConnectionInfo)connInfo;
                        out.print("\nremote host: " + ipConnInfo.remoteAddress + " remote port: " + ipConnInfo.remotePort);
                    }
                }
            }
            catch(com.zeroc.Ice.LocalException exc)
            {
                // Ignore.
            }
        }
        out.print("\n");
        ex.printStackTrace(pw);
        pw.flush();
        _instance.initializationData().logger.warning(sw.toString());
    }

    private void handleException(Throwable exc, boolean amd)
    {
        assert(_responseHandler != null);

        try
        {
            throw exc;
        }
        catch(com.zeroc.Ice.RequestFailedException ex)
        {
            if(ex.id == null || ex.id.name == null || ex.id.name.isEmpty())
            {
                ex.id = _current.id;
            }

            if(ex.facet == null || ex.facet.isEmpty())
            {
                ex.facet = _current.facet;
            }

            if(ex.operation == null || ex.operation.length() == 0)
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
                assert(_responseHandler != null && _current != null);
                _os = new OutputStream(_instance, Protocol.currentProtocolEncoding);
                _os.writeBlob(Protocol.replyHdr);
                _os.writeInt(_current.requestId);
                if(ex instanceof com.zeroc.Ice.ObjectNotExistException)
                {
                    _os.writeByte(ReplyStatus.replyObjectNotExist);
                }
                else if(ex instanceof com.zeroc.Ice.FacetNotExistException)
                {
                    _os.writeByte(ReplyStatus.replyFacetNotExist);
                }
                else if(ex instanceof com.zeroc.Ice.OperationNotExistException)
                {
                    _os.writeByte(ReplyStatus.replyOperationNotExist);
                }
                else
                {
                    assert(false);
                }
                ex.id.ice_writeMembers(_os);

                //
                // For compatibility with the old FacetPath.
                //
                if(ex.facet == null || ex.facet.length() == 0)
                {
                    _os.writeStringSeq(null);
                }
                else
                {
                    String[] facetPath2 = { ex.facet };
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
        catch(com.zeroc.Ice.UnknownLocalException ex)
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
                assert(_responseHandler != null && _current != null);
                _os = new OutputStream(_instance, Protocol.currentProtocolEncoding);
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
        catch(com.zeroc.Ice.UnknownUserException ex)
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
                assert(_responseHandler != null && _current != null);
                _os = new OutputStream(_instance, Protocol.currentProtocolEncoding);
                _os.writeBlob(Protocol.replyHdr);
                _os.writeInt(_current.requestId);
                _os.writeByte(ReplyStatus.replyUnknownUserException);
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
        catch(com.zeroc.Ice.UnknownException ex)
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
                assert(_responseHandler != null && _current != null);
                _os = new OutputStream(_instance, Protocol.currentProtocolEncoding);
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
        catch(com.zeroc.Ice.UserException ex)
        {
            if(_observer != null)
            {
                _observer.userException();
            }

            if(_response)
            {
                assert(_responseHandler != null && _current != null);
                _os = new OutputStream(_instance, Protocol.currentProtocolEncoding);
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
        catch(com.zeroc.Ice.Exception ex)
        {
            if(ex instanceof com.zeroc.Ice.SystemException)
            {
                if(_responseHandler.systemException(_current.requestId, (com.zeroc.Ice.SystemException)ex, amd))
                {
                    return;
                }
            }

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
                assert(_responseHandler != null && _current != null);
                _os = new OutputStream(_instance, Protocol.currentProtocolEncoding);
                _os.writeBlob(Protocol.replyHdr);
                _os.writeInt(_current.requestId);
                _os.writeByte(ReplyStatus.replyUnknownLocalException);
                //_os.writeString(ex.toString());
                java.io.StringWriter sw = new java.io.StringWriter();
                sw.write(ex.ice_id() + "\n");
                java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                ex.printStackTrace(pw);
                pw.flush();
                _os.writeString(sw.toString());
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
        catch(ExecutionException ex)
        {
            //
            // Raised by CompletableFuture.get(). The inner exception caused the future to complete exceptionally.
            // Recurse with the inner exception.
            //
            handleException(ex.getCause(), amd);
            return;
        }
        catch(Throwable ex)
        {
            if(_instance.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
            {
                warning(ex);
            }

            if(_observer != null)
            {
                _observer.failed(ex.getClass().getName());
            }

            if(_response)
            {
                assert(_responseHandler != null && _current != null);
                _os = new OutputStream(_instance, Protocol.currentProtocolEncoding);
                _os.writeBlob(Protocol.replyHdr);
                _os.writeInt(_current.requestId);
                _os.writeByte(ReplyStatus.replyUnknownException);
                //_os.writeString(ex.toString());
                java.io.StringWriter sw = new java.io.StringWriter();
                java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                ex.printStackTrace(pw);
                pw.flush();
                _os.writeString(sw.toString());
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

            if(_observer != null)
            {
                _observer.detach();
                _observer = null;
            }
            _responseHandler = null;

            if(!amd && ex instanceof java.lang.Error)
            {
                throw new ServantError((java.lang.Error)ex);
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
    private Current _current;
    private com.zeroc.Ice.Object _servant;
    private ServantLocator _locator;
    private java.lang.Object _cookie;
    private DispatchObserver _observer;
    private ResponseHandler _responseHandler;

    private boolean _response;
    private byte _compress;
    private FormatType _format = FormatType.DefaultFormat;

    private OutputStream _os;
    private InputStream _is;

    private int _inParamPos = -1;

    public Incoming next; // For use by ConnectionI.
}
