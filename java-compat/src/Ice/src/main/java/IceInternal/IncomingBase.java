// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

class IncomingBase
{
    protected
    IncomingBase(Instance instance, ResponseHandler handler, Ice.ConnectionI connection, Ice.ObjectAdapter adapter,
                 boolean response, byte compress, int requestId)
    {
        _instance = instance;
        _responseHandler = handler;
        _response = response;
        _compress = compress;
        _format = Ice.FormatType.DefaultFormat;

        _current = new Ice.Current();
        _current.id = new Ice.Identity();
        _current.adapter = adapter;
        _current.con = connection;
        _current.requestId = requestId;
    }

    protected
    IncomingBase(IncomingBase other)
    {
        //
        // We don't change _current as it's exposed by Ice::Request
        //
        _current = other._current;

        _instance = other._instance;
        _servant = other._servant;
        _locator = other._locator;
        _response = other._response;
        _compress = other._compress;
        _format = other._format;
        _responseHandler = other._responseHandler;

        // Adopt observer and cookie
        _observer = other._observer;
        other._observer = null;
        _cookie = other._cookie;
        other._cookie = null;

        //
        // Deep copy
        //
        if(other._interceptorCBs != null)
        {
            //
            // Copy, not just reference
            //
            _interceptorCBs =
                new java.util.LinkedList<Ice.DispatchInterceptorAsyncCallback>(other._interceptorCBs);
        }
    }

    public Ice.OutputStream
    startWriteParams()
    {
        if(!_response)
        {
            throw new Ice.MarshalException("can't marshal out parameters for oneway dispatch");
        }

        _os = new Ice.OutputStream(_instance, Protocol.currentProtocolEncoding);
        _os.writeBlob(Protocol.replyHdr);
        _os.writeInt(_current.requestId);
        _os.writeByte(ReplyStatus.replyOK);
        _os.startEncapsulation(_current.encoding, _format);
        return _os;
    }

    public void
    endWriteParams()
    {
        if(_response)
        {
            _os.endEncapsulation();
        }
    }

    public void
    writeEmptyParams()
    {
        if(_response)
        {
            _os = new Ice.OutputStream(_instance, Protocol.currentProtocolEncoding);
            _os.writeBlob(Protocol.replyHdr);
            _os.writeInt(_current.requestId);
            _os.writeByte(ReplyStatus.replyOK);
            _os.writeEmptyEncapsulation(_current.encoding);
        }
    }

    public void
    writeParamEncaps(byte[] v, boolean ok)
    {
        if(!ok && _observer != null)
        {
            _observer.userException();
        }

        if(_response)
        {
            _os = new Ice.OutputStream(_instance, Protocol.currentProtocolEncoding);
            _os.writeBlob(Protocol.replyHdr);
            _os.writeInt(_current.requestId);
            _os.writeByte(ok ? ReplyStatus.replyOK : ReplyStatus.replyUserException);
            if(v == null || v.length == 0)
            {
                _os.writeEmptyEncapsulation(_current.encoding);
            }
            else
            {
                _os.writeEncapsulation(v);
            }
        }
    }

    //
    // These functions allow this object to be reused, rather than reallocated.
    //
    public void
    reset(Instance instance, ResponseHandler handler, Ice.ConnectionI connection, Ice.ObjectAdapter adapter,
          boolean response, byte compress, int requestId)
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

