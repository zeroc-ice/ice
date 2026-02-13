// Copyright (c) ZeroC, Inc.

import type { Plugin } from "esbuild";
import type { Slice2jsOptions } from "./options.js";

/**
 * Creates an esbuild plugin for compiling Slice files.
 * @param options - Plugin configuration options.
 * @returns An esbuild plugin.
 */
declare function slice2js(options?: Slice2jsOptions): Plugin;

export default slice2js;
export type { Slice2jsOptions };
