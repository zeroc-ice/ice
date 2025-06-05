// Copyright (c) ZeroC, Inc.

import { ParseException } from "./LocalExceptions.js";
import { HashUtil } from "./HashUtil.js";
import { WSEndpointInfo } from "./Endpoint.js";
import { EndpointI } from "./EndpointI.js";
import { WSTransceiver } from "./WSTransceiver.js";

export class WSEndpoint extends EndpointI {
    constructor(instance, delegate, resource) {
        super();
        this._instance = instance;
        this._delegate = delegate;
        this._resource = resource || "/";
    }

    getInfo() {
        return new WSEndpointInfo(this._delegate.getInfo(), this._resource);
    }

    type() {
        return this._delegate.type();
    }

    protocol() {
        return this._delegate.protocol();
    }

    streamWrite(s) {
        s.startEncapsulation();
        this._delegate.streamWriteImpl(s);
        s.writeString(this._resource);
        s.endEncapsulation();
    }

    timeout() {
        return this._delegate.timeout();
    }

    connectionId() {
        return this._delegate.connectionId();
    }

    changeTimeout(timeout) {
        if (timeout === this._delegate.timeout()) {
            return this;
        } else {
            return new WSEndpoint(this._instance, this._delegate.changeTimeout(timeout), this._resource);
        }
    }

    changeConnectionId(connectionId) {
        if (connectionId === this._delegate.connectionId()) {
            return this;
        } else {
            return new WSEndpoint(this._instance, this._delegate.changeConnectionId(connectionId), this._resource);
        }
    }

    compress() {
        return this._delegate.compress();
    }

    changeCompress(compress) {
        if (compress === this._delegate.compress()) {
            return this;
        } else {
            return new WSEndpoint(this._instance, this._delegate.changeCompress(compress), this._resource);
        }
    }

    datagram() {
        return this._delegate.datagram();
    }

    secure() {
        return this._delegate.secure();
    }

    connect() {
        return new WSTransceiver(this._instance, this._delegate.secure(), this._delegate.getAddress(), this._resource);
    }

    hashCode() {
        let h = this._delegate.hashCode();
        h = HashUtil.addString(h, this._resource);
        return h;
    }

    compareTo(p) {
        if (this === p) {
            return 0;
        }

        if (p === null) {
            return 1;
        }

        if (!(p instanceof WSEndpoint)) {
            return this.type() < p.type() ? -1 : 1;
        }

        const r = this._delegate.compareTo(p._delegate);
        if (r !== 0) {
            return r;
        }

        if (this._resource !== p._resource) {
            return this._resource < p._resource ? -1 : 1;
        }

        return 0;
    }

    options() {
        //
        // WARNING: Certain features, such as proxy validation in Glacier2,
        // depend on the format of proxy strings. Changes to toString() and
        // methods called to generate parts of the reference string could break
        // these features. Please review for all features that depend on the
        // format of proxyToString() before changing this and related code.
        //
        let s = this._delegate.options();

        if (this._resource !== null && this._resource.length > 0) {
            s += " -r ";
            s += this._resource.indexOf(":") !== -1 ? '"' + this._resource + '"' : this._resource;
        }

        return s;
    }

    toConnectorString() {
        return this._delegate.toConnectorString();
    }

    initWithStream(s) {
        this._resource = s.readString();
    }

    checkOption(option, argument, endpoint) {
        if (option === "-r") {
            if (argument === null) {
                throw new ParseException(`no argument provided for -r option in endpoint ${endpoint}`);
            }
            this._resource = argument;
        } else {
            return false;
        }
        return true;
    }

    connectable() {
        return true;
    }
}
