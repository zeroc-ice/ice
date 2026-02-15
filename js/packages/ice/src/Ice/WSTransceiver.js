// Copyright (c) ZeroC, Inc.

import { ConnectFailedException, ConnectionLostException, SocketException } from "./LocalExceptions.js";
import { WSConnectionInfo, TCPConnectionInfo } from "./Connection.js";
import { ConnectionInfo as SSLConnectionInfo } from "./SSL/ConnectionInfo.js";
import { SocketOperation } from "./SocketOperation.js";
import { Timer } from "./Timer.js";

//
// With Chrome we don't want to close the socket while connection is in progress,
// see comments on close implementation below.
//
// We need to check for Edge browser as it might include Chrome in its user agent.
//
const IsChrome = navigator.userAgent.indexOf("Edge/") === -1 && navigator.userAgent.indexOf("Chrome/") !== -1;
const IsSafari = /^((?!chrome).)*safari/i.test(navigator.userAgent);

const StateNeedConnect = 0;
const StateConnectPending = 1;
const StateConnected = 2;
const StateClosePending = 3;
const StateClosed = 4;

class WSTransceiver {
    constructor(instance, secure, addr, resource) {
        this._readBuffers = [];
        this._readPosition = 0;
        this._maxBufferedAmount = instance.properties().getIcePropertyAsInt("Ice.WS.MaxBufferedAmount");
        this._writeReadyTimeout = 0;

        let url = secure ? "wss" : "ws";
        const isIPv6 = addr.host.indexOf(":") !== -1;
        url += "://" + (isIPv6 ? `[${addr.host}]` : addr.host);
        if (addr.port !== (secure ? 443 : 80)) {
            url += ":" + addr.port;
        }
        url += resource ? resource : "/";

        this._url = url;
        this._fd = null;
        this._addr = addr;
        this._desc = "local address = <not available>\nremote address = " + addr.host + ":" + addr.port;
        this._state = StateNeedConnect;
        this._secure = secure;
        this._exception = null;
    }

    writeReadyTimeout() {
        const t = Math.round(this._writeReadyTimeout);
        this._writeReadyTimeout += this._writeReadyTimeout >= 5 ? 5 : 0.2;
        return Math.min(t, 25);
    }

    setCallbacks(connectedCallback, bytesAvailableCallback, bytesWrittenCallback) {
        this._connectedCallback = connectedCallback;
        this._bytesAvailableCallback = bytesAvailableCallback;
        this._bytesWrittenCallback = bytesWrittenCallback;
    }

    //
    // Returns SocketOperation.None when initialization is complete.
    //
    initialize() {
        if (this._exception) {
            throw this._exception;
        }

        if (this._state === StateNeedConnect) {
            this._state = StateConnectPending;
            try {
                this._fd = new WebSocket(this._url, "ice.zeroc.com");
            } catch (e) {
                this._exception = new ConnectFailedException(this._url, { cause: e });
                throw this._exception;
            }
            this._fd.binaryType = "arraybuffer";
            this._fd.onopen = e => this.socketConnected(e);
            this._fd.onmessage = e => this.socketBytesAvailable(e.data);
            this._fd.onclose = e => this.socketClosed(e);
            this._fd.onerror = e => this.socketClosed(e);
            return SocketOperation.Connect; // Waiting for connect to complete.
        } else if (this._state === StateConnectPending) {
            //
            // Socket is connected.
            //
            this._desc = fdToString(this._addr);
            this._state = StateConnected;
        }

        console.assert(this._state === StateConnected);
        return SocketOperation.None;
    }

    register() {
        //
        // Register the socket data listener.
        //
        this._registered = true;
        if (this._hasBytesAvailable || this._exception) {
            this._hasBytesAvailable = false;
            Timer.setTimeout(() => this._bytesAvailableCallback(), 0);
        }
    }

    unregister() {
        //
        // Unregister the socket data listener.
        //
        this._registered = false;
    }

