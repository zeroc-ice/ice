// Copyright (c) ZeroC, Inc.

import { createWebpackPlugin } from "unplugin";
import { unpluginFactory } from "../unplugin-slice2js.js";

export default createWebpackPlugin(unpluginFactory);
