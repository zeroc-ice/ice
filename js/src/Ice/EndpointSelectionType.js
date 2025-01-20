// Copyright (c) ZeroC, Inc.

import { defineEnum } from "./EnumBase.js";

/**
 *  Determines the order in which the Ice run time uses the endpoints in a proxy when establishing a connection.
 **/
export const EndpointSelectionType = defineEnum([
    ["Random", 0],
    ["Ordered", 1],
]);
