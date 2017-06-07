// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var babel       = require("gulp-babel"),
    bundle      = require("./gulp/bundle"),
    concat      = require('gulp-concat'),
    del         = require("del"),
    extreplace  = require("gulp-ext-replace"),
    fs          = require("fs"),
    gulp        = require("gulp"),
    gzip        = require('gulp-gzip'),
    iceBuilder  = require('gulp-ice-builder'),
    jshint      = require('gulp-jshint'),
    cleancss    = require('gulp-clean-css'),
    newer       = require('gulp-newer'),
    open        = require("gulp-open"),
    path        = require('path'),
    paths       = require('vinyl-paths'),
    pump        = require('pump'),
    rollup      = require("rollup").rollup,
    sourcemaps  = require('gulp-sourcemaps'),
    spawn       = require("child_process").spawn,
    uglify      = require('uglify-js'),
    minifier    = require('gulp-uglify/minifier');

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

function slice2js(options) {
    var defaults = {};
    var opts = options || {};
    if(!useBinDist && process.platform == "win32" && !opts.exe)
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
    }
    defaults.args = opts.args || [];
    defaults.dest = opts.dest;
    defaults.exe = useBinDist ? undefined : (opts.exe || path.resolve(
            path.join("../cpp/bin", process.platform == "win32" ? path.join(platform, configuration, "slice2js.exe") : "slice2js")));
    defaults.args = defaults.args.concat(useBinDist ? [] : ["-I" + sliceDir]);
    return iceBuilder.compile(defaults);
}

//
// Test tasks
//
var tests = [
    "test/Ice/acm",
    "test/Ice/ami",
    "test/Ice/binding",
    "test/Ice/defaultValue",
    "test/Ice/enums",
    "test/Ice/exceptions",
    "test/Ice/exceptionsBidir",
    "test/Ice/facets",
    "test/Ice/facetsBidir",
    "test/Ice/hold",
    "test/Ice/info",
    "test/Ice/inheritance",
    "test/Ice/inheritanceBidir",
    "test/Ice/location",
    "test/Ice/objects",
    "test/Ice/operations",
    "test/Ice/operationsBidir",
    "test/Ice/optional",
    "test/Ice/optionalBidir",
    "test/Ice/promise",
    "test/Ice/properties",
    "test/Ice/proxy",
    "test/Ice/retry",
    "test/Ice/slicing/exceptions",
    "test/Ice/slicing/objects",
    "test/Ice/timeout",
    "test/Ice/number",
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
                      "test/Common/ControllerWorker.js"]),
            babel({compact: false}),
            gulp.dest("test/es5/Common")], cb);
    });

gulp.task("common:clean", [],
    function(){
        del(["test/Common/Controller.js",
             "test/Common/.depend",
             "test/es5/Common/Controller.js"]);
    });

gulp.task("import:slice2js", [],
    function(cb){
        pump([
            gulp.src(["test/Ice/import/Demo/Point.ice",
                      "test/Ice/import/Demo/Circle.ice",
                      "test/Ice/import/Demo/Square.ice",
                      "test/Ice/import/Demo/Canvas.ice"]),
            slice2js({ dest: "test/Ice/import/Demo", args:["-Itest/Ice/import"]}),
            gulp.dest("test/Ice/import/Demo")], cb);
    });

gulp.task("import:bundle", ["import:slice2js"],
    function(){
        return rollup({
            entry: "test/Ice/import/main.js",
            external: "ice"
        }).then(function(bundle){
            return bundle.write({
                format: "cjs",
                dest: "test/Ice/import/bundle.js"
            });
        });
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
                    slice2js({ args: ["-I" + name], dest: name }),
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
    ["common:slice-babel", "import:bundle"]));

gulp.task("test:clean", tests.map(testBabelCleanTask).concat(["common:clean", "import:clean"]));

