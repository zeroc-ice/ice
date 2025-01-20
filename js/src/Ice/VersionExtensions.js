// Copyright (c) ZeroC, Inc.

import { defineStruct } from "./Struct.js";
import { Ice as Ice_Version } from "./Version.js";

const { EncodingVersion, ProtocolVersion } = Ice_Version;

defineStruct(EncodingVersion, true, false);
defineStruct(ProtocolVersion, true, false);
