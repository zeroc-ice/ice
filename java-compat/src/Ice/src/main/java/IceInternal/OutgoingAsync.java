// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class OutgoingAsync extends ProxyOutgoingAsyncBase
{
    public static OutgoingAsync check(Ice.AsyncResult r, Ice.ObjectPrx prx, String operation)
    {
        ProxyOutgoingAsyncBase.checkImpl(r, prx, operation);
        try
        {
            return (OutgoingAsync)r;
        }
        catch(ClassCastException ex)
        {
            throw new IllegalArgumentException("Incorrect AsyncResult object for end_" + operation + " method");
        }
    }

    public OutgoingAsync(Ice.ObjectPrx prx, String operation, CallbackBase cb)
    {
        super((Ice.ObjectPrxHelperBase)prx, operation, cb);
        _encoding = Protocol.getCompatibleEncoding(_proxy._getReference().getEncoding());
        _is = null;
    }

    public OutgoingAsync(Ice.ObjectPrx prx, String operation, CallbackBase cb, Ice.InputStream is, Ice.OutputStream os)
    {
        super((Ice.ObjectPrxHelperBase)prx, operation, cb, os);
        _encoding = Protocol.getCompatibleEncoding(_proxy._getReference().getEncoding());
        _is = is;
    }

    public void prepare(String operation, Ice.OperationMode mode, java.util.Map<String, String> ctx,
                        boolean explicitCtx, boolean synchronous)
    {
        Protocol.checkSupportedProtocol(Protocol.getCompatibleProtocol(_proxy._getReference().getProtocol()));

        _mode = mode;
        _synchronous = synchronous;

        if(explicitCtx && ctx == null)
        {
            ctx = _emptyContext;
        }
        _observer = ObserverHelper.get(_proxy, operation, ctx);

        switch(_proxy._getReference().getMode())
        {
            case Reference.ModeTwoway:
            case Reference.ModeOneway:
            case Reference.ModeDatagram:
            {
                _os.writeBlob(IceInternal.Protocol.requestHdr);
                break;
            }

            case Reference.ModeBatchOneway:
            case Reference.ModeBatchDatagram:
            {
                _proxy._getBatchRequestQueue().prepareBatchRequest(_os);
                break;
            }
        }

        Reference ref = _proxy._getReference();

        ref.getIdentity().ice_writeMembers(_os);

        //
        // For compatibility with the old FacetPath.
        //
        String facet = ref.getFacet();
        if(facet == null || facet.length() == 0)
        {
            _os.writeStringSeq(null);
        }
        else
        {
            String[] facetPath = { facet };
            _os.writeStringSeq(facetPath);
        }

        _os.writeString(operation);

        _os.writeByte((byte) mode.value());

        if(ctx != null)
        {
            //
            // Explicit context
            //
            Ice.ContextHelper.write(_os, ctx);
        }
        else
        {
            //
            // Implicit context
            //
            Ice.ImplicitContextI implicitContext = ref.getInstance().getImplicitContext();
            java.util.Map<String, String> prxContext = ref.getContext();

            if(implicitContext == null)
            {
                Ice.ContextHelper.write(_os, prxContext);
            }
            else
            {
                implicitContext.write(prxContext, _os);
            }
        }
    }

    @Override
    public boolean sent()
    {
        return sent(!_proxy.ice_isTwoway()); // done = true if not a two-way proxy (no response expected)
    }

    @Override
    public int invokeRemote(Ice.ConnectionI connection, boolean compress, boolean response) throws RetryException
    {
        _cachedConnection = connection;
        return connection.sendAsyncRequest(this, compress, response, 0);
    }

    @Override
    public int invokeCollocated(CollocatedRequestHandler handler)
    {
        // The stream cannot be cached if the proxy is not a twoway or there is an invocation timeout set.
        if(!_proxy.ice_isTwoway() || _proxy._getReference().getInvocationTimeout() > 0)
        {
            // Disable caching by marking the streams as cached!
            _state |= StateCachedBuffers;
        }
        return handler.invokeAsyncRequest(this, 0, _synchronous);
    }

    @Override
    public void abort(Ice.Exception ex)
    {
        int mode = _proxy._getReference().getMode();
        if(mode == Reference.ModeBatchOneway || mode == Reference.ModeBatchDatagram)
        {
            //
            // If we didn't finish a batch oneway or datagram request, we
            // must notify the connection about that we give up ownership
            // of the batch stream.
            //
            _proxy._getBatchRequestQueue().abortBatchRequest(_os);
        }

        super.abort(ex);
    }

    public void invoke()
    {
        int mode = _proxy._getReference().getMode();
        if(mode == Reference.ModeBatchOneway || mode == Reference.ModeBatchDatagram)
        {
            //
            // NOTE: we don't call sent/completed callbacks for batch AMI requests
            //
            _sentSynchronously = true;
            _proxy._getBatchRequestQueue().finishBatchRequest(_os, _proxy, getOperation());
            finished(true, false);
        }
        else
        {
            //
            // NOTE: invokeImpl doesn't throw so this can be called from the
            // try block with the catch block calling abort() in case of an
            // exception.
            //
            invokeImpl(true); // userThread = true
        }
    }

    @Override
    public final boolean completed(Ice.InputStream is)
    {
        //
        // NOTE: this method is called from ConnectionI.parseMessage
        // with the connection locked. Therefore, it must not invoke
        // any user callbacks.
        //

        assert(_proxy.ice_isTwoway()); // Can only be called for twoways.

        if(_childObserver != null)
        {
            _childObserver.reply(is.size() - Protocol.headerSize - 4);
            _childObserver.detach();
            _childObserver = null;
        }

        byte replyStatus;
        try
        {
            // _is can already be initialized if the invocation is retried
            if(_is == null)
            {
                _is = new Ice.InputStream(_instance, IceInternal.Protocol.currentProtocolEncoding);
            }
            _is.swap(is);
            replyStatus = _is.readByte();

            switch(replyStatus)
            {
            case ReplyStatus.replyOK:
            {
                break;
            }

            case ReplyStatus.replyUserException:
            {
                if(_observer != null)
                {
                    _observer.userException();
                }
                break;
            }

            case ReplyStatus.replyObjectNotExist:
            case ReplyStatus.replyFacetNotExist:
            case ReplyStatus.replyOperationNotExist:
            {
                Ice.Identity id = new Ice.Identity();
                id.ice_readMembers(_is);

                //
                // For compatibility with the old FacetPath.
                //
                String[] facetPath = _is.readStringSeq();
                String facet;
                if(facetPath.length > 0)
                {
                    if(facetPath.length > 1)
                    {
                        throw new Ice.MarshalException();
                    }
                    facet = facetPath[0];
                }
                else
                {
                    facet = "";
                }

                String operation = _is.readString();

                Ice.RequestFailedException ex = null;
                switch(replyStatus)
                {
                case ReplyStatus.replyObjectNotExist:
                {
                    ex = new Ice.ObjectNotExistException();
                    break;
                }

                case ReplyStatus.replyFacetNotExist:
                {
                    ex = new Ice.FacetNotExistException();
                    break;
                }

                case ReplyStatus.replyOperationNotExist:
                {
                    ex = new Ice.OperationNotExistException();
                    break;
                }

                default:
                {
                    assert(false);
                    break;
                }
                }

                ex.id = id;
                ex.facet = facet;
                ex.operation = operation;
                throw ex;
            }

            case ReplyStatus.replyUnknownException:
            case ReplyStatus.replyUnknownLocalException:
            case ReplyStatus.replyUnknownUserException:
            {
                String unknown = _is.readString();

                Ice.UnknownException ex = null;
                switch(replyStatus)
                {
                case ReplyStatus.replyUnknownException:
                {
                    ex = new Ice.UnknownException();
                    break;
                }

                case ReplyStatus.replyUnknownLocalException:
                {
                    ex = new Ice.UnknownLocalException();
                    break;
                }

                case ReplyStatus.replyUnknownUserException:
                {
                    ex = new Ice.UnknownUserException();
                    break;
                }

                default:
                {
                    assert(false);
                    break;
                }
                }

                ex.unknown = unknown;
                throw ex;
            }

            default:
            {
                throw new Ice.UnknownReplyStatusException();
            }
            }

            return finished(replyStatus == ReplyStatus.replyOK, true);
        }
        catch(Ice.Exception ex)
        {
            return completed(ex);
        }
    }

    public Ice.OutputStream startWriteParams(Ice.FormatType format)
    {
        _os.startEncapsulation(_encoding, format);
        return _os;
    }

    public void endWriteParams()
    {
        _os.endEncapsulation();
    }

    public void writeEmptyParams()
    {
        _os.writeEmptyEncapsulation(_encoding);
    }

    public void writeParamEncaps(byte[] encaps)
    {
        if(encaps == null || encaps.length == 0)
        {
            _os.writeEmptyEncapsulation(_encoding);
        }
        else
        {
            _os.writeEncapsulation(encaps);
        }
    }

    public Ice.InputStream startReadParams()
    {
        _is.startEncapsulation();
        return _is;
    }

    public void endReadParams()
    {
        _is.endEncapsulation();
    }

    public void readEmptyParams()
    {
        _is.skipEmptyEncapsulation();
    }

    public byte[] readParamEncaps()
    {
        return _is.readEncapsulation(null);
    }

    public final void throwUserException()
        throws Ice.UserException
    {
        try
        {
            _is.startEncapsulation();
            _is.throwException(null);
        }
        catch(Ice.UserException ex)
        {
            _is.endEncapsulation();
            throw ex;
        }
    }

    @Override
    public void cacheMessageBuffers()
    {
        if(_proxy._getReference().getInstance().cacheMessageBuffers() > 0)
        {
            synchronized(this)
            {
                if((_state & StateCachedBuffers) > 0)
                {
                    return;
                }
                _state |= StateCachedBuffers;
            }

            if(_is != null)
            {
                _is.reset();
            }
            _os.reset();

            _proxy.cacheMessageBuffers(_is, _os);

            _is = null;
            _os = null;
        }
    }

    final private Ice.EncodingVersion _encoding;
    private Ice.InputStream _is;

    //
    // If true this AMI request is being used for a generated synchronous invocation.
    //
    private boolean _synchronous;

    private static final java.util.Map<String, String> _emptyContext = new java.util.HashMap<String, String>();
}
