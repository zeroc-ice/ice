//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

/* eslint no-sync: "off" */
/* eslint no-process-env: "off" */
/* eslint no-process-exit: "off" */

import { deleteAsync } from "del";
import vinylPaths from "vinyl-paths";
import extReplace from "gulp-ext-replace";
import fs from "fs";
import gulp from "gulp";
import iceBuilder from "gulp-ice-builder";
import path from "path";
import pump from "pump";
import { rollup } from "rollup";
import { fileURLToPath } from "url";
import ts from "typescript";
import tsc from "gulp-typescript";

const __dirname = path.dirname(fileURLToPath(import.meta.url));

const sliceDir = path.resolve(__dirname, "..", "slice");

const iceBinDist = (process.env.ICE_BIN_DIST || "").split(" ");
const useBinDist = iceBinDist.find((v) => v == "js" || v == "all") !== undefined;

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

const platform = parseArg(process.argv, "--cppPlatform") || process.env.CPP_PLATFORM;
const configuration = parseArg(process.argv, "--cppConfiguration") || process.env.CPP_CONFIGURATION;

function slice2js(options) {
    const defaults = {};
    const opts = options || {};
    if (!useBinDist) {
        if (process.platform == "win32") {
            if (!platform || (platform.toLowerCase() != "win32" && platform.toLowerCase() != "x64")) {
                console.log("Error: --cppPlatform must be set to `Win32' or `x64', in order to locate slice2js.exe");
                process.exit(1);
            }

            if (
                !configuration ||
                (configuration.toLowerCase() != "debug" && configuration.toLowerCase() != "release")
            ) {
                console.log(
                    "Error: --cppConfiguration must be set to `Debug' or `Release', in order to locate slice2js.exe",
                );
                process.exit(1);
            }
            defaults.iceToolsPath = path.resolve("../cpp/bin", platform, configuration);
        }
        defaults.iceHome = path.resolve(__dirname, "..");
    } else if (process.env.ICE_HOME) {
        defaults.iceHome = process.env.ICE_HOME;
    }
    defaults.include = opts.include || [];
    defaults.args = opts.args || [];
    defaults.jsbundle = opts.jsbundle;
    defaults.tsbundle = opts.tsbundle;
    defaults.jsbundleFormat = opts.jsbundleFormat;
    return iceBuilder(defaults);
}

//
// Tasks to build IceJS Distribution
//
const root = path.resolve(__dirname);
const libs = ["Ice", "Glacier2", "IceStorm", "IceGrid"];

const generateTask = (name) => name.toLowerCase() + ":generate";

const srcDir = (name) => path.join(root, "src", name);
const libCleanTask = (lib) => lib + ":clean";

function libFiles(name) {
    return [path.join(root, "lib", name + ".js")];
}

function libGeneratedFiles(lib, sources) {
    const tsSliceSources = sources.typescriptSlice || sources.slice;

    return sources.slice
        .map((f) => path.join(srcDir(lib), path.basename(f, ".ice") + ".js"))
        .concat(tsSliceSources.map((f) => path.join(srcDir(lib), path.basename(f, ".ice") + ".d.ts")))
        .concat(libFiles(lib))
        .concat([path.join(srcDir(lib), ".depend", "*")]);
}

const sliceFile = (f) => path.join(sliceDir, f);

for (const lib of libs) {
    const sources = JSON.parse(
        fs.readFileSync(path.join(srcDir(lib), "sources.json"), {
            encoding: "utf8",
        }),
    );

    gulp.task(generateTask(lib), (cb) => {
        pump(
            [
                gulp.src(sources.slice.map(sliceFile)),
                slice2js({
                    jsbundle: false,
                    tsbundle: false,
                    args: ["--typescript"],
                }),
                gulp.dest(srcDir(lib)),
            ],
            cb,
        );
    });

    gulp.task(libCleanTask(lib), (cb) => {
        deleteAsync(libGeneratedFiles(lib, sources));
        cb();
    });
}

if (useBinDist) {
    gulp.task("ice:module", (cb) => cb());
    gulp.task("ice:module:clean", (cb) => cb());
    gulp.task("dist", (cb) => cb());
    gulp.task("dist:clean", (cb) => cb());
} else {
    gulp.task("dist", gulp.parallel(libs.map(generateTask)));

    gulp.task("dist:clean", gulp.parallel(libs.map(libCleanTask)));

    gulp.task("ice:module:package", () => gulp.src(["package.json"]).pipe(gulp.dest(path.join("node_modules", "ice"))));

    gulp.task(
        "ice:module",
        gulp.series("ice:module:package", (cb) => {
            pump(
                [gulp.src([path.join(root, "src/**/*")]), gulp.dest(path.join(root, "node_modules", "ice", "src"))],
                cb,
            );
        }),
    );

    gulp.task("ice:module:clean", () => deleteAsync("node_modules/ice"));
}

