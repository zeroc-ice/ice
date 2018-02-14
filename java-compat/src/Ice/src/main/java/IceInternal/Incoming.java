// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

import Ice.Instrumentation.CommunicatorObserver;

final public class Incoming extends IncomingBase implements Ice.Request
{
    public
    Incoming(Instance instance, ResponseHandler responseHandler, Ice.ConnectionI connection, Ice.ObjectAdapter adapter,
             boolean response, byte compress, int requestId)
    {
        super(instance, responseHandler, connection, adapter, response, compress, requestId);
    }

    @Override
    public Ice.Current
    getCurrent()
    {
        return _current;
    }

    @Override
    public void
    reclaim()
    {
        super.reclaim();
        _inAsync = null;
        _inParamPos = -1;
    }

    public final void
    push(Ice.DispatchInterceptorAsyncCallback cb)
    {
        if(_interceptorCBs == null)
        {
            _interceptorCBs = new java.util.LinkedList<Ice.DispatchInterceptorAsyncCallback>();
        }

        _interceptorCBs.addFirst(cb);
    }

    public final void
    pop()
    {
        assert _interceptorCBs != null;
        _interceptorCBs.removeFirst();
    }

    public final void
    startOver()
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
            // Reset input stream's position and clear response
            if(_inAsync != null)
            {
                _inAsync.kill(this);
                _inAsync = null;
            }
            _os = null;

            _is.pos(_inParamPos);
        }
    }

    public void
    setAsync(IncomingAsync inAsync)
    {
        assert(_inAsync == null);
        _inAsync = inAsync;
    }

    public void
    setFormat(Ice.FormatType format)
    {
        _format = format;
    }

    public void
    invoke(ServantManager servantManager, Ice.InputStream stream)
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
                throw new Ice.MarshalException();
            }
            _current.facet = facetPath[0];
        }
        else
        {
            _current.facet = "";
        }

        _current.operation = _is.readString();
        _current.mode = Ice.OperationMode.values()[_is.readByte()];
        _current.ctx = new java.util.HashMap<String, String>();
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
                        if(_cookie == null)
                        {
                            _cookie = new Ice.LocalObjectHolder();
                        }
                        assert(_cookie.value == null);
                        _servant = _locator.locate(_current, _cookie);
                    }
                    catch(java.lang.Throwable ex)
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
            catch(java.lang.Exception ex)
            {
                skipReadParams(); // Required for batch requests.
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

            _servant._iceDispatch(this, _current);

            //
            // If the request was not dispatched asynchronously, send the response.
            //
            if(_inAsync == null)
            {
                response(false);
            }
        }
        catch(java.lang.Throwable ex)
        {
            if(_inAsync != null)
            {
                try
                {
                    _inAsync.kill(this);
                    _inAsync = null;
                }
                catch(Ice.ResponseSentException exc)
                {
                    if(ex instanceof java.lang.Error)
                    {
                        throw new ServantError((java.lang.Error)ex);
                    }

                    if(_instance.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 1)
                    {
                        warning(ex);
                    }
                    return;
                }
            }
            exception(ex, false);
        }
        finally
        {
            if(_instance.useApplicationClassLoader())
            {
                Thread.currentThread().setContextClassLoader(null);
            }
        }
    }

    public final Ice.InputStream
    startReadParams()
    {
        //
        // Remember the encoding used by the input parameters, we'll
        // encode the response parameters with the same encoding.
        //
        _current.encoding = _is.startEncapsulation();
        return _is;
    }

    public final void
    endReadParams()
    {
        _is.endEncapsulation();
    }

    public final void
    readEmptyParams()
    {
        _current.encoding = _is.skipEmptyEncapsulation();
    }

    public final void
    skipReadParams()
    {
        _current.encoding = _is.skipEncapsulation();
    }

    public final byte[]
    readParamEncaps()
    {
        _current.encoding = new Ice.EncodingVersion();
        return _is.readEncapsulation(_current.encoding);
    }

    public Incoming next; // For use by ConnectionI.

    private Ice.InputStream _is;
    private IncomingAsync _inAsync;
    private int _inParamPos = -1;
}
