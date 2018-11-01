// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

/* eslint no-sync: "off" */
/* eslint no-process-env: "off" */
/* eslint no-process-exit: "off" */

const babel = require("gulp-babel");
const bundle = require("./gulp/bundle");
const tsbundle = require("./gulp/ts-bundle");
const concat = require('gulp-concat');
const del = require("del");
const extreplace = require("gulp-ext-replace");
const fs = require("fs");
const gulp = require("gulp");
const gzip = require('gulp-gzip');
const iceBuilder = require('gulp-ice-builder');
const newer = require('gulp-newer');
const path = require('path');
const paths = require('vinyl-paths');
const pump = require('pump');
const rollup = require('rollup');
const sourcemaps = require('gulp-sourcemaps');
const terser = require('gulp-terser');
const tsc = require('gulp-typescript');
const tsformat = require('./gulp/ts-formatter');

const sliceDir = path.resolve(__dirname, '..', 'slice');

const iceBinDist = (process.env.ICE_BIN_DIST || "").split(" ");
const useBinDist = iceBinDist.find(v => v == "js" || v == "all") !== undefined;

function parseArg(argv, key)
{
    for(let i = 0; i < argv.length; ++i)
    {
        const e = argv[i];
        if(e == key)
        {
            return argv[i + 1];
        }
        else if(e.indexOf(key + "=") === 0)
        {
            return e.substr(key.length + 1);
        }
    }
}

const platform = parseArg(process.argv, "--cppPlatform") || process.env.CPP_PLATFORM;
const configuration = parseArg(process.argv, "--cppConfiguration") || process.env.CPP_CONFIGURATION;

function slice2js(options)
{
    const defaults = {};
    const opts = options || {};
    if(!useBinDist)
    {
        if(process.platform == "win32")
        {
            if(!platform || (platform.toLowerCase() != "win32" && platform.toLowerCase() != "x64"))
            {
                console.log("Error: --cppPlatform must be set to `Win32' or `x64', in order to locate slice2js.exe");
                process.exit(1);
            }

            if(!configuration || (configuration.toLowerCase() != "debug" && configuration.toLowerCase() != "release"))
            {
                console.log("Error: --cppConfiguration must be set to `Debug' or `Release', in order to locate slice2js.exe");
                process.exit(1);
            }
            defaults.iceToolsPath = path.resolve("../cpp/bin", platform, configuration);
        }
        defaults.iceHome = path.resolve("..");
    }
    else if(process.env.ICE_HOME)
    {
        defaults.iceHome = process.env.ICE_HOME;
    }
    defaults.include = opts.include || [];
    defaults.args = opts.args || [];
    return iceBuilder.compile(defaults);
}

//
// Tasks to build IceJS Distribution
//
const root = path.resolve(path.join('__dirname', '..'));
const libs = ["Ice", "Glacier2", "IceStorm", "IceGrid"];

const generateTypeScriptTask = name => name.toLowerCase() + ":generate-typescript";
const generateTask = name => name.toLowerCase() + ":generate";
const libTask = name => name.toLowerCase() + ":lib";
const minLibTask = name => name.toLowerCase() + ":lib-min";
const babelTask = name => name.toLowerCase() + ":babel";
const babelLibTask = name => libTask(name) + "-babel";
const babelMinLibTask = name => libTask(name) + "-babel-min";
const libDistTask = name => name.toLowerCase() + ":dist";

const libFile = name => path.join(root, "lib", name + ".js");
const libFileMin = name => path.join(root, "lib", name + ".min.js");

const babelLibFile = name => path.join(root, "lib", "es5", name + ".js");
const babelLibFileMin = name => path.join(root, "lib", "es5", name + ".min.js");

const srcDir = name => path.join(root, "src", name);
const libCleanTask = lib => lib + ":clean";

