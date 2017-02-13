// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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
        if(_response)
        {
            _os = new BasicStream(instance, Protocol.currentProtocolEncoding);
        }

        _current = new Ice.Current();
        _current.id = new Ice.Identity();
        _current.adapter = adapter;
        _current.con = connection;
        _current.requestId = requestId;

        _cookie = new Ice.LocalObjectHolder();
    }

    protected
    IncomingBase(IncomingBase in) // Adopts the argument. It must not be used afterwards.
    {
        //
        // We don't change _current as it's exposed by Ice::Request
        //
        _current = in._current;

        //
        // Deep copy
        //
        if(in._interceptorAsyncCallbackList != null)
        {
            //
            // Copy, not just reference
            //
            _interceptorAsyncCallbackList =
                new java.util.LinkedList<Ice.DispatchInterceptorAsyncCallback>(in._interceptorAsyncCallbackList);
        }

        adopt(in);
    }

    protected void
    adopt(IncomingBase other)
    {
        _instance = other._instance;
        //other._instance = null; // Don't reset _instance.

        _observer = other._observer;
        other._observer = null;

        _servant = other._servant;
        other._servant = null;

        _locator = other._locator;
        other._locator = null;

        _cookie = other._cookie;
        other._cookie = null;

        _response = other._response;
        other._response = false;

        _compress = other._compress;
        other._compress = 0;

        //
        // Adopt the stream - it creates less garbage.
        //
        _os = other._os;
        other._os = null;

        _responseHandler = other._responseHandler;
        other._responseHandler = null;
    }

    public BasicStream
    __startWriteParams(Ice.FormatType format)
    {
        if(!_response)
        {
            throw new Ice.MarshalException("can't marshal out parameters for oneway dispatch");
        }

        assert(_os.size() == Protocol.headerSize + 4); // Reply status position.
        assert(_current.encoding != null); // Encoding for reply is known.
        _os.writeByte((byte)0);
        _os.startWriteEncaps(_current.encoding, format);
        return _os;
    }

    public void
    __endWriteParams(boolean ok)
    {
        if(!ok && _observer != null)
        {
            _observer.userException();
        }

        assert(_response);

        int save = _os.pos();
        _os.pos(Protocol.headerSize + 4); // Reply status position.
        _os.writeByte(ok ? ReplyStatus.replyOK : ReplyStatus.replyUserException);
        _os.pos(save);
        _os.endWriteEncaps();
    }

    public void
    __writeEmptyParams()
    {
        if(_response)
        {
            assert(_os.size() == Protocol.headerSize + 4); // Reply status position.
            assert(_current.encoding != null); // Encoding for reply is known.
            _os.writeByte(ReplyStatus.replyOK);
            _os.writeEmptyEncaps(_current.encoding);
        }
    }

    public void
    __writeParamEncaps(byte[] v, boolean ok)
    {
        if(!ok && _observer != null)
        {
            _observer.userException();
        }

        if(_response)
        {
            assert(_os.size() == Protocol.headerSize + 4); // Reply status position.
            assert(_current.encoding != null); // Encoding for reply is known.
            _os.writeByte(ok ? ReplyStatus.replyOK : ReplyStatus.replyUserException);
            if(v == null || v.length == 0)
            {
                _os.writeEmptyEncaps(_current.encoding);
            }
            else
            {
                _os.writeEncaps(v);
            }
        }
    }

    public void
    __writeUserException(Ice.UserException ex, Ice.FormatType format)
    {
        BasicStream __os = __startWriteParams(format);
        __os.writeUserException(ex);
        __endWriteParams(false);
    }

    //
    // These functions allow this object to be reused, rather than reallocated.
    //
    public void
    reset(Instance instance, ResponseHandler handler, Ice.ConnectionI connection, Ice.ObjectAdapter adapter,
          boolean response, byte compress, int requestId)
    {
        _instance = instance;

        //
        // Don't recycle the Current object, because servants may keep a reference to it.
        //
        _current = new Ice.Current();
        _current.id = new Ice.Identity();
        _current.adapter = adapter;
        _current.con = connection;
        _current.requestId = requestId;

        if(_cookie == null)
        {
            _cookie = new Ice.LocalObjectHolder();
        }

        _response = response;

        _compress = compress;

        if(_response && _os == null)
        {
            _os = new BasicStream(instance, Protocol.currentProtocolEncoding);
        }

        _responseHandler = handler;

        _interceptorAsyncCallbackList = null;
    }

    public void
    reclaim()
    {
        _servant = null;

        _locator = null;

        if(_cookie != null)
        {
            _cookie.value = null;
        }

        _observer = null;

        if(_os != null)
        {
            _os.reset();
        }

        _interceptorAsyncCallbackList = null;
    }

    final protected void
    __warning(java.lang.Exception ex)
    {
        assert(_instance != null);

        java.io.StringWriter sw = new java.io.StringWriter();
        java.io.PrintWriter pw = new java.io.PrintWriter(sw);
        IceUtilInternal.OutputBase out = new IceUtilInternal.OutputBase(pw);
        out.setUseTab(false);
        out.print("dispatch exception:");
        out.print("\nidentity: " + _instance.identityToString(_current.id));
        out.print("\nfacet: " + IceUtilInternal.StringUtil.escapeString(_current.facet, ""));
        out.print("\noperation: " + _current.operation);
        if(_current.con != null)
        {
            try
            {
                Ice.ConnectionInfo connInfo = _current.con.getInfo();
                if(connInfo instanceof Ice.IPConnectionInfo)
                {
                    Ice.IPConnectionInfo ipConnInfo = (Ice.IPConnectionInfo)connInfo;
                    out.print("\nremote host: " + ipConnInfo.remoteAddress + " remote port: " +
                              Integer.toString(ipConnInfo.remotePort));
                }
            }
            catch(Ice.LocalException exc)
            {
                // Ignore.
            }
        }
        out.print("\n");
        ex.printStackTrace(pw);
        pw.flush();
        _instance.initializationData().logger.warning(sw.toString());
    }

    final protected boolean
    __servantLocatorFinished(boolean amd)
    {
        assert(_locator != null && _servant != null);
        try
        {
            _locator.finished(_current, _servant, _cookie.value);
            return true;
        }
        catch(Ice.UserException ex)
        {
            assert(_responseHandler != null);

            if(_observer != null)
            {
                _observer.userException();
            }

            //
            // The operation may have already marshaled a reply; we must overwrite that reply.
            //
            if(_response)
            {
                _os.resize(Protocol.headerSize + 4, false); // Reply status position.
                _os.writeByte(ReplyStatus.replyUserException);
                _os.startWriteEncaps(_current.encoding, Ice.FormatType.DefaultFormat);
                _os.writeUserException(ex);
                _os.endWriteEncaps();
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
        }
        catch(java.lang.Exception ex)
        {
            __handleException(ex, amd);
        }
        catch(java.lang.Error ex)
        {
            __handleError(ex, amd); // Always throws.
        }
        return false;
    }

    final protected void
    __handleException(java.lang.Exception exc, boolean amd)
    {
        assert(_responseHandler != null);

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
                __warning(ex);
            }

            if(_observer != null)
            {
                _observer.failed(ex.ice_name());
            }

            if(_response)
            {
                _os.resize(Protocol.headerSize + 4, false); // Reply status position.
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
                __warning(ex);
            }

            if(_observer != null)
            {
                _observer.failed(ex.ice_name());
            }

            if(_response)
            {
                _os.resize(Protocol.headerSize + 4, false); // Reply status position.
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
                __warning(ex);
            }

            if(_observer != null)
            {
                _observer.failed(ex.ice_name());
            }

            if(_response)
            {
                _os.resize(Protocol.headerSize + 4, false); // Reply status position.
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
                __warning(ex);
            }

            if(_observer != null)
            {
                _observer.failed(ex.ice_name());
            }

            if(_response)
            {
                _os.resize(Protocol.headerSize + 4, false); // Reply status position.
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
            if(ex instanceof Ice.SystemException)
            {
                if(_responseHandler.systemException(_current.requestId, (Ice.SystemException)ex, amd))
                {
                    return;
                }
            }

            if(_instance.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
            {
                __warning(ex);
            }

            if(_observer != null)
            {
                _observer.failed(ex.ice_name());
            }

            if(_response)
            {
                _os.resize(Protocol.headerSize + 4, false); // Reply status position.
                _os.writeByte(ReplyStatus.replyUnknownLocalException);
                //_os.writeString(ex.toString());
                java.io.StringWriter sw = new java.io.StringWriter();
                sw.write(ex.ice_name() + "\n");
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
            if(_instance.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
            {
                __warning(ex);
            }

            if(_observer != null)
            {
                _observer.failed(ex.ice_name());
            }

            if(_response)
            {
                _os.resize(Protocol.headerSize + 4, false); // Reply status position.
                _os.writeByte(ReplyStatus.replyUnknownUserException);
                //_os.writeString(ex.toString());
                java.io.StringWriter sw = new java.io.StringWriter();
                sw.write(ex.ice_name() + "\n");
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
        catch(java.lang.Exception ex)
        {
            if(_instance.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
            {
                __warning(ex);
            }

            if(_observer != null)
            {
                _observer.failed(ex.getClass().getName());
            }

            if(_response)
            {
                _os.resize(Protocol.headerSize + 4, false); // Reply status position.
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
        }

        if(_observer != null)
        {
            _observer.detach();
            _observer = null;
        }
        _responseHandler = null;
    }

    final protected void
    __handleError(java.lang.Error exc, boolean amd)
    {
        assert(_responseHandler != null);

        Ice.UnknownException uex = new Ice.UnknownException(exc);
        java.io.StringWriter sw = new java.io.StringWriter();
        java.io.PrintWriter pw = new java.io.PrintWriter(sw);
        exc.printStackTrace(pw);
        pw.flush();
        uex.unknown = sw.toString();

        if(_instance.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
        {
            __warning(uex);
        }

        if(_observer != null)
        {
            _observer.failed(uex.ice_name());
        }

        if(_response)
        {
            _os.resize(Protocol.headerSize + 4, false); // Reply status position.
            _os.writeByte(ReplyStatus.replyUnknownException);
            _os.writeString(uex.unknown);
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

        throw new ServantError(exc);
    }

    protected Instance _instance;
    protected Ice.Current _current;
    protected Ice.Object _servant;
    protected Ice.ServantLocator _locator;
    protected Ice.LocalObjectHolder _cookie;
    protected Ice.Instrumentation.DispatchObserver _observer;

    protected boolean _response;
    protected byte _compress;

    protected BasicStream _os;

    protected ResponseHandler _responseHandler;

    protected java.util.LinkedList<Ice.DispatchInterceptorAsyncCallback> _interceptorAsyncCallbackList;
}
