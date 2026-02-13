// Copyright (c) ZeroC, Inc.

/**
 * Options for the slice2js unplugin.
 */
export interface Slice2jsOptions {
    /** Slice files or glob patterns (e.g. "*.ice", "**\/*.ice"). */
    inputs: string | string[];
    /** Output directory for generated files. */
    outputDir: string;
    /** Additional include directories. */
    include?: string | string[];
    /** Additional slice2js CLI arguments. */
    args?: string | string[];
    /** Base directory for resolving inputs. */
    cwd?: string;
    /** Directory containing the slice2js binary. Overrides the bundled binary. */
    toolsPath?: string;
    /** Directory containing the Slice definitions. Overrides the bundled path. */
    slicePath?: string;
}
