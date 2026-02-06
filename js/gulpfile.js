// Copyright (c) ZeroC, Inc.

/* eslint no-sync: "off" */
/* eslint no-process-env: "off" */
/* eslint no-process-exit: "off" */

import { deleteAsync } from "del";
import fs from "fs";
import gulp from "gulp";
import tsc from "gulp-typescript";
import path from "path";
import { finished } from "node:stream/promises";
import { rollup } from "rollup";
import strip from "@rollup/plugin-strip";
import resolve from "@rollup/plugin-node-resolve";
import { fileURLToPath } from "url";
import { exec } from "node:child_process";
import { runSlice2js } from "@zeroc/slice2js/unplugin";

const __dirname = path.dirname(fileURLToPath(import.meta.url));

const optimize = (process.env.OPTIMIZE || "no") == "yes";

function parseArg(argv, key) {
    for (let i = 0; i < argv.length; ++i) {
        const e = argv[i];
        if (e == key) {
            return argv[i + 1];
        } else if (e.indexOf(key + "=") === 0) {
            return e.substr(key.length + 1);
        }
    }
}

const platform = parseArg(process.argv, "--cppPlatform") || process.env.CPP_PLATFORM || "x64";
const configuration = parseArg(process.argv, "--cppConfiguration") || process.env.CPP_CONFIGURATION || "Release";

// Determine toolsPath and slicePath for source builds.
// When toolsPath is set, compile() in slice2js.js uses the binary from that directory.
// Otherwise, it falls back to the bundled binary from @zeroc/ice.
function getSliceOptions() {
    const iceHome = path.resolve(__dirname, "..");
    const slicePath = path.resolve(iceHome, "slice");

    if (process.platform === "win32") {
        if (!platform || (platform.toLowerCase() != "win32" && platform.toLowerCase() != "x64")) {
            console.log("Error: --cppPlatform must be set to `Win32' or `x64', in order to locate slice2js.exe");
            process.exit(1);
        }

        if (!configuration || (configuration.toLowerCase() != "debug" && configuration.toLowerCase() != "release")) {
            console.log(
                "Error: --cppConfiguration must be set to `Debug' or `Release', in order to locate slice2js.exe",
            );
            process.exit(1);
        }
        return {
            toolsPath: path.resolve(iceHome, "cpp/bin", platform, configuration),
            slicePath,
        };
    }

    // For non-Windows source builds, check if the C++ build output exists.
    const sourceBuild = path.resolve(iceHome, "cpp/bin/slice2js");
    if (fs.existsSync(sourceBuild)) {
        return { toolsPath: path.resolve(iceHome, "cpp/bin"), slicePath };
    }

    // Fall back to bundled binary (no toolsPath needed).
    return { slicePath };
}

const sliceOptions = getSliceOptions();

const root = path.resolve(__dirname);
const libs = ["Ice", "Glacier2", "IceStorm", "IceGrid"];

const libTask = (libName, taskName) => libName + ":" + taskName;

const excludes = {
    Ice: ["Identity.ice", "Version.ice"],
};

//
// Library generation tasks: compile .ice -> .js + .d.ts using runSlice2js.
//
for (const lib of libs) {
    const sliceDir = path.resolve(__dirname, `../slice/${lib}`);
    const excludedFiles = excludes[lib] || [];

    gulp.task(libTask(lib, "generate"), async () => {
        const allFiles = fs
            .readdirSync(sliceDir)
            .filter(f => f.endsWith(".ice") && !excludedFiles.includes(f))
            .map(f => path.resolve(sliceDir, f));

        if (allFiles.length > 0) {
            await runSlice2js({
                inputs: allFiles,
                outputDir: path.resolve(root, "src", lib),
                args: ["--typescript"],
                ...sliceOptions,
            });
        }
    });

    gulp.task(libTask(lib, "clean"), async () => {
        const toDelete = fs
            .readdirSync(sliceDir)
            .filter(f => f.endsWith(".ice") && !excludedFiles.includes(f))
            .flatMap(f => {
                const baseName = path.basename(f, ".ice");
                return [
                    path.resolve(root, "src", lib, `${baseName}.js`),
                    path.resolve(root, "src", lib, `${baseName}.d.ts`),
                ];
            });
        await deleteAsync(toDelete, { force: true });
    });
}

gulp.task("dist", gulp.parallel(libs.map(libName => libTask(libName, "generate"))));

gulp.task("dist:clean", gulp.parallel(libs.map(libName => libTask(libName, "clean"))));

//
// Tests
//
const tests = [
    "test/Ice/adapterDeactivation",
    "test/Ice/ami",
    "test/Ice/binding",
    "test/Ice/defaultValue",
    "test/Ice/defaultServant",
    "test/Ice/enums",
    "test/Ice/exceptions",
    "test/Ice/facets",
    "test/Ice/idleTimeout",
    "test/Ice/inactivityTimeout",
    "test/Ice/info",
    "test/Ice/inheritance",
    "test/Ice/location",
    "test/Ice/middleware",
    "test/Ice/objects",
    "test/Ice/operations",
    "test/Ice/optional",
    "test/Ice/properties",
    "test/Ice/proxy",
    "test/Ice/retry",
    "test/Ice/servantLocator",
    "test/Ice/slicing/exceptions",
    "test/Ice/slicing/objects",
    "test/Ice/stream",
    "test/Ice/timeout",
    "test/Ice/scope",
    "test/Glacier2/router",
    "test/Slice/escape",
    "test/Slice/macros",
    "test/Slice/moduleMapping",
];

