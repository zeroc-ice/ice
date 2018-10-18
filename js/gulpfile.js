// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var babel       = require("gulp-babel"),
    bundle      = require("./gulp/bundle"),
    tsbundle    = require("./gulp/ts-bundle"),
    concat      = require('gulp-concat'),
    del         = require("del"),
    extreplace  = require("gulp-ext-replace"),
    fs          = require("fs"),
    gulp        = require("gulp"),
    gzip        = require('gulp-gzip'),
    iceBuilder  = require('gulp-ice-builder'),
    newer       = require('gulp-newer'),
    path        = require('path'),
    paths       = require('vinyl-paths'),
    pump        = require('pump'),
    rollup      = require('gulp-rollup'),
    sourcemaps  = require('gulp-sourcemaps'),
    terser      = require('gulp-terser');
    tsc         = require('gulp-typescript'),
    tsformat    = require('./gulp/ts-formatter');

var sliceDir   = path.resolve(__dirname, '..', 'slice');

var iceBinDist = (process.env.ICE_BIN_DIST || "").split(" ");
var useBinDist = iceBinDist.find(function(v) { return v == "js" || v == "all"; }) !== undefined;

function parseArg(argv, key)
{
    for(var i = 0; i < argv.length; ++i)
    {
        var e = argv[i];
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

var platform = parseArg(process.argv, "--cppPlatform") || process.env.CPP_PLATFORM;
var configuration = parseArg(process.argv, "--cppConfiguration") || process.env.CPP_CONFIGURATION;
var host = parseArg(process.argv, "--host") || "127.0.0.1";

function slice2js(options)
{
    var defaults = {};
    var opts = options || {};
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
// Test tasks
//
var tests = [
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

gulp.task("common:slice", [],
    function(cb){
        pump([
            gulp.src(["../scripts/Controller.ice"]),
            slice2js({dest: "test/Common"}),
            gulp.dest("test/Common")], cb);
    });

gulp.task("common:slice-babel", ["common:slice"],
    function(cb){
        pump([
            gulp.src(["test/Common/Controller.js",
                      "test/Common/ControllerI.js",
                      "test/Common/ControllerWorker.js",
                      "test/Common/TestHelper.js",
                      "test/Common/run.js"]),
            babel({compact: false}),
            gulp.dest("test/es5/Common")], cb);
    });

gulp.task("common:slice-es5-controllerworker", ["common:slice-babel"],
          function(cb){
              pump([
                  gulp.src(["node_modules/@babel/polyfill/dist/polyfill.js",
                            "test/es5/Common/ControllerWorker.js"]),
                  concat("ControllerWorker.js"),
                  gulp.dest("test/es5/Common/")
              ], cb);
          });

gulp.task("common:clean", [],
    function(){
        del(["test/Common/Controller.js",
             "test/Common/.depend",
             "test/es5/Common/Controller.js",
             "test/es5/Common/ControllerI.js",
             "test/es5/Common/ControllerWorker.js",
             "test/es5/Common/TestHelper.js"]);
    });

gulp.task("import:slice2js", ["dist"],
    function(cb){
        pump([
            gulp.src(["test/Ice/import/Demo/Point.ice",
                      "test/Ice/import/Demo/Circle.ice",
                      "test/Ice/import/Demo/Square.ice",
                      "test/Ice/import/Demo/Canvas.ice"]),
            slice2js({ dest: "test/Ice/import/Demo", include:["test/Ice/import"]}),
            gulp.dest("test/Ice/import/Demo")], cb);
    });

gulp.task("import:bundle", ["import:slice2js"],
          function(cb){
              pump([
                  rollup({
                      input: "test/Ice/import/main.js",
                      external: "ice",
                      format: "cjs",
                  }),
                  gulp.dest("test/Ice/import/bundle.js")], cb);

          });

gulp.task("import:clean", [],
    function() {
        del(["test/Ice/import/Demo/Point.js",
             "test/Ice/import/Demo/Circle.js",
             "test/Ice/import/Demo/Square.js",
             "test/Ice/import/Demo/Canvas.js",
             "test/Ice/import/bundle.js"]);
    });

function testTask(name) { return name.replace(/\//g, "_"); }
function testBabelTask(name) { return testTask(name) + "-babel"; }
function testCleanDependTask(name) { return testTask(name) + "-depend:clean"; }
function testCleanTask(name) { return testTask(name) + ":clean"; }
function testBabelCleanTask(name) { return testCleanTask(name) + "-babel"; }

tests.forEach(
    function(name){
        gulp.task(testTask(name), (useBinDist ? [] : ["ice-module"]),
            function(cb){
                pump([
                    gulp.src(path.join(name, "*.ice")),
                    slice2js({ include: [name], dest: name }),
                    gulp.dest(name)], cb);
            });

        gulp.task(testBabelTask(name), [testTask(name)],
            function(cb){
                pump([
                    gulp.src([path.join(name, "*.js")]),
                    babel({compact: false}),
                    gulp.dest(name.replace("test/", "test/es5/"))], cb);
            });

        gulp.task(testCleanDependTask(name), [],
            function(cb){
                pump([gulp.src(path.join(name, ".depend")), paths(del)], cb);
            });

        gulp.task(testCleanTask(name), [testCleanDependTask(name)],
            function(cb){
                pump([gulp.src(path.join(name, "*.ice")), extreplace(".js"), paths(del)], cb);
            });

        gulp.task(testBabelCleanTask(name), [testCleanTask(name)],
            function(cb){
                pump([gulp.src([path.join(name.replace("test/", "test/es5/"), "*.js")]), paths(del)], cb);
            });
    });

gulp.task("test", tests.map(testBabelTask).concat(
    ["common:slice-es5-controllerworker", "import:bundle"]));

gulp.task("test:clean", tests.map(testBabelCleanTask).concat(["common:clean", "import:clean"]));

// TypeScript tests
var tstests = [
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

function testTypeScriptSliceCompileJsTask(name) { return testTask(name) + ":ts:slice-compile-js"; }
function testTypeScriptSliceCompileTsTask(name) { return testTask(name) + ":ts:slice-compile-ts"; }
function testTypeScriptCompileTask(name) { return testTask(name) + ":ts:compile"; }

function testTypeScriptSliceJsCleanTask(name) { return testTask(name) + ":ts:slice:js-clean"; }
function testTypeScriptSliceTsCleanTask(name) { return testTask(name) + ":ts:slice:ts-clean"; }
function testTypeScriptCleanTask(name) { return testTask(name) + ":ts:clean"; }

tstests.forEach((name) =>
                {
                    gulp.task(testTypeScriptSliceCompileJsTask(name), [],
                              function(cb){
                                  pump([
                                      gulp.src(path.join(name, "*.ice")),
                                      slice2js({include:[name], dest: name}),
                                      gulp.dest(name)
                                  ], cb);
                              });

                    gulp.task(testTypeScriptSliceCompileTsTask(name), [],
                              function(cb){
                                  pump([
                                      gulp.src(path.join(name, "*.ice")),
                                      slice2js({include:[name], dest:name, args: ["--typescript"]}),
                                      gulp.dest(name)
                                  ], cb);
                              });

                    gulp.task(testTypeScriptCompileTask(name),
                              [
                                testTypeScriptSliceCompileJsTask(name),
                                testTypeScriptSliceCompileTsTask(name),
                              ].concat(useBinDist ? [] : ["ice-module"]),
                              function(cb){
                                  pump([
                                      gulp.src(path.join(name, "*.ts")),
                                      tsc(
                                          {
                                              lib: ["dom", "es2017"],
                                              target: "es2017",
                                              module: "commonjs",
                                              noImplicitAny:true
                                          }),
                                      gulp.dest(name)
                                  ], cb);
                              });

                    gulp.task(testTypeScriptSliceJsCleanTask(name), [],
                              function(cb){
                                  pump([gulp.src(path.join(name, "*.ice")), extreplace(".js"), paths(del)], cb);
                              });

                    gulp.task(testTypeScriptSliceTsCleanTask(name), [],
                              function(cb){
                                  pump([gulp.src(path.join(name, "*.ice")), extreplace(".d.ts"), paths(del)], cb);
                              });

                    gulp.task(testTypeScriptCleanTask(name),
                              [testTypeScriptSliceJsCleanTask(name), testTypeScriptSliceTsCleanTask(name)],
                              function(cb){
                                  pump([gulp.src(path.join(name, "*.ts")), extreplace(".js"), paths(del)], cb);
                              });
                });

gulp.task("test:ts", tstests.map(testTypeScriptCompileTask));
gulp.task("test:ts:clean", tstests.map(testTypeScriptCleanTask));

//
// Tasks to build IceJS Distribution
//
var root = path.resolve(path.join('__dirname', '..'));
var libs = ["Ice", "Glacier2", "IceStorm", "IceGrid"];

function generateTypeScriptTask(name) { return name.toLowerCase() + ":generate-typescript"; }
function generateTask(name){ return name.toLowerCase() + ":generate"; }
function libTask(name){ return name.toLowerCase() + ":lib"; }
function minLibTask(name){ return name.toLowerCase() + ":lib-min"; }
function babelTask(name){ return name.toLowerCase() + ":babel"; }
function babelLibTask(name){ return libTask(name) + "-babel";}
function babelMinLibTask(name){ return libTask(name) + "-babel-min"; }

function libFile(name) { return path.join(root, "lib", name + ".js"); }
function libFileMin(name) { return path.join(root, "lib", name + ".min.js"); }

function babelLibFile(name) { return path.join(root, "lib", "es5", name + ".js"); }
function babelLibFileMin(name) { return path.join(root, "lib", "es5", name + ".min.js"); }

function srcDir(name) { return "src/" + name; }
function libCleanTask(lib) { return lib + ":clean"; }

function libFiles(name){
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

function mapFiles(name){
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

function libSources(lib, sources){
    var srcs = sources.common || [];
    if(sources.browser){
        srcs = sources.common.concat(sources.browser);
    }

    srcs = srcs.map(function(f){
            return path.join(srcDir(lib), f);
        });

    if(sources.slice){
        srcs = srcs.concat(sources.slice.map(function(f){
                return path.join(srcDir(lib), path.basename(f, ".ice") + ".js");
            }));
    }
    return srcs;
}

function libGeneratedFiles(lib, sources){
    return sources.slice.map(function(f)
            {
                return path.join(srcDir(lib), path.basename(f, ".ice") + ".js");
            })
        .concat(path.join("src", "es5", lib, "*.js"))
        .concat(libFiles(lib))
        .concat(mapFiles(lib))
        .concat([path.join(srcDir(lib), ".depend", "*")]);
}

function sliceFile(f){ return path.join(sliceDir, f); }

libs.forEach(
    function(lib){
        var sources = JSON.parse(fs.readFileSync(path.join(srcDir(lib), "sources.json"), {encoding: "utf8"}));

        gulp.task(generateTypeScriptTask(lib),
                  function(cb){
                      var sliceSources = sources.typescriptSlice || sources.slice;
                      pump([
                          gulp.src(sliceSources.map(sliceFile)),
                          slice2js({dest: srcDir(lib), args: ["--typescript"]}),
                          gulp.dest(srcDir(lib))], cb);
                  });

        gulp.task(generateTask(lib), [generateTypeScriptTask(lib)],
                  function(cb){
                      pump([
                          gulp.src(sources.slice.map(sliceFile)),
                          slice2js({dest: srcDir(lib)}),
                          gulp.dest(srcDir(lib))], cb);
                  });

        gulp.task(libTask(lib), [generateTask(lib)],
            function(cb){
                pump([
                    gulp.src(libSources(lib, sources)),
                    sourcemaps.init(),
                    bundle(
                        {
                            srcDir: srcDir(lib),
                            modules: sources.modules,
                            target: libFile(lib)
                        }),
                    sourcemaps.write("../lib", {sourceRoot:"/src", addComment: false}),
                    gulp.dest("lib"),
                    gzip(),
                    gulp.dest("lib")], cb);
            });

        gulp.task(minLibTask(lib), [libTask(lib)],
            function(cb){
                pump([
                    gulp.src(libFile(lib)),
                    newer(libFileMin(lib)),
                    sourcemaps.init({loadMaps: false}),
                    terser(),
                    extreplace(".min.js"),
                    sourcemaps.write(".", {includeContent: false, addComment: false}),
                    gulp.dest("lib"),
                    gzip(),
                    gulp.dest("lib")], cb);
            });

        gulp.task(babelTask(lib), [generateTask(lib)],
            function(cb){
                pump([
                    gulp.src(path.join("src", lib, "*.js")),
                    newer(path.join("src", "es5", lib)),
                    babel({compact:false}),
                    gulp.dest(path.join("src", "es5", lib))], cb);
            });

        gulp.task(babelLibTask(lib), [libTask(lib)],
            function(cb){
                pump([
                    gulp.src(libFile(lib)),
                    newer(babelLibFile(lib)),
                    sourcemaps.init(),
                    babel({compact:false}),
                    sourcemaps.write("."),
                    gulp.dest("lib/es5"),
                    gzip(),
                    gulp.dest("lib/es5")], cb);
            });

        gulp.task(babelMinLibTask(lib), [babelLibTask(lib)],
            function(cb){
                pump([
                    gulp.src(babelLibFile(lib)),
                    newer(babelLibFileMin(lib)),
                    terser(),
                    extreplace(".min.js"),
                    sourcemaps.write(".", {includeContent: false, addComment: false}),
                    gulp.dest("lib/es5"),
                    gzip(),
                    gulp.dest("lib/es5")], cb);
            });

        gulp.task(libCleanTask(lib), [], function(){ del(libGeneratedFiles(lib, sources)); });
    });

gulp.task("ts:bundle", libs.map(generateTypeScriptTask),
    function(cb){
        pump([
            gulp.src("./src/index.d.ts"),
            tsbundle(),
            tsformat({}),
            gulp.dest("lib")], cb);
    });

gulp.task("ts:bundle:clean", [],
    function(){
        del("./lib/index.d.ts");
    });

gulp.task("dist", useBinDist ? [] :
    libs.map(libTask).concat(libs.map(minLibTask))
                     .concat(libs.map(babelMinLibTask))
                     .concat(libs.map(babelTask))
                     .concat(["ts:bundle"]));

gulp.task("dist:clean", libs.map(libCleanTask).concat("ts:bundle:clean"));

var buildDepends = ["dist", "test", "test:ts"];
var cleanDepends = ["test:clean", "test:ts:clean", "common:clean"];

if(!useBinDist)
{
    gulp.task("ice-module:package", ["dist"],
        function(cb){
            pump([gulp.src(['package.json']), gulp.dest(path.join("node_modules", "ice"))], cb);
        });
    gulp.task("ice-module", ["ice-module:package"],
        function(cb){
            pump([gulp.src(['src/**/*']), gulp.dest(path.join("node_modules", "ice", "src"))], cb);
        });
        buildDepends.push("ice-module");

    gulp.task("ice-module:clean", [],
        function(cb){
            pump([gulp.src(['node_modules/ice']), paths(del)], cb);
        });
        cleanDepends.push("ice-module:clean");
}

gulp.task("build", buildDepends);
gulp.task("clean", cleanDepends.concat(useBinDist ? [] : ["dist:clean"]));
gulp.task("default", ["build"]);
