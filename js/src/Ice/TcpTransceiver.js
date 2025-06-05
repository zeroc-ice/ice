// Copyright (c) ZeroC, Inc.

import { TCPConnectionInfo } from "./Connection.js";
import { SocketOperation } from "./SocketOperation.js";
import { Timer } from "./Timer.js";
import {
    ConnectionLostException,
    ConnectionRefusedException,
    ConnectFailedException,
    SocketException,
} from "./LocalExceptions.js";

import net from "net";

const StateNeedConnect = 0;
const StateConnectPending = 1;
const StateConnected = 2;

class TcpTransceiver {
    constructor(instance, addr, sourceAddr) {
        this._logger = instance.logger();
        this._readBuffers = [];
        this._readPosition = 0;
        this._maxSendPacketSize = instance.properties().getPropertyAsIntWithDefault("Ice.TCP.SndSize", 512 * 1024);

        this._fd = null;
        this._addr = addr;
        this._sourceAddr = sourceAddr;
        this._desc = "local address = <not connected>\nremote address = " + addr.host + ":" + addr.port;
        this._state = StateNeedConnect;
        this._registered = false;
        this._exception = null;
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
        try {
            if (this._exception) {
                throw this._exception;
            }

            if (this._state === StateNeedConnect) {
                this._state = StateConnectPending;
                this._fd = net.createConnection({
                    port: this._addr.port,
                    host: this._addr.host,
                    localAddress: this._sourceAddr,
                });

                this._fd.on("connect", () => this.socketConnected());
                this._fd.on("data", buf => this.socketBytesAvailable(buf));

                //
                // The error callback can be triggered from the socket
                // write(). We don't want it to be dispatched right away
                // from within the write() so we delay the call with
                // setImmediate. We do the same for close as a
                // precaution. See also issue #6226.
                //
                this._fd.on("close", err => Timer.setImmediate(() => this.socketClosed(err)));
                this._fd.on("error", err => Timer.setImmediate(() => this.socketError(err)));

                return SocketOperation.Connect; // Waiting for connect to complete.
            } else if (this._state === StateConnectPending) {
                //
                // Socket is connected.
                //
                this._desc = fdToString(this._fd, this._addr);
                this._state = StateConnected;
            }
        } catch (err) {
            if (!this._exception) {
                this._exception = translateError(this._state, err);
            }
            throw this._exception;
        }

        DEV: console.assert(this._state === StateConnected);
        return SocketOperation.None;
    }

    register() {
        this._registered = true;
        this._fd.resume();
        if (this._exception) {
            this._bytesAvailableCallback();
        }
    }

    unregister() {
        if (this._fd === null) {
            DEV: console.assert(this._exception); // Socket creation failed.
            return;
        }
        this._registered = false;
        this._fd.pause();
    }

    close() {
        if (this._fd === null) {
            // Socket creation failed or not yet initialized, the later can happen if the connection creation throws
            // before calling transceiver initialize.
            return;
        }

        this._fd.end();
    }

    destroy() {
        if (this._fd === null) {
            // Socket creation failed or not yet initialized, the later can happen if the connection creation throws
            // before calling transceiver initialize.
            return;
        }

        try {
            this._fd.destroy();
        } catch (ex) {
            throw translateError(this._state, ex);
        } finally {
            this._fd = null;
        }
    }

