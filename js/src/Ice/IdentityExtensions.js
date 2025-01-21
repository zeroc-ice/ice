// Copyright (c) ZeroC, Inc.

import { defineStruct } from "./Struct.js";
import { Ice as Ice_Identity } from "./Identity.js";
const { Identity } = Ice_Identity;
import { StreamHelpers } from "./StreamHelpers.js";

// Moved here from Identity.js to break circular dependencies

defineStruct(Identity, true, true);
export const IdentitySeqHelper = StreamHelpers.generateSeqHelper(Identity, false);
