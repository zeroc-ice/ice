// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class IncomingBase
{
    protected
    IncomingBase(Instance instance, Ice.ConnectionI connection, Ice.ObjectAdapter adapter, boolean response,
                 byte compress, int requestId)
    {
        _response = response;
        _compress = compress;
        _os = new BasicStream(instance);
        _connection = connection;

        _current = new Ice.Current();
        _current.id = new Ice.Identity();
        _current.adapter = adapter;
        _current.con = _connection;
        _current.requestId = requestId;

        _cookie = new Ice.LocalObjectHolder();

    }

    protected
    IncomingBase(IncomingBase in) // Adopts the argument. It must not be used afterwards.
    {
        adopt(in);

        //
        // Deep copy
        //
        if(in._interceptorAsyncCallbackList != null)
        {
            //
            // Copy, not just reference
            //
            _interceptorAsyncCallbackList = new java.util.LinkedList(in._interceptorAsyncCallbackList);
        }

        //
        // We don't change _current as it's exposed by Ice::Request
        //
        _current = in._current;
    }

    protected void
    adopt(IncomingBase other)
    {
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

        _os = other._os;
        other._os = null;

        _connection = other._connection;
        other._connection = null;
    }


    //
    // These functions allow this object to be reused, rather than reallocated.
    //
    public void
    reset(Instance instance, Ice.ConnectionI connection, Ice.ObjectAdapter adapter, boolean response, byte compress,
          int requestId)
    {
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

        if(_os == null)
        {
            _os = new BasicStream(instance);
        }

        _connection = connection;

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

        if(_os != null)
        {
            _os.reset();
        }

        _interceptorAsyncCallbackList = null;
    }

    final protected void
    __warning(java.lang.Exception ex)
    {
        assert(_os != null);

        java.io.StringWriter sw = new java.io.StringWriter();
        java.io.PrintWriter pw = new java.io.PrintWriter(sw);
        IceUtil.OutputBase out = new IceUtil.OutputBase(pw);
        out.setUseTab(false);
        out.print("dispatch exception:");
        out.print("\nidentity: " + _os.instance().identityToString(_current.id));
        out.print("\nfacet: " + IceUtil.StringUtil.escapeString(_current.facet, ""));
        out.print("\noperation: " + _current.operation);
        out.print("\n");
        ex.printStackTrace(pw);
        pw.flush();
        _os.instance().initializationData().logger.warning(sw.toString());
    }

    final protected void
    __handleException(java.lang.Exception exc)
    {
        try
        {
            throw exc;
        }
        catch(Ice.RequestFailedException ex)
        {
            if(ex.id == null)
            {
                ex.id = _current.id;
            }
            
            if(ex.facet == null)
            {
                ex.facet = _current.facet;
            }
            
            if(ex.operation == null || ex.operation.length() == 0)
            {
                ex.operation = _current.operation;
            }

            if(_os.instance().initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 1)
            {
                __warning(ex);
            }

            if(_response)
            {
                _os.endWriteEncaps();
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

                _connection.sendResponse(_os, _compress);
            }
            else
            {
                _connection.sendNoResponse();
            }
        }
        catch(Ice.UnknownLocalException ex)
        {
            if(_os.instance().initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
            {
                __warning(ex);
            }

            if(_response)
            {
                _os.endWriteEncaps();
                _os.resize(Protocol.headerSize + 4, false); // Reply status position.
                _os.writeByte(ReplyStatus.replyUnknownLocalException);
                _os.writeString(ex.unknown);
                _connection.sendResponse(_os, _compress);
            }
            else
            {
                _connection.sendNoResponse();
            }
        }
        catch(Ice.UnknownUserException ex)
        {
            if(_os.instance().initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
            {
                __warning(ex);
            }

            if(_response)
            {
                _os.endWriteEncaps();
                _os.resize(Protocol.headerSize + 4, false); // Reply status position.
                _os.writeByte(ReplyStatus.replyUnknownUserException);
                _os.writeString(ex.unknown);
                _connection.sendResponse(_os, _compress);
            }
            else
            {
                _connection.sendNoResponse();
            }
        }
        catch(Ice.UnknownException ex)
        {
            if(_os.instance().initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
            {
                __warning(ex);
            }

            if(_response)
            {
                _os.endWriteEncaps();
                _os.resize(Protocol.headerSize + 4, false); // Reply status position.
                _os.writeByte(ReplyStatus.replyUnknownException);
                _os.writeString(ex.unknown);
                _connection.sendResponse(_os, _compress);
            }
            else
            {
                _connection.sendNoResponse();
            }
        }
        catch(Ice.LocalException ex)
        {
            if(_os.instance().initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
            {
                __warning(ex);
            }

            if(_response)
            {
                _os.endWriteEncaps();
                _os.resize(Protocol.headerSize + 4, false); // Reply status position.
                _os.writeByte(ReplyStatus.replyUnknownLocalException);
                //_os.writeString(ex.toString());
                java.io.StringWriter sw = new java.io.StringWriter();
                java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                ex.printStackTrace(pw);
                pw.flush();
                _os.writeString(sw.toString());
                _connection.sendResponse(_os, _compress);
            }
            else
            {
                _connection.sendNoResponse();
            }
        }
        catch(Ice.UserException ex)
        {
            if(_os.instance().initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
            {
                __warning(ex);
            }
            
            if(_response)
            {
                _os.endWriteEncaps();
                _os.resize(Protocol.headerSize + 4, false); // Reply status position.
                _os.writeByte(ReplyStatus.replyUnknownUserException);
                //_os.writeString(ex.toString());
                java.io.StringWriter sw = new java.io.StringWriter();
                java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                ex.printStackTrace(pw);
                pw.flush();
                _os.writeString(sw.toString());
                _connection.sendResponse(_os, _compress);
            }
            else
            {
                _connection.sendNoResponse();
            }
        }
        catch(java.lang.Exception ex)
        {
            if(_os.instance().initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
            {
                __warning(ex);
            }
            
            if(_response)
            {
                _os.endWriteEncaps();
                _os.resize(Protocol.headerSize + 4, false); // Reply status position.
                _os.writeByte(ReplyStatus.replyUnknownException);
                //_os.writeString(ex.toString());
                java.io.StringWriter sw = new java.io.StringWriter();
                java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                ex.printStackTrace(pw);
                pw.flush();
                _os.writeString(sw.toString());
                _connection.sendResponse(_os, _compress);
            }
            else
            {
                _connection.sendNoResponse();
            }
        }
    }

    protected Ice.Current _current;
    protected Ice.Object _servant;
    protected Ice.ServantLocator _locator;
    protected Ice.LocalObjectHolder _cookie;

    protected boolean _response;
    protected byte _compress;

    protected BasicStream _os;

    protected Ice.ConnectionI _connection;

    protected java.util.LinkedList _interceptorAsyncCallbackList;
}
