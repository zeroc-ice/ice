// Copyright (c) ZeroC, Inc.

import { createVitePlugin } from "unplugin";
import { unpluginFactory } from "../unplugin-slice2js.js";

export default createVitePlugin(unpluginFactory);
