// Copyright (c) ZeroC, Inc.

import fg from "fast-glob";
import fs from "node:fs";
import path from "node:path";
import { createUnplugin, type UnpluginFactory } from "unplugin";
import { compile, type CompileOptions } from "./slice2js.js";

function toArray(value: string | string[] | undefined): string[] {
    if (!value) {
        return [];
    }
    return Array.isArray(value) ? value : [value];
}

export interface Slice2jsOptions extends CompileOptions {
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
}

/**
 * Runs slice2js for the provided inputs.
 */
export async function runSlice2js(options: Slice2jsOptions): Promise<void> {
    const cwd = options.cwd ? path.resolve(options.cwd) : process.cwd();
    const outputDir = path.resolve(cwd, options.outputDir);
    const patterns = toArray(options.inputs).map((p) => p.replace(/\\/g, "/"));
    const inputs = await fg(patterns, { cwd: cwd.replace(/\\/g, "/"), absolute: true });

    if (inputs.length === 0) {
        return;
    }

    fs.mkdirSync(outputDir, { recursive: true });

    const args: string[] = ["--output-dir", outputDir];
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

export const unpluginFactory: UnpluginFactory<Slice2jsOptions | undefined> = (options) => {
    return {
        name: "unplugin-slice2js",
        async buildStart() {
            const cwd = options?.cwd ? path.resolve(options.cwd) : process.cwd();
            const patterns = toArray(options?.inputs).map((p) => p.replace(/\\/g, "/"));
            const inputs = await fg(patterns, { cwd: cwd.replace(/\\/g, "/"), absolute: true });

            for (const file of inputs) {
                try {
                    this.addWatchFile(file);
                } catch {
                    // addWatchFile is not supported in all bundlers (e.g. esbuild).
                }
            }

            if (inputs.length > 0 && options?.inputs && options.outputDir) {
                return runSlice2js({ ...options, inputs, cwd });
            }
        },
    };
};

export const unpluginSlice2js = createUnplugin(unpluginFactory);

export default unpluginSlice2js;
