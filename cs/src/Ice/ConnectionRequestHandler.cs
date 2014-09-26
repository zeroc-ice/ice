// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using System.Collections.Generic;
using Ice.Instrumentation;

namespace IceInternal
{
    public class ConnectionRequestHandler : RequestHandler
    {
        public RequestHandler connect()
        {
            Debug.Assert(false); // This request handler is only created after connection binding.
            return null;
        }

        public RequestHandler update(RequestHandler previousHandler, RequestHandler newHandler)
        {
            try
            {
                if(previousHandler == this)
                {
                    return newHandler;
                }
                else if(previousHandler.getConnection() == _connection)
                {
                    //
                    // If both request handlers point to the same connection, we also
                    // update the request handler. See bug ICE-5489 for reasons why
                    // this can be useful.
                    //
                    return newHandler;
                }
            }
            catch(Ice.Exception)
            {
                // Ignore
            }
            return this;
        }

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

        public bool sendAsyncRequest(OutgoingAsyncMessageCallback outAsync, out Ice.AsyncCallback sentCallback)
        {
            return outAsync.send(_connection, _compress, _response, out sentCallback);
        }

        public void asyncRequestCanceled(OutgoingAsyncMessageCallback outAsync, Ice.LocalException ex)
        {
            _connection.asyncRequestCanceled(outAsync, ex);
        }

        public Reference getReference()
        {
            return _reference;
        }

        public Ice.ConnectionI getConnection()
        {
            return _connection;
        }

        public Ice.ConnectionI waitForConnection()
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
