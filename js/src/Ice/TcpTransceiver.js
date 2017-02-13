// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var net = require("net");

var Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module,
    [
        "../Ice/Class",
        "../Ice/Debug",
        "../Ice/ExUtil",
        "../Ice/SocketOperation",
        "../Ice/Connection",
        "../Ice/Exception",
        "../Ice/LocalException",
        "../Ice/Timer"
    ]);

var Debug = Ice.Debug;
var ExUtil = Ice.ExUtil;
var Network = Ice.Network;
var SocketOperation = Ice.SocketOperation;
var LocalException = Ice.LocalException;
var SocketException = Ice.SocketException;
var Timer = Ice.Timer;

var StateNeedConnect = 0;
var StateConnectPending = 1;
var StateProxyConnectRequest = 2;
var StateProxyConnectRequestPending = 3;
var StateConnected = 4;
var StateClosed = 5;

var TcpTransceiver = Ice.Class({
    __init__: function(instance)
    {
        this._logger = instance.logger();
        this._readBuffers = [];
        this._readPosition = 0;
        this._maxSendPacketSize = instance.properties().getPropertyAsIntWithDefault("Ice.TCP.SndSize", 512 * 1024);
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
                this._fd = net.createConnection({port: this._addr.port,
                                                 host: this._addr.host,
                                                 localAddress: this._sourceAddr});

                var self = this;
                this._fd.on("connect", function() { self.socketConnected(); });
                this._fd.on("data", function(buf) { self.socketBytesAvailable(buf); });

                //
                // The error callback can be triggered from the socket
                // write(). We don't want it to dispached right away
                // from within the write() so we delay the call with
                // setImmediate. We do the same for close as a
                // precaution. See also issue #6226.
                //
                this._fd.on("close", function(err) { Timer.setImmediate(function() { self.socketClosed(err); }); });
                this._fd.on("error", function(err) { Timer.setImmediate(function() { self.socketError(err); }); });

                return SocketOperation.Connect; // Waiting for connect to complete.
            }
            else if(this._state === StateConnectPending)
            {
                //
                // Socket is connected.
                //
                this._desc = fdToString(this._fd, this._proxy, this._addr);
                this._state = StateConnected;
            }
            else if(this._state === StateProxyConnectRequest)
            {
                //
                // Write completed.
                //
                this._proxy.endWriteConnectRequest(writeBuffer);
                this._state = StateProxyConnectRequestPending; // Wait for proxy response
                return SocketOperation.Read;
            }
            else if(this._state === StateProxyConnectRequestPending)
            {
                //
                // Read completed.
                //
                this._proxy.endReadConnectRequestResponse(readBuffer);
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
        this._registered = true;
        this._fd.resume();
        if(this._exception)
        {
            this._bytesAvailableCallback();
        }
    },
    unregister: function()
    {
        if(this._fd === null)
        {
            Debug.assert(this._exception); // Socket creation failed.
            return;
        }
        this._registered = false;
        this._fd.pause();
    },
    close: function()
    {
        if(this._fd === null)
        {
            Debug.assert(this._exception); // Socket creation failed.
            return;
        }

        try
        {
            this._fd.destroy();
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

        var bytesTotal = byteBuffer.remaining;
        var packetSize = bytesTotal;
        Debug.assert(packetSize > 0);

        if(this._maxSendPacketSize > 0 && packetSize > this._maxSendPacketSize)
        {
            packetSize = this._maxSendPacketSize;
        }

        while(packetSize > 0)
        {
            var slice = byteBuffer.b.slice(byteBuffer.position, byteBuffer.position + packetSize);

            var self = this;
            var sync = true;
            /*jshint -W083 */
            sync = this._fd.write(slice, null, function() {
                if(sync)
                {
                    return;
                }

                byteBuffer.position = byteBuffer.position + packetSize;
                self._bytesWrittenCallback(packetSize, bytesTotal);
            });
            /*jshint +W083 */

            if(sync)
            {
                byteBuffer.position = byteBuffer.position + packetSize;

                if(this._maxSendPacketSize > 0 && byteBuffer.remaining > this._maxSendPacketSize)
                {
                    packetSize = this._maxSendPacketSize;
                }
                else
                {
                    packetSize = byteBuffer.remaining;
                }
            }
            else
            {
                return false;
            }
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

        var avail = this._readBuffers[0].length - this._readPosition;
        Debug.assert(avail > 0);
        var remaining = byteBuffer.remaining;

        while(byteBuffer.remaining > 0)
        {
            if(avail > byteBuffer.remaining)
            {
                avail = byteBuffer.remaining;
            }

            this._readBuffers[0].copy(byteBuffer.b, byteBuffer.position, this._readPosition,
                                        this._readPosition + avail);

            byteBuffer.position += avail;
            this._readPosition += avail;
            if(this._readPosition === this._readBuffers[0].length)
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
                    avail = this._readBuffers[0].length;
                }
            }
        }
        moreData.value = this._readBuffers.length > 0;

        return byteBuffer.remaining === 0;
    },
    type: function()
    {
        return "tcp";
    },
    getInfo: function()
    {
        Debug.assert(this._fd !== null);
        var info = new Ice.TCPConnectionInfo();
        info.localAddress = this._fd.localAddress;
        info.localPort = this._fd.localPort;
        info.remoteAddress = this._fd.remoteAddress;
        info.remotePort = this._fd.remotePort;
        info.rcvSize = -1;
        info.sndSize = this._maxSendPacketSize;
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
    socketConnected: function()
    {
        Debug.assert(this._connectedCallback !== null);
        this._connectedCallback();
    },
    socketBytesAvailable: function(buf)
    {
        Debug.assert(this._bytesAvailableCallback !== null);

        //
        // TODO: Should we set a limit on how much data we can read?
        // We can call _fd.pause() to temporarily stop reading.
        //
        if(buf.length > 0)
        {
            this._readBuffers.push(buf);
            this._bytesAvailableCallback();
        }
    },
    socketClosed: function(err)
    {
        //
        // Don't call the closed callback if an error occurred; the error callback
        // will be called.
        //
        if(!err)
        {
            this.socketError(null);
        }
    },
    socketError: function(err)
    {
        this._exception = translateError(this._state, err);
        if(this._state < StateConnected)
        {
            this._connectedCallback();
        }
        else if(this._registered)
        {
            this._bytesAvailableCallback();
        }
    }
});

function fdToString(fd, targetAddr)
{
    if(fd === null)
    {
        return "<closed>";
    }

    return addressesToString(fd.localAddress, fd.localPort, fd.remoteAddress, fd.remotePort, targetAddr);
}

function translateError(state, err)
{
    if(!err)
    {
        return new Ice.ConnectionLostException();
    }
    else if(state < StateConnected)
    {
        if(connectionRefused(err.code))
        {
            return new Ice.ConnectionRefusedException(err.code, err);
        }
        else if(connectionFailed(err.code))
        {
            return new Ice.ConnectFailedException(err.code, err);
        }
    }
    else if(connectionLost(err.code))
    {
        return new Ice.ConnectionLostException(err.code, err);
    }
    return new Ice.SocketException(err.code, err);
}

function addressesToString(localHost, localPort, remoteHost, remotePort, targetAddr)
{
    remoteHost = remoteHost === undefined ? null : remoteHost;
    targetAddr = targetAddr === undefined ? null : targetAddr;

    var s = [];
    s.push("local address = ");
    s.push(localHost + ":" + localPort);

    if(remoteHost === null && targetAddr !== null)
    {
        remoteHost = targetAddr.host;
        remotePort = targetAddr.port;
    }

    if(remoteHost === null)
    {
        s.push("\nremote address = <not connected>");
    }
    else
    {
        s.push("\nremote address = ");
        s.push(remoteHost + ":" + remotePort);
    }

    return s.join("");
}

TcpTransceiver.createOutgoing = function(instance, addr, sourceAddr)
{
    var transceiver = new TcpTransceiver(instance);

    transceiver._fd = null;
    transceiver._addr = addr;
    transceiver._sourceAddr = sourceAddr;
    transceiver._desc = "local address = <not connected>\nremote address = " + addr.host + ":" + addr.port;
    transceiver._state = StateNeedConnect;
    transceiver._registered = false;
    transceiver._exception = null;

    return transceiver;
};

TcpTransceiver.createIncoming = function(instance, fd)
{
    var transceiver = new TcpTransceiver(instance);

    transceiver._fd = fd;
    transceiver._addr = null;
    transceiver._sourceAddr = null;
    transceiver._desc = fdToString(fd);
    transceiver._state = StateConnected;
    transceiver._registered = false;
    transceiver._exception = null;

    return transceiver;
};


var ECONNABORTED = "ECONNABORTED";
var ECONNREFUSED = "ECONNREFUSED";
var ECONNRESET = "ECONNRESET";
var EHOSTUNREACH = "EHOSTUNREACH";
var ENETUNREACH = "ENETUNREACH";
var ENOTCONN = "ENOTCONN";
var EPIPE = "EPIPE";
var ESHUTDOWN = "ESHUTDOWN";
var ETIMEDOUT = "ETIMEDOUT";

function connectionRefused(err)
{
    return err == ECONNREFUSED;
}

function connectionFailed(err)
{
    return err == ECONNREFUSED || err == ETIMEDOUT ||
           err == ENETUNREACH || err == EHOSTUNREACH ||
           err == ECONNRESET || err == ESHUTDOWN ||
           err == ECONNABORTED;
}

function connectionLost(err)
{
    return err == ECONNRESET || err == ENOTCONN ||
           err == ESHUTDOWN || err == ECONNABORTED ||
           err == EPIPE;
}

Ice.TcpTransceiver = TcpTransceiver;
module.exports.Ice = Ice;
