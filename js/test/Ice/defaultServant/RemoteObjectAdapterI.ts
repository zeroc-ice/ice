// Copyright (c) ZeroC, Inc.

import { Ice } from "@zeroc/ice";
import { Test } from "./Test.js";

export class RemoteObjectAdapterI extends Test.RemoteObjectAdapter {
    _objectAdapter: Ice.ObjectAdapter;
    _servant: Ice.Object;

    constructor(objectAdapter: Ice.ObjectAdapter, servant: Ice.Object) {
        super();
        this._objectAdapter = objectAdapter;
        this._servant = servant;
    }

    addDefaultServant(category: string, current: Ice.Current) {
        this._objectAdapter.addDefaultServant(this._servant, category);
    }

    removeDefaultServant(category: string, current: Ice.Current) {
        this._objectAdapter.removeDefaultServant(category);
    }

    shutdown(current: Ice.Current) {
        current.adapter.getCommunicator().shutdown();
    }
}