function libFiles(name)
{
    return [
        path.join(root, "lib", name + ".js"),
        path.join(root, "lib", name + ".js.gz"),
        path.join(root, "lib", name + ".min.js"),
        path.join(root, "lib", name + ".min.js.gz"),

        path.join(root, "lib", "es5", name + ".js"),
        path.join(root, "lib", "es5", name + ".js.gz"),
        path.join(root, "lib", "es5", name + ".min.js"),
        path.join(root, "lib", "es5", name + ".min.js.gz")];
}

function mapFiles(name)
{
    return [
        path.join(root, "lib", name + ".js.map"),
        path.join(root, "lib", name + ".js.map.gz"),
        path.join(root, "lib", name + ".min.js.map"),
        path.join(root, "lib", name + ".min.js.map.gz"),

        path.join(root, "lib", "es5", name + ".js.map"),
        path.join(root, "lib", "es5", name + ".js.map.gz"),
        path.join(root, "lib", "es5", name + ".min.js.map"),
        path.join(root, "lib", "es5", name + ".min.js.map.gz")];
}

function libSources(lib, sources)
{
    let srcs = sources.common || [];
    if(sources.browser)
    {
        srcs = sources.common.concat(sources.browser);
    }

    srcs = srcs.map(f => path.join(srcDir(lib), f));

    if(sources.slice)
    {
        srcs = srcs.concat(sources.slice.map(f => path.join(srcDir(lib), path.basename(f, ".ice") + ".js")));
    }
    return srcs;
}

function libGeneratedFiles(lib, sources)
{
    const tsSliceSources = sources.typescriptSlice || sources.slice;

    return sources.slice.map(f => path.join(srcDir(lib), path.basename(f, ".ice") + ".js"))
        .concat(tsSliceSources.map(f => path.join(srcDir(lib), path.basename(f, ".ice") + ".d.ts")))
        .concat(path.join("src", "es5", lib, "*.js"))
        .concat(libFiles(lib))
        .concat(mapFiles(lib))
        .concat([path.join(srcDir(lib), ".depend", "*")]);
}

const sliceFile = f => path.join(sliceDir, f);

for(const lib of libs)
{
    const sources = JSON.parse(fs.readFileSync(path.join(srcDir(lib), "sources.json"), {encoding: "utf8"}));

    gulp.task(generateTypeScriptTask(lib),
              cb =>
              {
                  const sliceSources = sources.typescriptSlice || sources.slice;
                  pump([gulp.src(sliceSources.map(sliceFile)),
                        slice2js({dest: srcDir(lib), args: ["--typescript"]}),
                        gulp.dest(srcDir(lib))], cb);
              });

    gulp.task(generateTask(lib),
              cb =>
              {
                  pump([gulp.src(sources.slice.map(sliceFile)),
                        slice2js({dest: srcDir(lib)}),
                        gulp.dest(srcDir(lib))], cb);
              });

    gulp.task(libTask(lib),
              cb =>
              {
                  pump([gulp.src(libSources(lib, sources)),
                        sourcemaps.init(),
                        bundle(
                            {
                                srcDir: srcDir(lib),
                                modules: sources.modules,
                                target: libFile(lib)
                            }),
                        sourcemaps.write("../lib", {sourceRoot: "/src", addComment: false}),
                        gulp.dest("lib"),
                        gzip(),
                        gulp.dest("lib")], cb);
              });

    gulp.task(minLibTask(lib),
              cb =>
              {
                  pump([gulp.src(libFile(lib)),
                        newer(libFileMin(lib)),
                        sourcemaps.init({loadMaps: false}),
                        terser(),
                        extreplace(".min.js"),
                        sourcemaps.write(".", {includeContent: false, addComment: false}),
                        gulp.dest(path.join(root, "lib")),
                        gzip(),
                        gulp.dest(path.join(root, "lib"))], cb);
              });

    gulp.task(babelTask(lib),
              cb =>
              {
                  pump([gulp.src(path.join(root, "src", lib, "*.js")),
                        newer(path.join(root, "src", "es5", lib)),
                        babel({compact: false}),
                        gulp.dest(path.join(root, "src", "es5", lib))], cb);
              });

    gulp.task(babelLibTask(lib),
              cb =>
              {
                  pump([gulp.src(libFile(lib)),
                        newer(babelLibFile(lib)),
                        sourcemaps.init(),
                        babel({compact: false}),
                        sourcemaps.write("."),
                        gulp.dest(path.join(root, "lib", "es5")),
                        gzip(),
                        gulp.dest("lib/es5")], cb);
              });

    gulp.task(babelMinLibTask(lib),
              cb =>
              {
                  pump([gulp.src(babelLibFile(lib)),
                        newer(babelLibFileMin(lib)),
                        terser(),
                        extreplace(".min.js"),
                        sourcemaps.write(".", {includeContent: false, addComment: false}),
                        gulp.dest("lib/es5"),
                        gzip(),
                        gulp.dest("lib/es5")], cb);
              });

    gulp.task(libCleanTask(lib),
              cb =>
              {
                  del(libGeneratedFiles(lib, sources));
                  cb();
              });

    gulp.task(libDistTask(lib),
              gulp.series(
                  gulp.parallel(generateTypeScriptTask(lib), generateTask(lib)),
                  libTask(lib),
                  gulp.parallel(babelTask(lib), babelLibTask(lib), minLibTask(lib)),
                  babelMinLibTask(lib)));
}

