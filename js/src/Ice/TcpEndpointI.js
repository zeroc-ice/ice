// Copyright (c) ZeroC, Inc.

import { HashUtil } from "./HashUtil.js";
import { StringUtil } from "./StringUtil.js";
import { TCPEndpointInfo } from "./Endpoint.js";
import { ParseException } from "./LocalExceptions.js";
import { IPEndpointI } from "./IPEndpointI.js";
import { EndpointInfo as SSLEndpointInfo } from "./SSL/EndpointInfo.js";
import { TcpTransceiver } from "./TcpTransceiver.js";

const defaultTimeout = 60000; // 60,000 milliseconds (1 minute)

export class TcpEndpointI extends IPEndpointI {
    constructor(instance, host, port, sourceAddress, timeout, connectionId, compress) {
        super(instance, host, port, sourceAddress, connectionId);
        // The timeout is not used in Ice 3.8 or greater.
        this._timeout = timeout === undefined ? (instance ? defaultTimeout : undefined) : timeout;
        this._compress = compress === undefined ? false : compress;
    }

    //
    // Return the endpoint information.
    //
    getInfo() {
        let info = new TCPEndpointInfo(
            this._timeout,
            this._compress,
            this._host,
            this._port,
            this._sourceAddr,
            this.type(),
            this.secure(),
        );

        if (this.secure()) {
            info = new SSLEndpointInfo(info);
        }

        return info;
    }

    //
    // Return the timeout for the endpoint in milliseconds. 0 means
    // non-blocking, -1 means no timeout.
    //
    timeout() {
        return this._timeout;
    }

    //
    // Return a new endpoint with a different timeout value, provided
    // that timeouts are supported by the endpoint. Otherwise the same
    // endpoint is returned.
    //
    changeTimeout(timeout) {
        if (timeout === this._timeout) {
            return this;
        } else {
            return new TcpEndpointI(
                this._instance,
                this._host,
                this._port,
                this._sourceAddr,
                timeout,
                this._connectionId,
                this._compress,
            );
        }
    }

    //
    // Return a new endpoint with a different connection id.
    //
    changeConnectionId(connectionId) {
        if (connectionId === this._connectionId) {
            return this;
        } else {
            return new TcpEndpointI(
                this._instance,
                this._host,
                this._port,
                this._sourceAddr,
                this._timeout,
                connectionId,
                this._compress,
            );
        }
    }

    //
    // Return true if the endpoints support bzip2 compress, or false
    // otherwise.
    //
    compress() {
        return this._compress;
    }

    //
    // Return a new endpoint with a different compression value,
    // provided that compression is supported by the
    // endpoint. Otherwise the same endpoint is returned.
    //
    changeCompress(compress) {
        if (compress === this._compress) {
            return this;
        } else {
            return new TcpEndpointI(
                this._instance,
                this._host,
                this._port,
                this._sourceAddr,
                this._timeout,
                this._connectionId,
                compress,
            );
        }
    }

    //
    // Return true if the endpoint is datagram-based.
    //
    datagram() {
        return false;
    }

    connectable() {
        // TCP endpoints are not connectable when running in a browser, SSL isn't currently supported.
        return TcpTransceiver !== null && !this.secure();
    }

    connect() {
        DEV: console.assert(!this.secure());
        return new TcpTransceiver(this._instance, this.getAddress(), this._sourceAddr);
    }

    //
    // Convert the endpoint to its string form
    //
    options() {
        //
        // WARNING: Certain features, such as proxy validation in Glacier2,
        // depend on the format of proxy strings. Changes to toString() and
        // methods called to generate parts of the reference string could break
        // these features. Please review for all features that depend on the
        // format of proxyToString() before changing this and related code.
        //
        let s = super.options();

        // We don't print the timeout when it's the default; this timeout is purely for backwards compatibility and
        // has no effect with Ice 3.8 or greater.
        if (this._timeout != defaultTimeout) {
            if (this._timeout == -1) {
                s += " -t infinite";
            } else {
                s += " -t " + this._timeout;
            }
        }

        if (this._compress) {
            s += " -z";
        }
        return s;
    }

    compareTo(p) {
        if (this === p) {
            return 0;
        }

        if (p === null) {
            return 1;
        }

        if (!(p instanceof TcpEndpointI)) {
            return this.type() < p.type() ? -1 : 1;
        }

        if (this._timeout < p._timeout) {
            return -1;
        } else if (p._timeout < this._timeout) {
            return 1;
        }

        if (!this._compress && p._compress) {
            return -1;
        } else if (!p._compress && this._compress) {
            return 1;
        }

        return super.compareTo(p);
    }

    streamWriteImpl(s) {
        super.streamWriteImpl(s);
        s.writeInt(this._timeout);
        s.writeBool(this._compress);
    }

    hashCode() {
        let h = super.hashCode();
        h = HashUtil.addNumber(h, this._timeout);
        h = HashUtil.addBoolean(h, this._compress);
        return h;
    }

    initWithStream(s) {
        super.initWithStream(s);
        this._timeout = s.readInt();
        this._compress = s.readBool();
    }

    checkOption(option, argument, endpoint) {
        if (super.checkOption(option, argument, endpoint)) {
            return true;
        }

        if (option === "-t") {
            if (argument === null) {
                throw new ParseException(`no argument provided for -t option in endpoint ${endpoint}`);
            }

            if (argument == "infinite") {
                this._timeout = -1;
            } else {
                let invalid = false;
                try {
                    this._timeout = StringUtil.toInt32(argument);
                } catch {
                    invalid = true;
                }
                if (invalid || this._timeout < 1) {
                    throw new ParseException(`invalid timeout value '${argument}' in endpoint ${endpoint}`);
                }
            }
        } else if (option === "-z") {
            if (argument !== null) {
                throw new ParseException(`unexpected argument ${argument}' provided for -z option in ${endpoint}`);
            }

            this._compress = true;
        } else {
            return false;
        }
        return true;
    }

    createEndpoint(host, port, connectionId) {
        return new TcpEndpointI(
            this._instance,
            host,
            port,
            this._sourceAddr,
            this._timeout,
            connectionId,
            this._compress,
        );
    }
}
