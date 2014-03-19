// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_FOR_ACTIONSCRIPT_LICENSE file included in this distribution.
//
// **********************************************************************

(function(global){
    require("Ice/Class");
    require("Ice/AsyncResult");
    require("Ice/AsyncStatus");
    require("Ice/BatchOutgoingAsync");
    require("Ice/Protocol");

    var Ice = global.Ice || {};

    var AsyncResult = Ice.AsyncResult;
    var AsyncStatus = Ice.AsyncStatus;
    var BatchOutgoingAsync = Ice.BatchOutgoingAsync;
    var Protocol = Ice.Protocol;

    var ProxyBatchOutgoingAsync = Ice.Class(BatchOutgoingAsync, {
        __init__: function(prx, operation)
        {
            BatchOutgoingAsync.call(this, prx.ice_getCommunicator(), operation);
            this._proxy = prx;
        },
        __send: function()
        {
            Protocol.checkSupportedProtocol(this._proxy.__reference().getProtocol());

            //
            // We don't automatically retry if ice_flushBatchRequests fails. Otherwise, if some batch
            // requests were queued with the connection, they would be lost without being noticed.
            //
            var handler = null;
            var cnt = -1; // Don't retry.
            try
            {
                handler = this._proxy.__getRequestHandler();
                handler.flushAsyncBatchRequests(this);
            }
            catch(__ex)
            {
                cnt = this._proxy.__handleException(handler, __ex, 0, cnt);
            }
        }
    });
    
    Ice.ProxyBatchOutgoingAsync = ProxyBatchOutgoingAsync;
    global.Ice = Ice;
}(typeof (global) === "undefined" ? window : global));