gulp.task("ts:bundle",
          cb =>
          {
              pump([gulp.src("./src/index.d.ts"),
                    tsbundle(),
                    tsformat({}),
                    gulp.dest("lib")], cb);
          });

gulp.task("ts:bundle:clean", () => del("./lib/index.d.ts"));

if(useBinDist)
{
    gulp.task("ice:module", cb => cb());
    gulp.task("ice:module:clean", cb => cb());
    gulp.task("dist", cb => cb());
    gulp.task("dist:clean", cb => cb());
}
else
{
    gulp.task("dist", gulp.series(gulp.parallel(libs.map(libDistTask)), "ts:bundle"));

    gulp.task("dist:clean",
              gulp.series(gulp.parallel(libs.map(libCleanTask).concat("ts:bundle:clean")),
                          cb =>
                          {
                              const srcs = [gulp.src(path.join(root, "lib", "es5"), {allowEmpty: true})];
                              for(const lib of libs)
                              {
                                  srcs.push(gulp.src(path.join(root, "src", "es5", lib), {allowEmpty: true}));
                              }
                              pump([...srcs, paths(del)], cb);
                          }));

    gulp.task("ice:module:package",
              () => gulp.src(['package.json']).pipe(gulp.dest(path.join("node_modules", "ice"))));

    gulp.task("ice:module",
              gulp.series("ice:module:package",
                          cb =>
                          {
                              pump([
                                  gulp.src([path.join(root, 'src/**/*')]),
                                  gulp.dest(path.join(root, "node_modules", "ice", "src"))], cb);
                          }));

    gulp.task("ice:module:clean", () => gulp.src(['node_modules/ice'], {allowEmpty: true}).pipe(paths(del)));
}

const tests = [
    "test/Ice/acm",
    "test/Ice/adapterDeactivation",
    "test/Ice/ami",
    "test/Ice/binding",
    "test/Ice/defaultValue",
    "test/Ice/enums",
    "test/Ice/exceptions",
    "test/Ice/facets",
    "test/Ice/hold",
    "test/Ice/info",
    "test/Ice/inheritance",
    "test/Ice/location",
    "test/Ice/objects",
    "test/Ice/operations",
    "test/Ice/optional",
    "test/Ice/promise",
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
    "test/Slice/macros"
];

gulp.task("test:common:generate",
          cb =>
          {
              pump([gulp.src(["../scripts/Controller.ice"]),
                    slice2js({dest: "test/Common"}),
                    gulp.dest("test/Common")], cb);
          });

