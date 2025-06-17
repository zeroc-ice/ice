// Copyright (c) ZeroC, Inc.

import { Address } from "./Address.js";
import { ParseException } from "./LocalExceptions.js";
import { HashUtil } from "./HashUtil.js";
import { StringUtil } from "./StringUtil.js";
import { EndpointI } from "./EndpointI.js";

export class IPEndpointI extends EndpointI {
    constructor(instance, host, port, sourceAddress, connectionId) {
        super();
        this._instance = instance;
        this._host = host === undefined ? null : host;
        this._port = port === undefined ? 0 : port;
        this._sourceAddr = sourceAddress === undefined ? null : sourceAddress;
        this._connectionId = connectionId === undefined ? "" : connectionId;
    }

    //
    // Marshal the endpoint
    //
    streamWrite(s) {
        s.startEncapsulation();
        this.streamWriteImpl(s);
        s.endEncapsulation();
    }

    //
    // Return the endpoint type
    //
    type() {
        return this._instance.type();
    }

    //
    // Return the protocol string
    //
    protocol() {
        return this._instance.protocol();
    }

    //
    // Return true if the endpoint is secure.
    //
    secure() {
        return this._instance.secure();
    }

    connectionId() {
        return this._connectionId;
    }

    //
    // Return a new endpoint with a different connection id.
    //
    changeConnectionId(connectionId) {
        if (connectionId === this._connectionId) {
            return this;
        } else {
            return this.createEndpoint(this._host, this._port, connectionId);
        }
    }

    //
    // Return the endpoint information.
    //
    hashCode() {
        let h = 5381;
        h = HashUtil.addNumber(h, this.type());
        h = HashUtil.addString(h, this._host);
        h = HashUtil.addNumber(h, this._port);
        h = HashUtil.addString(h, this._sourceAddr);
        h = HashUtil.addString(h, this._connectionId);
        return h;
    }

    options() {
        //
        // WARNING: Certain features, such as proxy validation in Glacier2,
        // depend on the format of proxy strings. Changes to toString() and
        // methods called to generate parts of the reference string could break
        // these features. Please review for all features that depend on the
        // format of proxyToString() before changing this and related code.
        //
        let s = "";

        if (this._host !== null && this._host.length > 0) {
            s += " -h ";
            const addQuote = this._host.indexOf(":") != -1;
            if (addQuote) {
                s += '"';
            }
            s += this._host;
            if (addQuote) {
                s += '"';
            }
        }

        s += " -p " + this._port;

        if (this._sourceAddr !== null && this._sourceAddr.length > 0) {
            s += " --sourceAddress ";
            const addQuote = this._sourceAddr.indexOf(":") != -1;
            if (addQuote) {
                s += '"';
            }
            s += this._sourceAddr;
            if (addQuote) {
                s += '"';
            }
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

        if (!(p instanceof IPEndpointI)) {
            return this.type() < p.type() ? -1 : 1;
        }

        if (this._port < p._port) {
            return -1;
        } else if (p._port < this._port) {
            return 1;
        }

        if (this._host != p._host) {
            return this._host < p._host ? -1 : 1;
        }

        if (this._sourceAddr != p._sourceAddr) {
            return this._sourceAddr < p._sourceAddr ? -1 : 1;
        }

        if (this._connectionId != p._connectionId) {
            return this._connectionId < p._connectionId ? -1 : 1;
        }

        return 0;
    }

    getAddress() {
        return new Address(this._host, this._port);
    }

    //
    // Convert the endpoint to its Connector string form
    //
    toConnectorString() {
        return this._host + ":" + this._port;
    }

    streamWriteImpl(s) {
        s.writeString(this._host);
        s.writeInt(this._port);
    }

    initWithOptions(args, oaEndpoint) {
        super.initWithOptions(args);

        if (this._host === null || this._host.length === 0) {
            this._host = this._instance.defaultHost();
        } else if (this._host == "*") {
            if (oaEndpoint) {
                this._host = "";
            } else {
                throw new ParseException(`'-h *' not valid for proxy endpoint '${this}'`);
            }
        }

        if (this._host === null) {
            this._host = "";
        }

        if (this._sourceAddr === null) {
            if (!oaEndpoint) {
                this._sourceAddr = this._instance.defaultSourceAddress();
            }
        } else if (oaEndpoint) {
            throw new ParseException(`--sourceAddress not valid for object adapter endpoint '${this}'`);
        }
    }

    initWithStream(s) {
        this._host = s.readString();
        this._port = s.readInt();
    }

    checkOption(option, argument, str) {
        if (option === "-h") {
            if (argument === null) {
                throw new ParseException(`no argument provided for -h option in endpoint ${str}`);
            }

            this._host = argument;
        } else if (option === "-p") {
            if (argument === null) {
                throw new ParseException(`no argument provided for -p option in endpoint ${str}`);
            }

            try {
                this._port = StringUtil.toInt32(argument);
            } catch {
                throw new ParseException(`invalid port value '${argument}' in endpoint ${str}`);
            }

            if (this._port < 0 || this._port > 65535) {
                throw new ParseException(`port value '${argument}' out of range in endpoint ${str}`);
            }
        } else if (option === "--sourceAddress") {
            if (argument === null) {
                throw new ParseException(`no argument provided for --sourceAddress option in endpoint ${str}`);
            }

            this._sourceAddr = argument;
        } else {
            return false;
        }
        return true;
    }
}
