// Copyright (c) ZeroC, Inc.

import fg from "fast-glob";
import fs from "fs";
import path from "path";
import { createUnplugin } from "unplugin";
import { compile } from "./slice2js.js";

function toArray(value) {
    if (!value) {
        return [];
    }
    return Array.isArray(value) ? value : [value];
}

/**
 * Runs slice2js for the provided inputs.
 * @param {object} options
 * @param {string[] | string} options.inputs - Slice files or glob patterns (e.g. "*.ice", "**\/*.ice").
 * @param {string} options.outputDir - Output directory for generated files.
 * @param {string[] | string} [options.include] - Additional include directories.
 * @param {string[] | string} [options.args] - Additional slice2js CLI arguments.
 * @param {string} [options.cwd] - Base directory for resolving inputs.
 * @param {string} [options.toolsPath] - Directory containing the slice2js binary (for source builds).
 * @param {string} [options.slicePath] - Directory containing Slice definitions (for source builds).
 */
export async function runSlice2js(options) {
    const cwd = options.cwd ? path.resolve(options.cwd) : process.cwd();
    const outputDir = path.resolve(cwd, options.outputDir);
    const inputs = fg.sync(toArray(options.inputs), { cwd, absolute: true });

    if (inputs.length === 0) {
        return;
    }

    fs.mkdirSync(outputDir, { recursive: true });

    const args = ["--output-dir", outputDir];
    for (const dir of toArray(options.include)) {
        args.push(`-I${path.resolve(cwd, dir)}`);
    }
    args.push(...toArray(options.args));
    args.push(...inputs);

    const exitCode = await compile(args, {
        toolsPath: options.toolsPath,
        slicePath: options.slicePath,
        cwd,
    });

    if (exitCode !== 0) {
        throw new Error(`slice2js failed with exit code ${exitCode}.`);
    }
}

const unpluginFactory = options => {
    return {
        name: "unplugin-slice2js",
        buildStart() {
            const cwd = options?.cwd ? path.resolve(options.cwd) : process.cwd();
            const inputs = fg.sync(toArray(options?.inputs || []), { cwd, absolute: true });

            for (const file of inputs) {
                try {
                    this.addWatchFile(file);
                } catch {
                    // addWatchFile is not supported in all bundlers (e.g. esbuild).
                }
            }

            if (inputs.length > 0) {
                return runSlice2js({ ...options, inputs, cwd });
            }
        },
    };
};

export { unpluginFactory };

export const unpluginSlice2js = createUnplugin(unpluginFactory);

export default unpluginSlice2js;
