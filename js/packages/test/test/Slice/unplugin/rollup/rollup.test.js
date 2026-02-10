// Copyright (c) ZeroC, Inc.

import { describe, it, before, after } from "node:test";
import assert from "node:assert/strict";
import fs from "fs";
import path from "path";
import { fileURLToPath } from "url";
import { rollup } from "rollup";
import typescript from "@rollup/plugin-typescript";
import resolve from "@rollup/plugin-node-resolve";
import slice2js from "@zeroc/slice2js/unplugin/rollup";
import { getSliceOptions } from "../testutil.js";

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const generatedDir = path.join(__dirname, "generated");

describe("rollup plugin", () => {
    before(() => fs.rmSync(generatedDir, { recursive: true, force: true }));
    after(() => fs.rmSync(generatedDir, { recursive: true, force: true }));

    it("compiles .ice files and bundles Client.ts", async () => {
        const bundle = await rollup({
            input: path.join(__dirname, "Client.ts"),
            plugins: [
                slice2js({
                    inputs: [path.join(__dirname, "..", "Test.ice")],
                    outputDir: generatedDir,
                    args: ["--typescript"],
                    ...getSliceOptions(),
                }),
                resolve(),
                typescript({
                    tsconfig: path.join(__dirname, "..", "tsconfig.json"),
                    include: [path.join(__dirname, "**/*.ts")],
                }),
            ],
            external: [/^@zeroc\/ice/],
        });

        const { output } = await bundle.generate({ format: "es" });
        await bundle.close();

        assert.ok(fs.existsSync(path.join(generatedDir, "Test.js")));
        assert.ok(fs.existsSync(path.join(generatedDir, "Test.d.ts")));

        const code = output[0].code;
        assert.ok(code.includes("Hello"));
    });
});
