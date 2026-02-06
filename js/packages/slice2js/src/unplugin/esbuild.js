// Copyright (c) ZeroC, Inc.

import { createEsbuildPlugin } from "unplugin";
import { unpluginFactory } from "../unplugin-slice2js.js";

export default createEsbuildPlugin(unpluginFactory);
