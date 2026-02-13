// Copyright (c) ZeroC, Inc.

import type { Plugin } from "vite";
import type { Slice2jsOptions } from "./options.js";

/**
 * Creates a Vite plugin for compiling Slice files.
 * @param options - Plugin configuration options.
 * @returns A Vite plugin.
 */
declare function slice2js(options?: Slice2jsOptions): Plugin;

export default slice2js;
export type { Slice2jsOptions };
