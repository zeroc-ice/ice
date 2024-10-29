//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { TcpEndpointI } from "./TcpEndpointI.js";

export class TcpEndpointFactory {
    constructor(instance) {
        this._instance = instance;
    }

    type() {
        return this._instance.type();
    }

    protocol() {
        return this._instance.protocol();
    }

    create(args, oaEndpoint) {
        const e = new TcpEndpointI(this._instance);
        e.initWithOptions(args, oaEndpoint);
        return e;
    }

    read(s) {
        const e = new TcpEndpointI(this._instance);
        e.initWithStream(s);
        return e;
    }

    clone(instance) {
        return new TcpEndpointFactory(instance);
    }
}
