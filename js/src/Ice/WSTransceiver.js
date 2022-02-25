
//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/ModuleRegistry").Ice;

require("../Ice/ConnectionInfo");
require("../Ice/Debug");
require("../Ice/Exception");
require("../Ice/LocalException");
require("../Ice/SocketOperation");
require("../Ice/Timer");

const IceSSL = Ice._ModuleRegistry.module("IceSSL");

let WSTransceiver = {};

if (typeof WebSocket !== 'undefined')
{
    //
    // With Chrome we don't want to close the socket while connection is in progress,
    // see comments on close implementation below.
    //
    // We need to check for Edge browser as it might include Chrome in its user agent.
    //
    const IsChrome = navigator.userAgent.indexOf("Edge/") === -1 &&
                    navigator.userAgent.indexOf("Chrome/") !== -1;
    const IsSafari = (/^((?!chrome).)*safari/i).test(navigator.userAgent);

    const Debug = Ice.Debug;
    const SocketOperation = Ice.SocketOperation;
    const Timer = Ice.Timer;

    const StateNeedConnect = 0;
    const StateConnectPending = 1;
    const StateConnected = 2;
    const StateClosePending = 3;
    const StateClosed = 4;

    WSTransceiver = class
    {
        constructor(instance)
        {
            this._readBuffers = [];
            this._readPosition = 0;
            this._maxSendPacketSize = instance.properties().getPropertyAsIntWithDefault("Ice.TCP.SndSize", 512 * 1024);
            this._writeReadyTimeout = 0;
        }

        writeReadyTimeout()
        {
            const t = Math.round(this._writeReadyTimeout);
            this._writeReadyTimeout += (this._writeReadyTimeout >= 5 ? 5 : 0.2);
            return Math.min(t, 25);
        }

        setCallbacks(connectedCallback, bytesAvailableCallback, bytesWrittenCallback)
        {
            this._connectedCallback = connectedCallback;
            this._bytesAvailableCallback = bytesAvailableCallback;
            this._bytesWrittenCallback = bytesWrittenCallback;
        }

        //
        // Returns SocketOperation.None when initialization is complete.
        //
        initialize(readBuffer, writeBuffer)
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
                    this._fd.onopen = e => this.socketConnected(e);
                    this._fd.onmessage = e => this.socketBytesAvailable(e.data);
                    this._fd.onclose = e => this.socketClosed(e);
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
        }

        register()
        {
            //
            // Register the socket data listener.
            //
            this._registered = true;
            if(this._hasBytesAvailable || this._exception)
            {
                this._hasBytesAvailable = false;
                Timer.setTimeout(() => this._bytesAvailableCallback(), 0);
            }
        }

        unregister()
        {
            //
            // Unregister the socket data listener.
            //
            this._registered = false;
        }

        close()
        {
            if(this._fd === null)
            {
                Debug.assert(this._exception); // Websocket creation failed.
                return;
            }

            //
            // With Chrome (in particular on macOS) calling close() while the websocket isn't
            // connected yet doesn't abort the connection attempt, and might result in the
            // connection being reused by a different web socket.
            //
            // To workaround this problem, we always wait for the socket to be connected or
            // closed before closing the socket.
            //
            // NOTE: when this workaround is no longer necessary, don't forget removing the
            // StateClosePending state.
            //
            if((IsChrome || IsSafari) && this._fd.readyState === WebSocket.CONNECTING)
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
        }

        //
        // Returns true if all of the data was flushed to the kernel buffer.
        //
        write(byteBuffer)
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

            const cb = () =>
                {
                    if(this._fd)
                    {
                        const packetSize = this._maxSendPacketSize > 0 && byteBuffer.remaining > this._maxSendPacketSize ?
                                this._maxSendPacketSize : byteBuffer.remaining;
                        if(this._fd.bufferedAmount + packetSize <= this._maxSendPacketSize)
                        {
                            this._bytesWrittenCallback(0, 0);
                        }
                        else
                        {
                            Timer.setTimeout(cb, this.writeReadyTimeout());
                        }
                    }
                };

            while(true)
            {
                const packetSize = this._maxSendPacketSize > 0 && byteBuffer.remaining > this._maxSendPacketSize ?
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
                const slice = byteBuffer.b.slice(byteBuffer.position, byteBuffer.position + packetSize);
                this._fd.send(slice);
                byteBuffer.position += packetSize;

                //
                // TODO: WORKAROUND for Safari issue. The websocket accepts all the
                // data (bufferedAmount is always 0). We relinquish the control here
                // to ensure timeouts work properly.
                //
                if(IsSafari && byteBuffer.remaining > 0)
                {
                    Timer.setTimeout(cb, this.writeReadyTimeout());
                    return false;
                }
            }
            return true;
        }

        read(byteBuffer, moreData)
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

            let avail = this._readBuffers[0].byteLength - this._readPosition;
            Debug.assert(avail > 0);

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

            moreData.value = this._readBuffers.length > 0;

            return byteBuffer.remaining === 0;
        }

        type()
        {
            return this._secure ? "wss" : "ws";
        }

        getInfo()
        {
            Debug.assert(this._fd !== null);
            const info = new Ice.WSConnectionInfo();
            const tcpinfo = new Ice.TCPConnectionInfo();
            tcpinfo.localAddress = "";
            tcpinfo.localPort = -1;
            tcpinfo.remoteAddress = this._addr.host;
            tcpinfo.remotePort = this._addr.port;
            info.underlying = this._secure ? new IceSSL.ConnectionInfo(tcpinfo, tcpinfo.timeout, tcpinfo.compress) : tcpinfo;
            info.rcvSize = -1;
            info.sndSize = this._maxSendPacketSize;
            info.headers = {};
            return info;
        }

        checkSendSize(stream)
        {
        }

        setBufferSize(rcvSize, sndSize)
        {
            this._maxSendPacketSize = sndSize;
        }

        toString()
        {
            return this._desc;
        }

        socketConnected(e)
        {
            if(this._state == StateClosePending)
            {
                this.close();
                return;
            }

            Debug.assert(this._connectedCallback !== null);
            this._connectedCallback();
        }

        socketBytesAvailable(buf)
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
        }

        socketClosed(err)
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
        }

        static createOutgoing(instance, secure, addr, resource)
        {
            const transceiver = new WSTransceiver(instance);
            let url = secure ? "wss" : "ws";
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
        }
    }

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
}
else
{
    WSTransceiver = class {}
}
Ice.WSTransceiver = WSTransceiver;

exports.Ice = Ice;
