// Copyright (c) ZeroC, Inc.

import { describe, it, before, after } from "node:test";
import assert from "node:assert/strict";
import fs from "fs";
import path from "path";
import { fileURLToPath } from "url";
import webpack from "webpack";
import slice2js from "@zeroc/slice2js/unplugin/webpack";
import { getSliceOptions } from "../testutil.js";

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const generatedDir = path.join(__dirname, "generated");

describe("webpack plugin", () => {
    before(() => fs.rmSync(generatedDir, { recursive: true, force: true }));
    after(() => fs.rmSync(generatedDir, { recursive: true, force: true }));

    it("compiles .ice files and bundles Client.ts", async () => {
        await new Promise((resolve, reject) => {
            webpack(
                {
                    mode: "production",
                    entry: path.join(__dirname, "Client.ts"),
                    output: {
                        path: path.join(generatedDir, "bundle"),
                        filename: "bundle.js",
                        module: true,
                        library: { type: "module" },
                    },
                    experiments: { outputModule: true },
                    module: {
                        rules: [
                            {
                                test: /\.ts$/,
                                use: {
                                    loader: "ts-loader",
                                    options: {
                                        configFile: path.join(__dirname, "..", "tsconfig.json"),
                                        transpileOnly: true,
                                    },
                                },
                                exclude: /node_modules/,
                            },
                        ],
                    },
                    resolve: {
                        extensions: [".ts", ".js"],
                    },
                    externalsType: "module",
                    externals: [/^@zeroc\/ice/],
                    plugins: [
                        slice2js({
                            inputs: [path.join(__dirname, "..", "Test.ice")],
                            outputDir: generatedDir,
                            args: ["--typescript"],
                            ...getSliceOptions(),
                        }),
                    ],
                },
                (err, stats) => {
                    if (err) reject(err);
                    else if (stats.hasErrors()) reject(new Error(stats.compilation.errors[0].message));
                    else resolve(stats);
                },
            );
        });

        assert.ok(fs.existsSync(path.join(generatedDir, "Test.js")));
        assert.ok(fs.existsSync(path.join(generatedDir, "Test.d.ts")));
        assert.ok(fs.existsSync(path.join(generatedDir, "bundle", "bundle.js")));
    });
});
