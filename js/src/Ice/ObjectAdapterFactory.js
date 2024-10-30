//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { ObjectAdapter } from "./ObjectAdapter.js";
import { CommunicatorDestroyedException, AlreadyRegisteredException } from "./LocalExceptions.js";
import { generateUUID } from "./UUID.js";

//
// Only for use by Instance.
//
export class ObjectAdapterFactory {
    constructor(instance, communicator) {
        this._instance = instance;
        this._communicator = communicator;
        this._adapters = [];
        this._adapterNamesInUse = [];
    }

    destroy() {
        // We cannot directly iterate over this._adapters because ObjectAdapter.destroy() will remove the
        // adapter from the list of adapters by calling removeObjectAdapter. We use slice() to create a
        // shallow copy of the array to avoid modifying the array during iteration.
        const adapters = this._adapters.slice();
        for (const adapter of adapters) {
            adapter.destroy();
        }
    }

    createObjectAdapter(name, router, promise) {
        let adapter = null;
        try {
            if (name.length === 0) {
                adapter = new ObjectAdapter(
                    this._instance,
                    this._communicator,
                    this,
                    generateUUID(),
                    null,
                    true,
                    promise,
                );
            } else {
                if (this._adapterNamesInUse.indexOf(name) !== -1) {
                    throw new AlreadyRegisteredException("object adapter", name);
                }
                adapter = new ObjectAdapter(this._instance, this._communicator, this, name, router, false, promise);
                this._adapterNamesInUse.push(name);
            }
            this._adapters.push(adapter);
        } catch (ex) {
            promise.reject(ex);
        }
    }

    removeObjectAdapter(adapter) {
        let n = this._adapters.indexOf(adapter);
        if (n !== -1) {
            this._adapters.splice(n, 1);
        }

        n = this._adapterNamesInUse.indexOf(adapter.getName());
        if (n !== -1) {
            this._adapterNamesInUse.splice(n, 1);
        }
    }
}
