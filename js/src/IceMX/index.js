// Copyright (c) ZeroC, Inc.

import { IceMX as IceMX_Ice } from "../Ice/Metrics.js";
import { IceMX as IceMX_Glacier2 } from "../Glacier2/Metrics.js";
import { IceMX as IceMX_IceStorm } from "../IceStorm/Metrics.js";

export const IceMX = {
    ...IceMX_Ice,
    ...IceMX_Glacier2,
    ...IceMX_IceStorm,
};
