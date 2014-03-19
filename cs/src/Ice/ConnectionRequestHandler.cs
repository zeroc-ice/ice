// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;
using Ice.Instrumentation;

namespace IceInternal
{
    public class ConnectionRequestHandler : RequestHandler
    {
        public void prepareBatchRequest(BasicStream @out)
        {
            _connection.prepareBatchRequest(@out);
        }

        public void finishBatchRequest(BasicStream @out)
        {
            _connection.finishBatchRequest(@out, _compress);
        }

        public void abortBatchRequest()
        {
            _connection.abortBatchRequest();
        }

        public Ice.ConnectionI sendRequest(Outgoing @out)
        {
            if(!_connection.sendRequest(@out, _compress, _response) || _response)
            {
                return _connection; // The request has been sent or we're expecting a response.
            }
            else
            {
                return null; // The request hasn't been sent yet.
            }
        }

        public bool sendAsyncRequest(OutgoingAsync @out, out Ice.AsyncCallback sentCallback)
        {
            return _connection.sendAsyncRequest(@out, _compress, _response, out sentCallback);
        }

        public bool flushBatchRequests(BatchOutgoing @out)
        {
            return _connection.flushBatchRequests(@out);
        }

        public bool flushAsyncBatchRequests(BatchOutgoingAsync @out, out Ice.AsyncCallback sentCallback)
        {
            return _connection.flushAsyncBatchRequests(@out, out sentCallback);
        }

        public Outgoing getOutgoing(string operation, Ice.OperationMode mode, Dictionary<string, string> context,
                                    InvocationObserver observer)
        {
            return _connection.getOutgoing(this, operation, mode, context, observer);
        }

        public void reclaimOutgoing(Outgoing @out)
        {
            _connection.reclaimOutgoing(@out);
        }

        public Reference getReference()
        {
            return _reference;
        }

        public Ice.ConnectionI getConnection(bool wait)
        {
            return _connection;
        }

        public ConnectionRequestHandler(Reference @ref, Ice.ObjectPrx proxy)
        {
            _reference = @ref;
            _response = _reference.getMode() == Reference.Mode.ModeTwoway;

            _connection = _reference.getConnection(out _compress);

            //
            // If this proxy is for a non-local object, and we are using a router, then
            // add this proxy to the router info object.
            //
            IceInternal.RouterInfo ri = _reference.getRouterInfo();
            if(ri != null)
            {
                ri.addProxy(proxy);
            }
        }

        public ConnectionRequestHandler(Reference @ref, Ice.ConnectionI connection, bool compress)
        {
            _reference = @ref;
            _response = _reference.getMode() == Reference.Mode.ModeTwoway;
            _connection = connection;
            _compress = compress;
        }

        private Reference _reference;
        private bool _response;
        private Ice.ConnectionI _connection;
        private bool _compress;
    }
}
