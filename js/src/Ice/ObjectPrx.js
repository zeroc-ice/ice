//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

export class ObjectPrx {
    constructor(communicatorOrReference, proxyString = undefined) {
        // _setup is implemented in ObjectPrxExtensions.js
        this._setup(communicatorOrReference, proxyString);
    }
}
