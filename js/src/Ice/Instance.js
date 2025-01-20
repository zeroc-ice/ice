// Copyright (c) ZeroC, Inc.

import { ToStringMode } from "./ToStringMode.js";

export const StateActive = 0;
export const StateDestroyInProgress = 1;
export const StateDestroyed = 2;

//
// Instance - only for use by Communicator
//
export class Instance {
    constructor(initData) {
        this._state = StateActive;
        this._initData = initData;

        this._traceLevels = null;
        this._defaultsAndOverrides = null;
        this._clientConnectionOptions = null;
        this._messageSizeMax = 0;
        this._batchAutoFlushSize = 0;
        this._classGraphDepthMax = 0;
        this._toStringMode = ToStringMode.Unicode;
        this._implicitContext = null;
        this._routerManager = null;
        this._locatorManager = null;
        this._referenceFactory = null;
        this._outgoingConnectionFactory = null;
        this._objectAdapterFactory = null;
        this._retryQueue = null;
        this._endpointHostResolver = null;
        this._endpointFactoryManager = null;
    }
}