gulp.task("test:common:babel",
          cb =>
          {
              pump([gulp.src(["test/Common/Controller.js",
                              "test/Common/ControllerI.js",
                              "test/Common/ControllerWorker.js",
                              "test/Common/TestHelper.js",
                              "test/Common/run.js"]),
                    babel({compact: false}),
                    gulp.dest("test/es5/Common")], cb);
          });

gulp.task("test:common:controllerworker",
          cb =>
          {
              pump([gulp.src(["node_modules/@babel/polyfill/dist/polyfill.js",
                              "test/es5/Common/ControllerWorker.js"]),
                    concat("ControllerWorker.js"),
                    gulp.dest("test/es5/Common/")], cb);
          });

gulp.task("test:common:clean",
          cb =>
          {
              del(["test/Common/Controller.js",
                   "test/Common/.depend",
                   "test/es5/Common/Controller.js",
                   "test/es5/Common/ControllerI.js",
                   "test/es5/Common/ControllerWorker.js",
                   "test/es5/Common/TestHelper.js"]);
              cb();
          });

gulp.task("test:import:generate",
          cb =>
          {
              pump([gulp.src(["test/Ice/import/Demo/Point.ice",
                              "test/Ice/import/Demo/Circle.ice",
                              "test/Ice/import/Demo/Square.ice",
                              "test/Ice/import/Demo/Canvas.ice"]),
                    slice2js({dest: "test/Ice/import/Demo", include: ["test/Ice/import"]}),
                    gulp.dest("test/Ice/import/Demo")], cb);
          });

gulp.task("test:import:bundle",
          () =>
          {
              const p = rollup.rollup(
                  {
                      input: "test/Ice/import/main.js",
                      external: ["ice"]
                  }).then(bundle => bundle.write(
                      {
                          file: "test/Ice/import/bundle.js",
                          format: "cjs"
                      }));
              return p;
          });

gulp.task("test:import:clean",
          cb =>
          {
              del(["test/Ice/import/Demo/Point.js",
                   "test/Ice/import/Demo/Circle.js",
                   "test/Ice/import/Demo/Square.js",
                   "test/Ice/import/Demo/Canvas.js",
                   "test/Ice/import/bundle.js"]);
              cb();
          });

