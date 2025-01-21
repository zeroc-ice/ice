// Copyright (c) ZeroC, Inc.

import { defineEnum } from "./EnumBase.js";

/**
 *  The output mode for xxxToString method such as identityToString and proxyToString. The actual encoding format for
 *  the string is the same for all modes: you don't need to specify an encoding format or mode when reading such a
 *  string.
 **/
export const ToStringMode = defineEnum([
    ["Unicode", 0],
    ["ASCII", 1],
    ["Compat", 2],
]);