// Shared TypeScript compiler options for all test builds.
const tsCompilerOptions = {
    lib: ["dom", "es2020"],
    target: "es2020",
    module: "es2020",
    moduleResolution: "bundler",
    strict: true,
    noImplicitAny: true,
    noImplicitOverride: true,
    noFallthroughCasesInSwitch: true,
    noImplicitReturns: true,
    noUnusedLocals: true,
    noUnusedParameters: true,
};

// Ice bundle: bundle the Ice runtime for browser usage.
gulp.task("ice:bundle", async () => {
    const plugins = [resolve({ browser: true })];
    if (optimize) {
        plugins.push(
            strip({
                functions: ["console.assert"],
            }),
        );
    }

    let bundle = await rollup({
        input: "src/index.js",
        plugins: plugins,
    });
    await bundle.write({
        file: "dist/ice.js",
        format: "esm",
    });
    await bundle.close();
});

// Test common: compile Controller.ice and bundle common test infrastructure.
gulp.task("test:common:build", async () => {
    await runSlice2js({
        inputs: [path.resolve("../scripts/Controller.ice")],
        outputDir: path.resolve("test/Common"),
        ...sliceOptions,
    });

    let bundle = await rollup({
        input: ["test/Common/ControllerI.js", "test/Common/ControllerWorker.js"],
        external: ["@zeroc/ice"],
    });
    await bundle.write({
        format: "esm",
        dir: "dist/test/Common/",
        paths: { "@zeroc/ice": "/ice.js" },
    });
    await bundle.close();
});

gulp.task("test:common:clean", async () => {
    await deleteAsync(["test/Common/Controller.js", "test/Common/.depend"]);
});

const testTask = (testName, taskName) => testName.replace(/\//g, "_") + ":" + taskName;

for (const name of tests) {
    const testDir = `${root}/${name}`;

    // Build test: compile .ice → .js/.d.ts, compile .ts → .js (in-place for Node.js), then bundle for browser.
    gulp.task(testTask(name, "build"), async () => {
        // Step 1: Compile .ice → .js + .d.ts
        const iceFiles = fs.existsSync(testDir)
            ? fs
                  .readdirSync(testDir)
                  .filter(f => f.endsWith(".ice"))
                  .map(f => path.resolve(testDir, f))
            : [];

        if (iceFiles.length > 0) {
            await runSlice2js({
                inputs: iceFiles,
                outputDir: testDir,
                args: ["--typescript"],
                ...sliceOptions,
            });
        }

        // Step 2: Compile .ts → .js in-place (for Node.js test runner)
        await finished(
            gulp
                .src([`${testDir}/*.ts`, `!${testDir}/*.d.ts`])
                .pipe(tsc(tsCompilerOptions))
                .pipe(gulp.dest(testDir)),
        );

        // Step 3: Bundle .js files for browser
        let input;
        if (fs.existsSync(`${name}/index.js`)) {
            input = `${name}/index.js`;
        } else {
            input = `${name}/Client.js`;
        }

        let bundle = await rollup({
            input: input,
            external: ["@zeroc/ice", "fs", "path"],
            onwarn: (warning, next) => {
                if (warning.code === "THIS_IS_UNDEFINED") return;
                next(warning);
            },
        });
        await bundle.write({
            file: path.join("dist", name, "index.js"),
            format: "esm",
            paths: { "@zeroc/ice": "/ice.js" },
        });
        await bundle.close();
    });

    gulp.task(testTask(name, "copy:assets"), () => {
        return gulp.src(["test/Common/controller.html", "test/Common/style.css"]).pipe(gulp.dest(`dist/${name}`));
    });

    gulp.task(testTask(name, "clean"), async () => {
        const toDelete = [];

        if (fs.existsSync(testDir)) {
            for (const file of fs.readdirSync(testDir)) {
                // Delete generated .js and .d.ts from .ice sources.
                if (file.endsWith(".ice")) {
                    const baseName = path.basename(file, ".ice");
                    toDelete.push(path.join(testDir, `${baseName}.js`));
                    toDelete.push(path.join(testDir, `${baseName}.d.ts`));
                }
                // Delete compiled .js from .ts sources.
                // Hand-written .js files (like index.js) won't match since they have no .ts counterpart.
                if (file.endsWith(".ts") && !file.endsWith(".d.ts")) {
                    const baseName = path.basename(file, ".ts");
                    toDelete.push(path.join(testDir, `${baseName}.js`));
                }
            }
        }

        await deleteAsync(toDelete, { force: true });
    });
}

gulp.task(
    "test",
    gulp.series(
        "ice:bundle",
        "test:common:build",
        gulp.series(tests.map(testName => testTask(testName, "build"))),
        gulp.series(tests.map(testName => testTask(testName, "copy:assets"))),
    ),
);

gulp.task("test:bundle:clean", () => deleteAsync("dist"));

gulp.task(
    "test:clean",
    gulp.series(
        "test:common:clean",
        "test:bundle:clean",
        tests.map(testName => testTask(testName, "clean")),
    ),
);

gulp.task("test:unplugin", () => {
    return new Promise((resolve, reject) => {
        exec("node --test test/Slice/unplugin/*/*.test.js", { cwd: root }, (err, stdout, stderr) => {
            process.stdout.write(stdout);
            process.stderr.write(stderr);
            if (err) reject(err);
            else resolve();
        });
    });
});

gulp.task("build", gulp.series("dist", "test", "test:unplugin"));
gulp.task("clean", gulp.series("dist:clean", "test:clean"));
gulp.task("default", gulp.series("build"));