const testTask = name => name.replace(/\//g, "_");
const testBabelTask = name => testTask(name) + ":babel";
const testCleanTask = name => testTask(name) + ":clean";
const testBuildTask = name => testTask(name) + ":build";

for(const name of tests)
{
    gulp.task(testTask(name),
              cb =>
              {
                  const outdir = path.join(root, name);
                  pump([gulp.src(path.join(outdir, "*.ice")),
                        slice2js({include: [outdir], dest: outdir}),
                        gulp.dest(outdir)], cb);
              });

    gulp.task(testBabelTask(name),
              cb =>
              {
                  const outdir = path.join(root, name.replace("test/", "test/es5/"));
                  pump([gulp.src([path.join(root, name, "*.js")]),
                        babel({compact: false}),
                        gulp.dest(outdir)], cb);
              });

    gulp.task(testBuildTask(name), gulp.series(testTask(name), testBabelTask(name)));

    gulp.task(testCleanTask(name),
              cb =>
              {
                  pump([gulp.src(path.join(name, "*.ice")),
                        extreplace(".js"),
                        gulp.src(path.join(name, ".depend"), {allowEmpty: true}),
                        gulp.src([path.join(name.replace("test/", "test/es5/"), "*.js")]),
                        paths(del)], cb);
              });
}

gulp.task("test",
          gulp.series("test:common:generate", "test:common:babel", "test:common:controllerworker",
                      "test:import:generate", "test:import:bundle",
                      gulp.parallel(tests.map(testBuildTask))));

gulp.task("test:clean",
          gulp.series(
              gulp.parallel("test:common:clean", "test:import:clean", tests.map(testCleanTask)),
              cb =>
              {
                  pump([gulp.src(path.join(root, "test", "es5"), {allowEmpty: true}),
                        paths(del)], cb);
              }));

//
// TypeScript tests
//
const tstests = [
    "test/ts/Ice/acm",
    "test/ts/Ice/adapterDeactivation",
    "test/ts/Ice/ami",
    "test/ts/Ice/binding",
    "test/ts/Ice/defaultValue",
    "test/ts/Ice/enums",
    "test/ts/Ice/exceptions",
    "test/ts/Ice/facets",
    "test/ts/Ice/hold",
    "test/ts/Ice/info",
    "test/ts/Ice/inheritance",
    "test/ts/Ice/location",
    "test/ts/Ice/number",
    "test/ts/Ice/objects",
    "test/ts/Ice/operations",
    "test/ts/Ice/optional",
    "test/ts/Ice/properties",
    "test/ts/Ice/proxy",
    "test/ts/Ice/retry",
    "test/ts/Ice/scope",
    "test/ts/Ice/servantLocator",
    "test/ts/Ice/slicing/exceptions",
    "test/ts/Ice/slicing/objects",
    "test/ts/Ice/stream",
    "test/ts/Ice/timeout",
    "test/ts/Glacier2/router",
    "test/ts/Slice/macros"
];

const testTypeScriptSliceCompileJsTask = name => testTask(name) + ":ts:slice-compile-js";
const testTypeScriptSliceCompileTsTask = name => testTask(name) + ":ts:slice-compile-ts";
const testTypeScriptCompileTask = name => testTask(name) + ":ts:compile";
const testTypeScriptBuildTask = name => testTask(name) + ":ts:build";
const testTypeScriptCleanTask = name => testTask(name) + ":ts:clean";

for(const name of tstests)
{
    gulp.task(testTypeScriptSliceCompileJsTask(name),
              cb =>
              {
                  const outdir = path.join(root, name);
                  pump([gulp.src(path.join(outdir, "*.ice")),
                        slice2js({include: [outdir], dest: outdir}),
                        gulp.dest(outdir)], cb);
              });

    gulp.task(testTypeScriptSliceCompileTsTask(name),
              cb =>
              {
                  const outdir = path.join(root, name);
                  pump([gulp.src(path.join(outdir, "*.ice")),
                        slice2js({include: [outdir], dest: outdir, args: ["--typescript"]}),
                        gulp.dest(outdir)], cb);
              });

    gulp.task(testTypeScriptCompileTask(name),
              cb =>
              {
                  pump([gulp.src(path.join(name, "*.ts")),
                        tsc(
                            {
                                lib: ["dom", "es2017"],
                                target: "es2017",
                                module: "commonjs",
                                noImplicitAny: true
                            }),
                        gulp.dest(name)
                       ], cb);
              });

    gulp.task(testTypeScriptBuildTask(name),
              gulp.series(
                  gulp.parallel(testTypeScriptSliceCompileJsTask(name), testTypeScriptSliceCompileTsTask(name)),
                  testTypeScriptCompileTask(name)));

    gulp.task(testTypeScriptCleanTask(name),
              cb =>
              {
                  pump([gulp.src([path.join(name, "**/*.js"),
                                  path.join(name, "**/*.d.ts")]),
                        paths(del)], cb);
              });
}

gulp.task("test:ts", gulp.series(tstests.map(testTypeScriptBuildTask)));
gulp.task("test:ts:clean", gulp.parallel(tstests.map(testTypeScriptCleanTask)));

gulp.task("build", gulp.series("dist", "ice:module", "test", "test:ts"));
gulp.task("clean", gulp.series("dist:clean", "ice:module:clean", "test:clean", "test:ts:clean"));
gulp.task("default", gulp.series("build"));
