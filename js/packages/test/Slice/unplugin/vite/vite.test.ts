// Copyright (c) ZeroC, Inc.

import { describe, it, before, after } from "node:test";
import assert from "node:assert/strict";
import fs from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";
import { build, type Rollup } from "vite";
import slice2js from "@zeroc/slice2js/unplugin/vite";
import { getSliceOptions } from "../testutil.ts";

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const generatedDir = path.join(__dirname, "generated");

describe("vite plugin", () => {
    before(() => fs.rmSync(generatedDir, { recursive: true, force: true }));
    after(() => fs.rmSync(generatedDir, { recursive: true, force: true }));

    it("compiles .ice files and bundles Client.ts", async () => {
        const result = await build({
            root: __dirname,
            logLevel: "silent",
            build: {
                write: false,
                lib: {
                    entry: path.join(__dirname, "Client.ts"),
                    formats: ["es"],
                },
                rollupOptions: {
                    external: [/^@zeroc\/ice/],
                },
            },
            plugins: [
                slice2js({
                    inputs: [path.join(__dirname, "..", "Test.ice")],
                    outputDir: generatedDir,
                    args: ["--typescript"],
                    ...getSliceOptions(),
                }),
            ],
        });

        assert.ok(fs.existsSync(path.join(generatedDir, "Test.js")));
        assert.ok(fs.existsSync(path.join(generatedDir, "Test.d.ts")));

        const output = (Array.isArray(result) ? result[0] : result) as Rollup.RollupOutput;
        const code = output.output[0].code;
        assert.ok(code.includes("Hello"));
    });
});
