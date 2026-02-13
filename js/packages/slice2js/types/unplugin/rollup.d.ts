// Copyright (c) ZeroC, Inc.

import type { Plugin } from "rollup";
import type { Slice2jsOptions } from "./options.js";

/**
 * Creates a Rollup plugin for compiling Slice files.
 * @param options - Plugin configuration options.
 * @returns A Rollup plugin.
 */
declare function slice2js(options?: Slice2jsOptions): Plugin;

export default slice2js;
export type { Slice2jsOptions };