    close() {
        if (this._fd === null) {
            // Socket creation failed or not yet initialized, the later can happen if the connection creation throws
            // before calling transceiver initialize.
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
        if ((IsChrome || IsSafari) && this._fd.readyState === WebSocket.CONNECTING) {
            this._state = StateClosePending;
            return;
        }
        this._state = StateClosed;
        this._fd.close();
        this._fd = null;
    }

    destroy() {}

    /**
     * Write the given byte buffer to the web socket. The buffer is written using multiple web socket send calls.
     *
     * @param byteBuffer the byte buffer to write.
     * @returns Whether or not the write completed synchronously.
     */
    write(byteBuffer) {
        if (this._exception) {
            throw this._exception;
        } else if (byteBuffer.remaining === 0) {
            return true;
        }
        console.assert(this._fd);

        // If the write buffer is full, we wait until some data has been sent before sending more data.
        // The bufferedAmount represents the number of bytes that have been queued using send() but not yet transmitted to the network.
        // We don't want to keep calling send() or the socket might be closed by the browser.
        // The writeReadyTimeout uses an exponential backoff to avoid busy loop.

        const cb = () => {
            if (this._fd) {
                const packetSize =
                    this._maxBufferedAmount > 0 && byteBuffer.remaining > this._maxBufferedAmount
                        ? this._maxBufferedAmount
                        : byteBuffer.remaining;
                if (this._fd.bufferedAmount + packetSize <= this._maxBufferedAmount) {
                    this._bytesWrittenCallback();
                } else {
                    Timer.setTimeout(cb, this.writeReadyTimeout());
                }
            }
        };

        while (true) {
            const packetSize =
                this._maxBufferedAmount > 0 && byteBuffer.remaining > this._maxBufferedAmount
                    ? this._maxBufferedAmount
                    : byteBuffer.remaining;
            if (byteBuffer.remaining === 0) {
                break;
            }
            console.assert(packetSize > 0);
            if (this._maxBufferedAmount > 0 && this._fd.bufferedAmount + packetSize > this._maxBufferedAmount) {
                Timer.setTimeout(cb, this.writeReadyTimeout());
                return false;
            }
            this._writeReadyTimeout = 0;
            const slice = byteBuffer.b.slice(byteBuffer.position, byteBuffer.position + packetSize);

            this._fd.send(slice);
            byteBuffer.position += packetSize;
        }
        return true;
    }

    read(byteBuffer, moreData) {
        if (this._exception) {
            throw this._exception;
        }

        moreData.value = false;

        if (this._readBuffers.length === 0) {
            return false; // No data available.
        }

        let avail = this._readBuffers[0].byteLength - this._readPosition;
        console.assert(avail > 0);

        while (byteBuffer.remaining > 0) {
            if (avail > byteBuffer.remaining) {
                avail = byteBuffer.remaining;
            }

            new Uint8Array(byteBuffer.b).set(
                new Uint8Array(this._readBuffers[0], this._readPosition, avail),
                byteBuffer.position,
            );

            byteBuffer.position += avail;
            this._readPosition += avail;
            if (this._readPosition === this._readBuffers[0].byteLength) {
                //
                // We've exhausted the current read buffer.
                //
                this._readPosition = 0;
                this._readBuffers.shift();
                if (this._readBuffers.length === 0) {
                    break; // No more data - we're done.
                } else {
                    avail = this._readBuffers[0].byteLength;
                }
            }
        }

        moreData.value = this._readBuffers.length > 0;

        return byteBuffer.remaining === 0;
    }

    type() {
        return this._secure ? "wss" : "ws";
    }

    getInfo(adapterName, connectionId) {
        console.assert(this._fd !== null);

        let info = new TCPConnectionInfo(
            adapterName,
            connectionId,
            "",
            -1,
            this._addr.host,
            this._addr.port,
            this._maxBufferedAmount,
        );

        if (this._secure) {
            info = new SSLConnectionInfo(info);
        }

        return new WSConnectionInfo(info);
    }

    toString() {
        return this._desc;
    }

    socketConnected() {
        if (this._state == StateClosePending) {
            this.close();
            return;
        }

        console.assert(this._connectedCallback !== null);
        this._connectedCallback();
    }

    socketBytesAvailable(buf) {
        console.assert(this._bytesAvailableCallback !== null);
        if (buf.byteLength > 0) {
            this._readBuffers.push(buf);
            if (this._registered) {
                this._bytesAvailableCallback();
            } else if (!this._hasBytesAvailable) {
                this._hasBytesAvailable = true;
            }
        }
    }

    socketClosed(event) {
        if (this._state == StateClosePending) {
            this.close();
            return;
        }

        if (this._exception) {
            return; // Already handled (e.g., onerror followed by onclose)
        }

        if (this._state < StateConnected) {
            this._exception = new ConnectFailedException(this._url, { cause: event });
            this._connectedCallback();
        } else if (event.type === "error" || (event.type === "close" && (event.code === 1000 || event.code === 1006))) {
            // This is either:
            // - CloseEvent with code 1000 CLOSE_NORMAL or code 1006 CLOSE_ABRUPT,
            // - or a generic Event generated by onerror (which doesn't have a code).
            // We use event.type instead of instanceof CloseEvent because Node.js doesn't expose CloseEvent.
            this._exception = new ConnectionLostException(this._url, { cause: event });
        } else {
            this._exception = new SocketException(`Connection to ${this._url} closed.`, { cause: event });
        }

        if (this._registered) {
            this._bytesAvailableCallback();
        }
    }
}

function fdToString(address) {
    return `local address = <not available>\nremote address = ${address.host}:${address.port}`;
}

export { WSTransceiver };
