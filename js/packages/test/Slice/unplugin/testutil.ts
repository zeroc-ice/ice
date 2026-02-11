// Copyright (c) ZeroC, Inc.

import fs from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const iceHome = path.resolve(__dirname, "../../../../..");

const cppPlatform = process.env.CPP_PLATFORM || "x64";
const cppConfiguration = process.env.CPP_CONFIGURATION || "Release";

export function getSliceOptions(): { toolsPath?: string; slicePath: string } {
    const slicePath = path.resolve(iceHome, "slice");

    if (process.platform === "win32") {
        return {
            toolsPath: path.resolve(iceHome, "cpp/bin", cppPlatform, cppConfiguration),
            slicePath,
        };
    }

    const sourceBuild = path.resolve(iceHome, "cpp/bin/slice2js");
    if (fs.existsSync(sourceBuild)) {
        return { toolsPath: path.resolve(iceHome, "cpp/bin"), slicePath };
    }

    // Fall back to bundled binary (no toolsPath needed).
    return { slicePath };
}
