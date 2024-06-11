//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { defineEnum } from "./EnumBase.js";

export const FormatType = defineEnum(
    [
        ["DefaultFormat", 0],
        ["CompactFormat", 1],
        ["SlicedFormat", 2]
    ]);