        _interceptorCBs = null;
    }

    public void
    reclaim()
    {
        _current = null;
        _servant = null;
        _locator = null;

        if(_cookie != null)
        {
            _cookie.value = null;
        }

        //_observer = null;
        assert(_observer == null);

        _os = null;

        _responseHandler = null;

        _interceptorCBs = null;
    }

    final protected void
    response(boolean amd)
    {
        try
        {
            if(_locator != null && !servantLocatorFinished(amd))
            {
                return;
            }

            assert(_responseHandler != null);
            if(_response)
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
            _responseHandler.invokeException(_current.requestId, ex, 1, amd); // Fatal invocation exception
        }

        if(_observer != null)
        {
            _observer.detach();
            _observer = null;
        }
        _responseHandler = null;
    }

    final protected void
    exception(java.lang.Throwable exc, boolean amd)
    {
        try
        {
            if(_locator != null && !servantLocatorFinished(amd))
            {
                return;
            }
            handleException(exc, amd);
        }
        catch(Ice.LocalException ex)
        {
            _responseHandler.invokeException(_current.requestId, ex, 1, amd); // Fatal invocation exception
        }
    }

    final protected void
    warning(java.lang.Throwable ex)
    {
        assert(_instance != null);

        java.io.StringWriter sw = new java.io.StringWriter();
        java.io.PrintWriter pw = new java.io.PrintWriter(sw);
        IceUtilInternal.OutputBase out = new IceUtilInternal.OutputBase(pw);
        out.setUseTab(false);
        out.print("dispatch exception:");
        out.print("\nidentity: " + Ice.Util.identityToString(_current.id));
        out.print("\nfacet: " + IceUtilInternal.StringUtil.escapeString(_current.facet, ""));
        out.print("\noperation: " + _current.operation);
        if(_current.con != null)
        {
            for(Ice.ConnectionInfo connInfo = _current.con.getInfo(); connInfo != null; connInfo = connInfo.underlying)
            {
                if(connInfo instanceof Ice.IPConnectionInfo)
                {
                    Ice.IPConnectionInfo ipConnInfo = (Ice.IPConnectionInfo)connInfo;
                    out.print("\nremote host: " + ipConnInfo.remoteAddress + " remote port: " + ipConnInfo.remotePort);
                }
            }
        }
        out.print("\n");
        ex.printStackTrace(pw);
        pw.flush();
        _instance.initializationData().logger.warning(sw.toString());
    }

    final protected boolean
    servantLocatorFinished(boolean amd)
    {
        assert(_locator != null && _servant != null);
        try
        {
            assert(_cookie != null);
            _locator.finished(_current, _servant, _cookie.value);
            return true;
        }
        catch(java.lang.Throwable ex)
        {
            handleException(ex, amd);
        }
        return false;
    }

    final protected void
    handleException(java.lang.Throwable exc, boolean amd)
    {
        assert(_responseHandler != null);

        if(exc instanceof Ice.SystemException)
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
                _os = new Ice.OutputStream(_instance, Protocol.currentProtocolEncoding);
                _os.writeBlob(Protocol.replyHdr);
                _os.writeInt(_current.requestId);
                if(ex instanceof Ice.ObjectNotExistException)
                {
                    _os.writeByte(ReplyStatus.replyObjectNotExist);
                }
                else if(ex instanceof Ice.FacetNotExistException)
                {
                    _os.writeByte(ReplyStatus.replyFacetNotExist);
                }
                else if(ex instanceof Ice.OperationNotExistException)
                {
                    _os.writeByte(ReplyStatus.replyOperationNotExist);
                }
                else
                {
                    assert(false);
                }
                ex.id.__write(_os);

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
                _os = new Ice.OutputStream(_instance, Protocol.currentProtocolEncoding);
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
                _os = new Ice.OutputStream(_instance, Protocol.currentProtocolEncoding);
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
                _os = new Ice.OutputStream(_instance, Protocol.currentProtocolEncoding);
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
                _os = new Ice.OutputStream(_instance, Protocol.currentProtocolEncoding);
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
        catch(Ice.UserException ex)
        {
            if(_observer != null)
            {
                _observer.userException();
            }

            if(_response)
            {
                _os = new Ice.OutputStream(_instance, Protocol.currentProtocolEncoding);
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
                _responseHandler.sendResponse(_current.requestId, _os, _compress, amd);
            }
            else
            {
                _responseHandler.sendNoResponse();
            }
        }
        catch(java.lang.Throwable ex)
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
                _os = new Ice.OutputStream(_instance, Protocol.currentProtocolEncoding);
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

            if(!amd && ex instanceof java.lang.Error)
            {
                throw new ServantError((java.lang.Error)ex);
            }
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

    protected Instance _instance;
    protected Ice.Current _current;
    protected Ice.Object _servant;
    protected Ice.ServantLocator _locator;
    protected Ice.LocalObjectHolder _cookie;
    protected Ice.Instrumentation.DispatchObserver _observer;

    protected boolean _response;
    protected byte _compress;
    protected Ice.FormatType _format;

    protected Ice.OutputStream _os;

    protected ResponseHandler _responseHandler;

    protected java.util.LinkedList<Ice.DispatchInterceptorAsyncCallback> _interceptorCBs;
}
