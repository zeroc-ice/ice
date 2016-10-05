// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module,
    [
        "../Ice/Class",
        "../Ice/Debug",
        "../Ice/ExUtil",
        "../Ice/Network",
        "../Ice/SocketOperation",
        "../Ice/Connection",
        "../Ice/Exception",
        "../Ice/LocalException",
        "../Ice/Timer",
        "../Ice/ConnectionInfo"
    ]);
var IceSSL = Ice.__M.module("IceSSL");

//
// With Chrome we don't want to close the socket while connection is in progress,
// see comments on close implementation below.
//
// We need to check for Edge browser as it might include Chrome in its user agent.
//
var IsChrome = navigator.userAgent.indexOf("Edge/") === -1 &&
               navigator.userAgent.indexOf("Chrome/") !== -1;

var Debug = Ice.Debug;
var ExUtil = Ice.ExUtil;
var Network = Ice.Network;
var SocketOperation = Ice.SocketOperation;
var Conn = Ice.Connection;
var LocalException = Ice.LocalException;
var SocketException = Ice.SocketException;
var Timer = Ice.Timer;

var StateNeedConnect = 0;
var StateConnectPending = 1;
var StateConnected = 2;
var StateClosePending = 3;
var StateClosed = 4;

var WSTransceiver = Ice.Class({
    __init__: function(instance)
    {
        this._logger = instance.logger();
        this._readBuffers = [];
        this._readPosition = 0;
        this._maxSendPacketSize = instance.properties().getPropertyAsIntWithDefault("Ice.TCP.SndSize", 512 * 1024);
        this._writeReadyTimeout = 0;
    },
    writeReadyTimeout: function()
    {
        var t = Math.round(this._writeReadyTimeout);
        this._writeReadyTimeout +=  (this._writeReadyTimeout >= 5 ? 5 : 0.2);
        return Math.min(t, 25);
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
            throw this._exception;
        }

        Debug.assert(this._state === StateConnected);
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
        // With Chrome calling close() while the websocket isn't connected yet
        // doesn't abort the connection attempt, and might result in the connection
        // being reused by a different web socket.
        //
        // To workaround this problem, we always wait for the socket to be
        // connected or closed before closing the socket.
        //
        if(IsChrome && this._fd.readyState === WebSocket.CONNECTING)
        {
            this._state = StateClosePending;
            return;
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
        else if(byteBuffer.remaining === 0)
        {
            return true;
        }
        Debug.assert(this._fd);

        var transceiver = this;
        var cb = function()
        {
            if(transceiver._fd)
            {
                if(transceiver._fd.bufferedAmount + packetSize <= transceiver._maxSendPacketSize)
                {
                    transceiver._bytesWrittenCallback(0, 0);
                }
                else
                {
                    Timer.setTimeout(cb, transceiver.writeReadyTimeout());
                }
            }
        };

        var i = byteBuffer.position;
        while(true)
        {
            var packetSize = (this._maxSendPacketSize > 0 && byteBuffer.remaining > this._maxSendPacketSize) ?
                this._maxSendPacketSize : byteBuffer.remaining;
            if(byteBuffer.remaining === 0)
            {
                break;
            }
            Debug.assert(packetSize > 0);
            if(this._fd.bufferedAmount + packetSize > this._maxSendPacketSize)
            {
                Timer.setTimeout(cb, this.writeReadyTimeout());
                return false;
            }
            this._writeReadyTimeout = 0;
            var slice = byteBuffer.b.slice(byteBuffer.position, byteBuffer.position + packetSize);
            this._fd.send(slice);
            byteBuffer.position = byteBuffer.position + packetSize;
        }
        return true;
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
        var info = this._secure ? new IceSSL.WSSConnectionInfo() : new Ice.WSConnectionInfo();

        //
        // The WebSocket API doens't provide this info
        //
        info.localAddress = "";
        info.localPort = -1;
        info.remoteAddress = this._addr.host;
        info.remotePort = this._addr.port;
        info.rcvSize = -1;
        info.sndSize = this._maxSendPacketSize;
        info.headers = {};
        return info;
    },
    checkSendSize: function(stream)
    {
    },
    setBufferSize: function(rcvSize, sndSize)
    {
        this._maxSendPacketSize = sndSize;
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
        if(err.code === 1000 || err.code === 1006) // CLOSE_NORMAL | CLOSE_ABNORMAL
        {
            return new Ice.ConnectionLostException();
        }
        return new Ice.SocketException(err.code, err);
    }
}

WSTransceiver.createOutgoing = function(instance, secure, addr, resource)
{
    var transceiver = new WSTransceiver(instance);

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
    transceiver._desc = "local address = <not available>\nremote address = " + addr.host + ":" + addr.port;
    transceiver._state = StateNeedConnect;
    transceiver._secure = secure;
    transceiver._exception = null;

    return transceiver;
};

Ice.WSTransceiver = WSTransceiver;