    /**
     * Write the given byte buffer to the socket. The buffer is written using multiple socket write calls.
     *
     * @param byteBuffer the byte buffer to write.
     * @returns Whether or not the write operation completed synchronously.
     **/
    write(byteBuffer) {
        if (this._exception) {
            throw this._exception;
        }

        let packetSize = byteBuffer.remaining;
        DEV: console.assert(packetSize > 0);

        if (this._maxSendPacketSize > 0 && packetSize > this._maxSendPacketSize) {
            packetSize = this._maxSendPacketSize;
        }

        while (packetSize > 0) {
            const slice = byteBuffer.b.slice(byteBuffer.position, byteBuffer.position + packetSize);
            let sync = true;
            sync = this._fd.write(Buffer.from(slice), null, () => {
                if (!sync) {
                    this._bytesWrittenCallback();
                }
            });

            byteBuffer.position += packetSize;

            if (!sync) {
                return false; // Wait for callback to be called before sending more data.
            }

            if (this._maxSendPacketSize > 0 && byteBuffer.remaining > this._maxSendPacketSize) {
                packetSize = this._maxSendPacketSize;
            } else {
                packetSize = byteBuffer.remaining;
            }
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

        let avail = this._readBuffers[0].length - this._readPosition;
        DEV: console.assert(avail > 0);

        while (byteBuffer.remaining > 0) {
            if (avail > byteBuffer.remaining) {
                avail = byteBuffer.remaining;
            }

            this._readBuffers[0].copy(
                Buffer.from(byteBuffer.b),
                byteBuffer.position,
                this._readPosition,
                this._readPosition + avail,
            );

            byteBuffer.position += avail;
            this._readPosition += avail;
            if (this._readPosition === this._readBuffers[0].length) {
                //
                // We've exhausted the current read buffer.
                //
                this._readPosition = 0;
                this._readBuffers.shift();
                if (this._readBuffers.length === 0) {
                    break; // No more data - we're done.
                } else {
                    avail = this._readBuffers[0].length;
                }
            }
        }
        moreData.value = this._readBuffers.length > 0;

        return byteBuffer.remaining === 0;
    }

    type() {
        return "tcp";
    }

    getInfo(adapterName, connectionId) {
        DEV: console.assert(this._fd !== null);
        return new TCPConnectionInfo(
            adapterName,
            connectionId,
            this._fd.localAddress,
            this._fd.localPort,
            this._fd.remoteAddress,
            this._fd.remotePort,
            this._maxSendPacketSize,
        );
    }

    setBufferSize(rcvSize, sndSize) {
        this._maxSendPacketSize = sndSize;
    }

    toString() {
        return this._desc;
    }

    socketConnected() {
        DEV: console.assert(this._connectedCallback !== null);
        this._connectedCallback();
    }

    socketBytesAvailable(buf) {
        DEV: console.assert(this._bytesAvailableCallback !== null);

        //
        // TODO: Should we set a limit on how much data we can read?
        // We can call _fd.pause() to temporarily stop reading.
        //
        if (buf.length > 0) {
            this._readBuffers.push(buf);
            this._bytesAvailableCallback();
        }
    }

    socketClosed(err) {
        //
        // Don't call the closed callback if an error occurred; the error callback
        // will be called.
        //
        if (!err) {
            this.socketError(null);
        }
    }

    socketError(err) {
        this._exception = translateError(this._state, err);
        if (this._state < StateConnected) {
            this._connectedCallback();
        } else if (this._registered) {
            this._bytesAvailableCallback();
        }
    }
}

function fdToString(fd, targetAddr) {
    if (fd === null) {
        return "<closed>";
    }

    return addressesToString(fd.localAddress, fd.localPort, fd.remoteAddress, fd.remotePort, targetAddr);
}

function translateError(state, err) {
    if (!err) {
        return new ConnectionLostException();
    } else if (state < StateConnected) {
        if (connectionRefused(err.code)) {
            return new ConnectionRefusedException("connection refused", { cause: err });
        } else if (connectionFailed(err.code)) {
            return new ConnectFailedException("connect failed", { cause: err });
        }
    } else if (connectionLost(err.code)) {
        return new ConnectionLostException("connection lost", { cause: err });
    }
    return new SocketException("socket exception", { cause: err });
}

function addressesToString(localHost, localPort, remoteHost, remotePort, targetAddr) {
    remoteHost = remoteHost === undefined ? null : remoteHost;
    targetAddr = targetAddr === undefined ? null : targetAddr;

    const s = [];
    s.push("local address = ");
    s.push(localHost + ":" + localPort);

    if (remoteHost === null && targetAddr !== null) {
        remoteHost = targetAddr.host;
        remotePort = targetAddr.port;
    }

    if (remoteHost === null) {
        s.push("\nremote address = <not connected>");
    } else {
        s.push("\nremote address = ");
        s.push(remoteHost + ":" + remotePort);
    }

    return s.join("");
}

const ECONNABORTED = "ECONNABORTED";
const ECONNREFUSED = "ECONNREFUSED";
const ECONNRESET = "ECONNRESET";
const EHOSTUNREACH = "EHOSTUNREACH";
const ENETUNREACH = "ENETUNREACH";
const ENOTCONN = "ENOTCONN";
const EPIPE = "EPIPE";
const ESHUTDOWN = "ESHUTDOWN";
const ETIMEDOUT = "ETIMEDOUT";

function connectionRefused(err) {
    return err == ECONNREFUSED;
}

function connectionFailed(err) {
    return (
        err == ECONNREFUSED ||
        err == ETIMEDOUT ||
        err == ENETUNREACH ||
        err == EHOSTUNREACH ||
        err == ECONNRESET ||
        err == ESHUTDOWN ||
        err == ECONNABORTED
    );
}

function connectionLost(err) {
    return err == ECONNRESET || err == ENOTCONN || err == ESHUTDOWN || err == ECONNABORTED || err == EPIPE;
}

export { TcpTransceiver };
