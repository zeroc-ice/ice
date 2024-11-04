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
import tsc from "gulp-typescript";

const __dirname = path.dirname(fileURLToPath(import.meta.url));

const iceBinDist = (process.env.ICE_BIN_DIST || "").split(" ");
const useBinDist = iceBinDist.find(v => v == "js" || v == "all") !== undefined;

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

const root = path.resolve(__dirname);
const libs = ["Ice", "Glacier2", "IceStorm", "IceGrid"];

const libTask = (libName, taskName) => libName + ":" + taskName;

const excludes = {
    Ice: ["!../slice/Ice/Identity.ice", "!../slice/Ice/Version.ice"],
};

function createCleanTask(taskName, patterns, extension, dest = undefined) {
    gulp.task(taskName, async cb => {
        const pumpArgs = [gulp.src(patterns), extReplace(extension)];
        if (dest !== undefined) {
            pumpArgs.push(gulp.dest(dest));
        }
        pumpArgs.push(vinylPaths(deleteAsync));
        return new Promise((resolve, reject) => {
            pump(pumpArgs, err => {
                if (err) {
                    reject(err);
                } else {
                    resolve();
                }
            });
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
                    jsbundle: false,
                    tsbundle: false,
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

if (useBinDist) {
    gulp.task("ice:module", cb => cb());
    gulp.task("ice:module:clean", cb => cb());
    gulp.task("dist", cb => cb());
    gulp.task("dist:clean", cb => cb());
} else {
    gulp.task("dist", gulp.parallel(libs.map(libName => libTask(libName, "generate"))));

    gulp.task("dist:clean", gulp.parallel(libs.map(libName => libTask(libName, "clean"))));

    gulp.task("ice:module:package", () => gulp.src(["package.json"]).pipe(gulp.dest("node_modules/ice")));

    gulp.task(
        "ice:module",
        gulp.series("ice:module:package", cb => {
            pump([gulp.src([`${root}/src/**/*`]), gulp.dest(`${root}/node_modules/ice/src`)], cb);
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
    "test/Ice/number",
    "test/Ice/scope",
    "test/Glacier2/router",
    "test/Slice/escape",
    "test/Slice/macros",
];

gulp.task("test:common:generate", cb => {
    pump([gulp.src(["../scripts/Controller.ice"]), slice2js(), gulp.dest("test/Common")], cb);
});

// A rollup plug-in to resolve "net" module as a mockup file.

const nodeMockups = ["net", "fs", "path"];
function NodeMockupResolver() {
    return {
        name: "mockup-resolver",
        async resolveId(source) {
            if (nodeMockups.includes(source)) {
                return `${__dirname}/test/Common/${source}.mockup.js`;
            }
            return null;
        },
    };
}

gulp.task("ice:bundle", cb => {
    return new Promise(async (resolve, reject) => {
        try {
            let bundle = await rollup({
                input: "src/index.js",
                plugins: [NodeMockupResolver()],
            });
            bundle.write({
                file: "dist/ice.js",
                format: "esm",
            });
            resolve();
        } catch (e) {
            reject(e);
        }
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

gulp.task("test:common:bundle", cb => {
    return new Promise(async (resolve, reject) => {
        try {
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
            resolve();
        } catch (e) {
            reject(e);
        }
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
                    jsbundle: false,
                    tsbundle: false,
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
                    moduleResolution: "node",
                }),
                gulp.dest(`${root}/${name}`),
            ],
            cb,
        );
    });

    gulp.task(testTask(name, "bundle"), async cb => {
        let input = fs.existsSync(`${name}/index.js`) ? `${name}/index.js` : `${name}/Client.js`;

        let bundle = await rollup({
            input: input,
            plugins: [IceResolver(), NodeMockupResolver()],
        });
        await bundle.write({
            file: path.join("dist", name, "index.js"),
            format: "esm",
        });
    });

    gulp.task(testTask(name, "copy:assets"), async cb => {
        pump([gulp.src("test/Common/controller.html"), gulp.dest(`dist/${name}`)], cb);
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

gulp.task("build", gulp.series("dist", "ice:module", "test"));
gulp.task("clean", gulp.series("dist:clean", "ice:module:clean", "test:clean"));
gulp.task("default", gulp.series("build"));
