//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { Communicator } from "./Communicator.js";
import { Instance } from "./Instance.js";

Communicator.prototype.createInstance = function (initData) {
    return new Instance(initData);
};
