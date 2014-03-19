// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(global){
    require("Ice/Class");
    require("Ice/Debug");
    require("Ice/ExUtil");
    require("Ice/Network");
    require("Ice/SocketOperation");
    require("Ice/Connection");
    require("Ice/Exception");
    require("Ice/LocalException");
    
    require("Ice/browser/ConnectionInfo");
    
    var Ice = global.Ice || {};
    var IceWS = global.IceWS || {};
    
    var Debug = Ice.Debug;
    var ExUtil = Ice.ExUtil;
    var Network = Ice.Network;
    var SocketOperation = Ice.SocketOperation;
    var Conn = Ice.Connection;
    var LocalException = Ice.LocalException;
    var SocketException = Ice.SocketException;

    var StateNeedConnect = 0;
    var StateConnectPending = 1;
    var StateConnected = 2;
    var StateClosePending = 3;
    var StateClosed = 4;

    var IsFirefox = navigator.userAgent.indexOf("Firefox") !== -1;
    
    var Transceiver = Ice.Class({
        __init__: function(instance)
        {
            this._traceLevels = instance.traceLevels();
            this._logger = instance.initializationData().logger;
            this._readBuffers = [];
            this._readPosition = 0;
        },
        setCallbacks: function(connectedCallback, bytesAvailableCallback, bytesWrittenCallback)
        {
            this._connectedCallback = connectedCallback;
            this._bytesAvailableCallback = bytesAvailableCallback;
            this._bytesWrittenCallback = bytesWrittenCallback;
        },
        //
        // Returns SocketOperation.None when initialization is complete.
        //
        initialize: function(readBuffer, writeBuffer)
        {
            try
            {
                if(this._exception)
                {
                    throw this._exception;
                }

                if(this._state === StateNeedConnect)
                {
                    this._state = StateConnectPending;
                    this._fd = new WebSocket(this._url, "ice.zeroc.com");
                    this._fd.binaryType = "arraybuffer";
                    var self = this;
                    this._fd.onopen = function(e) { self.socketConnected(e); };
                    this._fd.onmessage = function(e) { self.socketBytesAvailable(e.data); };
                    this._fd.onclose = function(e) { self.socketClosed(e); };
                    return SocketOperation.Connect; // Waiting for connect to complete.
                }
                else if(this._state === StateConnectPending)
                {
                    //
                    // Socket is connected.
                    //
                    this._desc = fdToString(this._addr);
                    this._state = StateConnected;
                }
            }
            catch(err)
            {
                if(!this._exception)
                {
                    this._exception = translateError(this._state, err);
                }

                if(this._traceLevels.network >= 2)
                {
                    var s = [];
                    s.push("failed to establish " + this.type() + " connection\n");
                    s.push(fdToString(this._addr));
                    this._logger.trace(this._traceLevels.networkCat, s.join(""));
                }
                throw this._exception;
            }

            Debug.assert(this._state === StateConnected);
            if(this._traceLevels.network >= 1)
            {
                this._logger.trace(this._traceLevels.networkCat, this.type() + 
                                   " connection established\n" + this._desc);
            }
            return SocketOperation.None;
        },
        register: function()
        {
            //
            // Register the socket data listener.
            //
            this._registered = true;
            if(this._hasBytesAvailable || this._exception)
            {
                this._bytesAvailableCallback();
                this._hasBytesAvailable = false;
            }
        },
        unregister: function()
        {
            //
            // Unregister the socket data listener.
            //
            this._registered = false;
        },
        close: function()
        {
            if(this._fd === null)
            {
                Debug.assert(this._exception); // Websocket creation failed.
                return;
            }

            //
            // WORKAROUND: With Firefox, calling close() if the websocket isn't connected 
            // yet doesn't close the connection. The server doesn't receive any close frame
            // and the underlying socket isn't closed causing the server to hang on closing
            // the connection until the browser exits.
            //
            // To workaround this problem, we always wait for the socket to be connected 
            // or closed before closing the socket.
            //
            if(this._fd.readyState === WebSocket.CONNECTING && IsFirefox)
            {
                this._state = StateClosePending;
                return;
            }
            
            if(this._state == StateConnected && this._traceLevels.network >= 1)
            {
                this._logger.trace(this._traceLevels.networkCat, "closing " + this.type() + " connection\n" + 
                                   this._desc);
            }

            Debug.assert(this._fd !== null);
            try
            {
                this._state = StateClosed;
                this._fd.close();
            }
            catch(ex)
            {
                throw translateError(this._state, ex);
            }
            finally
            {
                this._fd = null;
            }
        },
        //
        // Returns true if all of the data was flushed to the kernel buffer.
        //
        write: function(byteBuffer)
        {
            if(this._exception)
            {
                throw this._exception;
            }

            var remaining = byteBuffer.remaining;
            Debug.assert(remaining > 0);
            Debug.assert(this._fd);
            
            //
            // Delay the send if more than 1KB is already queued for
            // sending on the socket. If less, we consider that it's
            // fine to push more data on the socket.
            //
            if(this._fd.bufferedAmount < 1024)
            {
                //
                // Create a slice of the source buffer representing the remaining data to be written.
                //
                var slice = byteBuffer.b.slice(byteBuffer.position, byteBuffer.position + remaining);
                
                //
                // The socket will accept all of the data.
                //
                byteBuffer.position = byteBuffer.position + remaining;
                this._fd.send(slice);
                if(remaining > 0 && this._traceLevels.network >= 3)
                {
                    var msg = "sent " + remaining + " of " + remaining + " bytes via " + this.type() + "\n" +this._desc;
                    this._logger.trace(this._traceLevels.networkCat, msg);
                }
                return true;
            }
            else
            {
                var transceiver = this;
                var writtenCB = function()
                {
                    if(transceiver._fd)
                    {
                        if(transceiver._fd.bufferedAmount === 0 || this._exception)
                        {
                            transceiver._bytesWrittenCallback();
                        }
                        else
                        {
                            setTimeout(writtenCB, 50);
                        }
                    }
                };
                setTimeout(writtenCB, 50);
                return false;
            }
        },
        read: function(byteBuffer, moreData)
        {
            if(this._exception)
            {
                throw this._exception;
            }

            moreData.value = false;

            if(this._readBuffers.length === 0)
            {
                return false; // No data available.
            }

            var avail = this._readBuffers[0].byteLength - this._readPosition;
            Debug.assert(avail > 0);
            var remaining = byteBuffer.remaining;

            while(byteBuffer.remaining > 0)
            {
                if(avail > byteBuffer.remaining)
                {
                    avail = byteBuffer.remaining;
                }
                
                new Uint8Array(byteBuffer.b).set(new Uint8Array(this._readBuffers[0], this._readPosition, avail), 
                                                 byteBuffer.position);
                
                byteBuffer.position += avail;
                this._readPosition += avail;
                if(this._readPosition === this._readBuffers[0].byteLength)
                {
                    //
                    // We've exhausted the current read buffer.
                    //
                    this._readPosition = 0;
                    this._readBuffers.shift();
                    if(this._readBuffers.length === 0)
                    {
                        break; // No more data - we're done.
                    }
                    else
                    {
                        avail = this._readBuffers[0].byteLength;
                    }
                }
            }

            var n = remaining - byteBuffer.remaining;
            if(n > 0 && this._traceLevels.network >= 3)
            {
                var msg = "received " + n + " of " + remaining + " bytes via " + this.type() + "\n" + this._desc;
                this._logger.trace(this._traceLevels.networkCat, msg);
            }

            moreData.value = this._readBuffers.byteLength > 0;

            return byteBuffer.remaining === 0;
        },
        type: function()
        {
            return this._secure ? "wss" : "ws";
        },
        getInfo: function()
        {
            Debug.assert(this._fd !== null);
            var info = this.createInfo();
            
            //
            // The WebSocket API doens't provide this info
            //
            info.localAddress = "";
            info.localPort = -1;
            info.remoteAddress = this._addr.host;
            info.remotePort = this._addr.port;
            return info;
        },
        createInfo: function()
        {
            return new IceWS.ConnectionInfo();
        },
        checkSendSize: function(stream, messageSizeMax)
        {
            if(stream.size > messageSizeMax)
            {
                ExUtil.throwMemoryLimitException(stream.size, messageSizeMax);
            }
        },
        toString: function()
        {
            return this._desc;
        },
        socketConnected: function(e)
        {
            if(this._state == StateClosePending)
            {
                this.close();
                return;
            }

            Debug.assert(this._connectedCallback !== null);
            this._connectedCallback();
        },
        socketBytesAvailable: function(buf)
        {
            Debug.assert(this._bytesAvailableCallback !== null);
            if(buf.byteLength > 0)
            {
                this._readBuffers.push(buf);
                if(this._registered)
                {
                    this._bytesAvailableCallback();
                }
                else if(!this._hasBytesAvailable)
                {
                    this._hasBytesAvailable = true;
                }
            }
        },
        socketClosed: function(err)
        {
            if(this._state == StateClosePending)
            {
                this.close();
                return;
            }
            
            this._exception = translateError(this._state, err);
            if(this._state < StateConnected)
            {
                this._connectedCallback();
            }
            else if(this._registered)
            {
                this._bytesAvailableCallback();
            }
        },
    });
    
    function fdToString(address)
    {
        return "local address = <not available>\nremote address = " + address.host + ":" + address.port;
    }

    function translateError(state, err)
    {
        if(state < StateConnected)
        {
            return new Ice.ConnectFailedException(err.code, err);
        }
        else
        {
            if(err === 1000) // CLOSE_NORMAL
            {
                throw new Ice.ConnectionLostException();
            }
            return new Ice.SocketException(err.code, err);
        }
    }
    
    Transceiver.createOutgoing = function(instance, secure, addr, resource)
    {
        var transceiver = new Transceiver(instance);

        var url = secure ? "wss" : "ws";
        url += "://" + addr.host;
        if(addr.port !== 80)
        {
            url += ":" + addr.port;
        }
        url += resource ? resource : "/";
        transceiver._url = url;
        transceiver._fd = null;
        transceiver._addr = addr;
        transceiver._desc = "remote address: " + addr.host + ":" + addr.port + " <not connected>";
        transceiver._state = StateNeedConnect;
        transceiver._secure = secure;
        transceiver._exception = null;
        
        return transceiver;
    };

    IceWS.Transceiver = Transceiver;
    global.IceWS = IceWS;
}(typeof (global) === "undefined" ? window : global));
