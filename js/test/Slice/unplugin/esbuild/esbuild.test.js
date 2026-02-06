// Copyright (c) ZeroC, Inc.

import { describe, it, before, after } from "node:test";
import assert from "node:assert/strict";
import fs from "fs";
import path from "path";
import { fileURLToPath } from "url";
import esbuild from "esbuild";
import slice2js from "@zeroc/slice2js/unplugin/esbuild";
import { getSliceOptions } from "../testutil.js";

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const generatedDir = path.join(__dirname, "generated");

describe("esbuild plugin", () => {
    before(() => fs.rmSync(generatedDir, { recursive: true, force: true }));
    after(() => fs.rmSync(generatedDir, { recursive: true, force: true }));

    it("compiles .ice files and bundles Client.ts", async () => {
        await esbuild.build({
            entryPoints: [path.join(__dirname, "Client.ts")],
            bundle: true,
            outfile: path.join(generatedDir, "bundle", "bundle.js"),
            format: "esm",
            external: ["@zeroc/ice"],
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
        assert.ok(fs.existsSync(path.join(generatedDir, "bundle", "bundle.js")));
    });
});
