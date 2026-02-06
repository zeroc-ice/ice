// Copyright (c) ZeroC, Inc.

import { createRollupPlugin } from "unplugin";
import { unpluginFactory } from "../unplugin-slice2js.js";

export default createRollupPlugin(unpluginFactory);
