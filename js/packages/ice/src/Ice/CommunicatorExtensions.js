// Copyright (c) ZeroC, Inc.

import { Communicator } from "./Communicator.js";
import { Instance } from "./Instance.js";

Communicator.prototype.createInstance = function (initData) {
    return new Instance(initData);
};