//
// Tasks to build IceJS Distribution
//
var root = path.resolve(path.join('__dirname', '..'));
var libs = ["Ice", "Glacier2", "IceStorm", "IceGrid"];

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

        gulp.task(generateTask(lib),
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
                    minifier({compress:false}, uglify),
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
                    minifier({compress:false}, uglify),
                    extreplace(".min.js"),
                    sourcemaps.write(".", {includeContent: false, addComment: false}),
                    gulp.dest("lib/es5"),
                    gzip(),
                    gulp.dest("lib/es5")], cb);
            });

        gulp.task(libCleanTask(lib), [], function(){ del(libGeneratedFiles(lib, sources)); });
    });

gulp.task("dist", useBinDist ? [] :
    libs.map(libTask).concat(libs.map(minLibTask))
                     .concat(libs.map(babelMinLibTask))
                     .concat(libs.map(babelTask)));
gulp.task("dist:clean", libs.map(libCleanTask));

function runTestsWithBrowser(url)
{
    require("./bin/HttpServer")();
    var cmd = ["../scripts/Controller.py", "--endpoints", "ws -p 15002:wss -p 15003", "-d"];
    if(platform)
    {
        cmd.push("--platform=" + platform);
    }
    if(configuration)
    {
        cmd.push("--config=" + configuration);
    }

    var i = process.argv.indexOf("--");
    var argv = process.argv.filter(
        function(element, index, argv)
        {
            return i !== -1 && index > i;
        });
    cmd = cmd.concat(argv);

    var p  = require("child_process").spawn("python", cmd, {stdio: "inherit"});
    p.on("error", function(err)
        {
            if(err.message == "spawn python ENOENT")
            {
                console.log("Error: python is required in PATH to run tests");
                process.exit(1);
            }
            else
            {
                throw err;
            }
        });
    process.on(process.platform == "win32" ? "SIGBREAK" : "SIGINT",
        function()
        {
            process.exit();
        });
    process.on("exit", function()
        {
            p.kill();
        });
    return gulp.src("").pipe(open({uri: url}));
}

gulp.task("test:browser", useBinDist ? ["test"] : ["build"],
    function(url){
        return runTestsWithBrowser("http://127.0.0.1:8080/test/Ice/acm/index.html");
    });

gulp.task("test:browser-es5", useBinDist ? ["test"] : ["build"],
    function(url){
        return runTestsWithBrowser("http://127.0.0.1:8080/test/es5/Ice/acm/index.html");
    });

gulp.task("test:node", (useBinDist ? ["test"] : ["build"]),
    function(){
        var args = ["allTests.py", "--all"];
        if(platform)
        {
            args = args.concat(["--cpp-platform", platform]);
        }
        if(configuration)
        {
            args = args.concat(["--cpp-config", configuration]);
        }

        var p = require("child_process").spawn("python", args, {stdio: "inherit"});
        p.on("error",
            function(err)
            {
                if(err.message == "spawn python ENOENT")
                {
                    console.log("Error: python is required in PATH to run tests");
                    process.exit(1);
                }
                else
                {
                    throw err;
                }
            });

        process.on(process.platform == "win32" ? "SIGBREAK" : "SIGINT",
            function()
            {
                process.exit();
            });
        process.on("exit", function()
            {
                p.kill();
            });
    });

gulp.task("lint:html", ["build"],
    function(cb){
        pump([
            gulp.src(["**/*.html",
                      "!node_modules/**/*.html",
                      "!test/**/index.html"]),
            jshint.extract("auto"),
            jshint(),
            jshint.reporter('default')], cb);
    });

gulp.task("lint:js", ["build"],
    function(cb){
        pump([
            gulp.src(["gulpfile.js",
                      "gulp/**/*.js",
                      "src/**/*.js",
                      "src/**/browser/*.js",
                      "test/**/*.js",
                      "!src/es5/**/*.js",
                      "!test/es5/**/**/*.js",
                      "!**/Client.min.js"]),
            jshint(),
            jshint.reporter("default")], cb);
    });

var buildDepends = ["dist", "test"];
var cleanDepends = ["test:clean", "common:clean"];

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

gulp.task("lint", ["lint:js", "lint:html"]);
gulp.task("build", buildDepends);
gulp.task("clean", cleanDepends.concat(useBinDist ? [] : ["dist:clean"]));
gulp.task("default", ["build"]);
