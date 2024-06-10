//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { defineDictionary } from "./HashMap.js";
import { StreamHelpers } from "./StreamHelpers.js";
import { ObjectHelper } from "./Stream.js";

// TODO rework this defineDictionary is for types defined in Slice
// FacetMap is a local definition
export const [ FacetMap, FacetMapHelper] = defineDictionary(
    StreamHelpers.StringHelper,
    ObjectHelper,
    false,
    undefined,
    "::Ice::Value");
