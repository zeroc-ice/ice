// Copyright (c) ZeroC, Inc.

import type { WebpackPluginInstance } from "webpack";
import type { Slice2jsOptions } from "./options.js";

/**
 * Creates a webpack plugin for compiling Slice files.
 * @param options - Plugin configuration options.
 * @returns A webpack plugin instance.
 */
declare function slice2js(options?: Slice2jsOptions): WebpackPluginInstance;

export default slice2js;
export type { Slice2jsOptions };
