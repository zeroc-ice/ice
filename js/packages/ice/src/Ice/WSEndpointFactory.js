// Copyright (c) ZeroC, Inc.

import { WSEndpoint } from "./WSEndpoint.js";

export class WSEndpointFactory {
    constructor(instance, delegate) {
        this._instance = instance;
        this._delegate = delegate;
    }

    type() {
        return this._instance.type();
    }

    protocol() {
        return this._instance.protocol();
    }

    create(args, oaEndpoint) {
        const e = new WSEndpoint(this._instance, this._delegate.create(args, oaEndpoint));
        e.initWithOptions(args);
        return e;
    }

    read(s) {
        const e = new WSEndpoint(this._instance, this._delegate.read(s));
        e.initWithStream(s);
        return e;
    }
}