const tests = [
    "test/Ice/adapterDeactivation",
    "test/Ice/ami",
    "test/Ice/binding",
    "test/Ice/defaultValue",
    "test/Ice/enums",
    "test/Ice/exceptions",
    "test/Ice/facets",
    "test/Ice/hold",
    "test/Ice/idleTimeout",
    "test/Ice/info",
    "test/Ice/inheritance",
    "test/Ice/location",
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
    "test/Ice/number",
    "test/Ice/scope",
    "test/Glacier2/router",
    "test/Slice/escape",
    "test/Slice/macros",
];

gulp.task("test:common:generate", (cb) => {
    pump([gulp.src(["../scripts/Controller.ice"]), slice2js(), gulp.dest("test/Common")], cb);
});

// A rollup plug-in to resolve "net" module as a mockup file.

const nodeMokups = ["net", "fs", "path"];
function NodeMookupResolver() {
    return {
        name: "mookup-resolver",
        async resolveId(source) {
            if (nodeMokups.includes(source)) {
                return path.join(__dirname, `test/Common/${source}.mockup.js`);
            }
            return null;
        },
    };
}

gulp.task("ice:bundle", async (cb) => {
    let bundle = await rollup({
        input: "src/index.js",
        plugins: [NodeMookupResolver()],
    });
    return bundle.write({
        file: "dist/ice.js",
        format: "esm",
    });
});

// A rollup resolver plugin to resolve "ice" module as an external file.
function IceResolver() {
    return {
        name: "ice-resolver",
        async resolveId(source) {
            if (source == "ice") {
                return { id: "/ice.js", external: "absolute" };
            }
            return null;
        },
    };
}

gulp.task("test:common:bundle", async (cb) => {
    let bundle = await rollup({
        input: ["test/Common/ControllerI.js", "test/Common/ControllerWorker.js"],
        plugins: [IceResolver()],
    });
    return bundle.write({
        format: "esm",
        output: {
            dir: "dist/test/Common/",
        },
    });
});

gulp.task("test:common:clean", (cb) => {
    deleteAsync(["test/Common/Controller.js", "test/Common/.depend"]);
    cb();
});

const testTask = (name) => name.replace(/\//g, "_");
const testCleanTask = (name) => testTask(name) + ":clean";
const testBuildTask = (name) => testTask(name) + ":build";
const testTypeScriptCompileTask = (name) => testTask(name) + ":ts-compile";
const testBundleTask = (name) => testTask(name) + ":bundle";
const testAssetsTask = (name) => testTask(name) + ":assets";

for (const name of tests) {
    gulp.task(testBuildTask(name), (cb) => {
        const outputDirectory = path.join(root, name);
        pump(
            [
                gulp.src(path.join(outputDirectory, "*.ice")),
                slice2js({
                    include: [outputDirectory],
                    args: ["--typescript"],
                    jsbundle: false,
                    tsbundle: false,
                }),
                gulp.dest(outputDirectory),
            ],
            cb,
        );
    });

    gulp.task(testTypeScriptCompileTask(name), (cb) => {
        const baseName = path.join(root, name);
        pump(
            [
                gulp.src([`${baseName}/*.ts`, `!${baseName}/*.d.ts`]),
                tsc({
                    lib: ["dom", "es2020"],
                    target: "es2020",
                    module: "es2020",
                    noImplicitAny: true,
                    moduleResolution: "node",
                }),
                gulp.dest(path.join(root, name)),
            ],
            cb,
        );
    });

    gulp.task(testBundleTask(name), async (cb) => {
        let input = fs.existsSync(path.join(name, "index.js"))
            ? path.join(name, "index.js")
            : path.join(name, "Client.js");

        let bundle = await rollup({
            input: input,
            plugins: [IceResolver(), NodeMookupResolver()],
        });
        await bundle.write({
            file: path.join("dist", name, "index.js"),
            format: "esm",
        });
    });

    gulp.task(testAssetsTask(name), async (cb) => {
        pump([gulp.src("test/Common/controller.html"), gulp.dest(path.join("dist", name))], cb);
    });

    gulp.task(testCleanTask(name), (cb) =>
        pump(
            [
                gulp.src(path.join(name, "*.ice")),
                extReplace(".js"),
                gulp.src(path.join(name, ".depend"), { allowEmpty: true }),
                vinylPaths(deleteAsync),
            ],
            cb(),
        ),
    );
}

gulp.task(
    "test",
    gulp.series(
        "ice:bundle",
        "test:common:generate",
        "test:common:bundle",
        gulp.series(tests.map(testBuildTask)),
        gulp.series(tests.map(testTypeScriptCompileTask)),
        gulp.series(tests.map(testBundleTask)),
        gulp.series(tests.map(testAssetsTask)),
    ),
);

gulp.task("test:clean", gulp.parallel("test:common:clean", tests.map(testCleanTask)));

gulp.task("build", gulp.series("dist", "ice:module", "test"));
gulp.task("clean", gulp.series("dist:clean", "ice:module:clean", "test:clean"));
gulp.task("default", gulp.series("build"));
