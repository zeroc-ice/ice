// Copyright (c) ZeroC, Inc.

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
import strip from "@rollup/plugin-strip";
import resolve from "@rollup/plugin-node-resolve";
import { fileURLToPath } from "url";
import tsc from "gulp-typescript";

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

function slice2js(options) {
    const defaults = {};
    const opts = options || {};
    if (process.platform == "win32") {
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
        defaults.iceToolsPath = path.resolve("../cpp/bin", platform, configuration);
    }
    defaults.iceHome = path.resolve(__dirname, "..");
    defaults.include = opts.include || [];
    defaults.args = opts.args || [];
    defaults.jsbundle = false;
    defaults.tsbundle = false;
    return iceBuilder(defaults);
}

const root = path.resolve(__dirname);
const libs = ["Ice", "Glacier2", "IceStorm", "IceGrid"];

const libTask = (libName, taskName) => libName + ":" + taskName;

const excludes = {
    Ice: ["!../slice/Ice/Identity.ice", "!../slice/Ice/Version.ice"],
};

function createCleanTask(taskName, patterns, extension, dest = undefined) {
    gulp.task(taskName, () => {
        const pumpArgs = [gulp.src(patterns), extReplace(extension)];

        if (dest !== undefined) {
            pumpArgs.push(gulp.dest(dest));
        }

        pumpArgs.push(vinylPaths(deleteAsync));

        return new Promise((resolve, reject) => {
            pump(pumpArgs, err => (err ? reject(err) : resolve()));
        });
    });
}

for (const lib of libs) {
    const slicePatterns = [`../slice/${lib}/*.ice`, ...(excludes[lib] || [])];
    gulp.task(libTask(lib, "generate"), cb => {
        pump(
            [
                gulp.src(slicePatterns),
                slice2js({
                    args: ["--typescript"],
                }),
                gulp.dest(`${root}/src/${lib}`),
            ],
            cb,
        );
    });

    createCleanTask(libTask(lib, "clean:js"), slicePatterns, ".js", `${root}/src/${lib}`);
    createCleanTask(libTask(lib, "clean:d.ts"), slicePatterns, ".d.ts", `${root}/src/${lib}`);

    gulp.task(libTask(lib, "clean"), gulp.series(libTask(lib, "clean:js"), libTask(lib, "clean:d.ts")));
}

gulp.task("dist", gulp.parallel(libs.map(libName => libTask(libName, "generate"))));

gulp.task("dist:clean", gulp.parallel(libs.map(libName => libTask(libName, "clean"))));

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
];

gulp.task("test:common:generate", cb => {
    pump([gulp.src(["../scripts/Controller.ice"]), slice2js(), gulp.dest("test/Common")], cb);
});

gulp.task("ice:bundle", async () => {
    // Tell rollup to use the browser version of Ice.
    const plugins = [resolve({ browser: true })];
    if (optimize) {
        plugins.push(
            strip({
                labels: ["DEV"],
            }),
        );
    }

    let bundle = await rollup({
        input: "src/index.js",
        plugins: plugins,
    });
    bundle.write({
        file: "dist/ice.js",
        format: "esm",
    });
});

// A rollup resolver plugin to resolve "@zeroc/ice" module as an external file.
function IceResolver() {
    return {
        name: "ice-resolver",
        async resolveId(source) {
            if (source == "@zeroc/ice") {
                return { id: "/ice.js", external: "absolute" };
            }
            return null;
        },
    };
}

gulp.task("test:common:bundle", async () => {
    let bundle = await rollup({
        input: ["test/Common/ControllerI.js", "test/Common/ControllerWorker.js"],
        plugins: [IceResolver()],
    });
    bundle.write({
        format: "esm",
        output: {
            dir: "dist/test/Common/",
        },
    });
});

gulp.task("test:common:clean", cb => {
    deleteAsync(["test/Common/Controller.js", "test/Common/.depend"]);
    cb();
});

const testTask = (testName, taskName) => testName.replace(/\//g, "_") + ":" + taskName;

for (const name of tests) {
    gulp.task(testTask(name, "build"), cb => {
        const outputDirectory = `${root}/${name}`;
        pump(
            [
                gulp.src(`${outputDirectory}/*.ice`),
                slice2js({
                    include: [outputDirectory],
                    args: ["--typescript"],
                }),
                gulp.dest(outputDirectory),
            ],
            cb,
        );
    });

    gulp.task(testTask(name, "ts-compile"), cb => {
        pump(
            [
                gulp.src([`${root}/${name}/*.ts`, `!${root}/${name}/*.d.ts`]),
                tsc({
                    lib: ["dom", "es2020"],
                    target: "es2020",
                    module: "es2020",
                    noImplicitAny: true,
                    moduleResolution: "bundler",
                }),
                gulp.dest(`${root}/${name}`),
            ],
            cb,
        );
    });

    gulp.task(testTask(name, "bundle"), async () => {
        let input = fs.existsSync(`${name}/index.js`) ? `${name}/index.js` : `${name}/Client.js`;

        let bundle = await rollup({
            input: input,
            plugins: [IceResolver()],
            onwarn: (warning, next) => {
                // Ignore the "this is undefined" warning, let rollup silently rewrite it.
                // This avoids warnings from the TypeScript polyfills for async disposable resources.
                if (warning.code === "THIS_IS_UNDEFINED") return;

                // Ignore unresolved fs/path dynamic imports (Node-only usage)
                if (warning.code === "UNRESOLVED_IMPORT" && ["fs", "path"].includes(warning.source)) {
                    return;
                }
                next(warning);
            },
        });
        await bundle.write({
            file: path.join("dist", name, "index.js"),
            format: "esm",
        });
    });

    gulp.task(testTask(name, "copy:assets"), async cb => {
        pump([gulp.src(["test/Common/controller.html", "test/Common/style.css"]), gulp.dest(`dist/${name}`)], cb);
    });

    createCleanTask(testTask(name, "clean:js"), [`${name}/*.ice`], ".js");
    createCleanTask(testTask(name, "clean:d.ts"), [`${name}/*.ice`], ".d.ts");
    createCleanTask(testTask(name, "clean:ts"), [`${name}/*.ts`], ".js");

    gulp.task(
        testTask(name, "clean"),
        gulp.series(testTask(name, "clean:js"), testTask(name, "clean:d.ts"), testTask(name, "clean:ts")),
    );
}

gulp.task(
    "test",
    gulp.series(
        "ice:bundle",
        "test:common:generate",
        "test:common:bundle",
        gulp.series(tests.map(testName => testTask(testName, "build"))),
        gulp.series(tests.map(testName => testTask(testName, "ts-compile"))),
        gulp.series(tests.map(testName => testTask(testName, "bundle"))),
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

gulp.task("build", gulp.series("dist", "test"));
gulp.task("clean", gulp.series("dist:clean", "test:clean"));
gulp.task("default", gulp.series("build"));
