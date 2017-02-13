// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var babel       = require("gulp-babel"),
    bower       = require("bower"),
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
    sourcemaps  = require('gulp-sourcemaps'),
    spawn       = require("child_process").spawn,
    uglify      = require("gulp-uglify"),
    rollup      = require("rollup").rollup;

var sliceDir   = path.resolve(__dirname, '..', 'slice');

var iceBinDist = (process.env.ICE_BIN_DIST || "").split(" ");
var useBinDist = iceBinDist.find(function(variable) {return variable == "js" || variable == "all" }) !== undefined;

function parseArg(argv, key)
{
    for(var i = 0; i < argv.length; ++i)
    {
        var e = argv[i];
        if(e == key)
        {
            return argv[i + 1];
        }
        else if(e.indexOf(key + "=") == 0)
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
    if(process.platform == "win32" && !opts.exe)
    {
        if(!platform || (platform != "Win32" && platform != "x64"))
        {
            console.log("Error: CPP_PLATFORM environment variable must be set to `Win32' or `x64', in order to locate slice2js.exe");
            process.exit(1);
        }

        if(!configuration || (configuration != "Debug" && configuration != "Release"))
        {
            console.log("Error: CPP_CONFIGURATION environment variable must be set to `Debug' or `Release', in order to locate slice2js.exe");
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

var common = {
    "scripts": [
        "bower_components/foundation/js/vendor/modernizr.js",
        "bower_components/foundation/js/vendor/jquery.js",
        "bower_components/foundation/js/foundation.min.js",
        "bower_components/nouislider/distribute/jquery.nouislider.all.js",
        "bower_components/animo.js/animo.js",
        "bower_components/spin.js/spin.js",
        "bower_components/spin.js/jquery.spin.js",
        "bower_components/URIjs/src/URI.js",
        "bower_components/highlightjs/highlight.pack.js",
        "assets/icejs.js"
    ],
    "styles": [
        "bower_components/foundation/css/foundation.css",
        "bower_components/animo.js/animate+animo.css",
        "bower_components/highlightjs/styles/vs.css",
        "bower_components/nouislider/distribute/jquery.nouislider.min.css",
        "assets/icejs.css"
    ]
};

gulp.task("common:slice", [],
    function(){
        return gulp.src(["../scripts/Controller.ice"])
            .pipe(slice2js({dest: "test/Common"}))
            .pipe(gulp.dest("test/Common"));
    });

gulp.task("common:slice-babel", ["common:slice"],
    function(){
        return gulp.src(["test/Common/Controller.js", "test/Common/ControllerI.js", "test/Common/ControllerWorker.js"])
            .pipe(babel({compact: false}))
            .pipe(gulp.dest("test/es5/Common"));
    });

gulp.task("common:slice:clean", [],
    function(){
        del(["test/Common/Controller.js", "test/Common/.depend", "test/es5/Common/Controller.js"]);
    });

gulp.task("common:js", ["bower"],
    function(){
        return gulp.src(common.scripts)
            .pipe(newer("assets/common.min.js"))
            .pipe(concat("common.min.js"))
            //.pipe(uglify()) // TODO: uglify doesn't support es6
            .pipe(gulp.dest("assets"))
            .pipe(gzip())
            .pipe(gulp.dest("assets"));
    });

gulp.task("common:css", ["bower"],
    function(){
        return gulp.src(common.styles)
            .pipe(newer("assets/common.css"))
            .pipe(concat("common.css"))
            .pipe(cleancss())
            .pipe(gulp.dest("assets"))
            .pipe(gzip())
            .pipe(gulp.dest("assets"));
    });

gulp.task("common:js-babel", [],
    function(){
        return gulp.src("test/Common/Common.js")
                   .pipe(babel({compact: false}))
                   .pipe(gulp.dest("test/es5/Common/"));
    });

gulp.task("common:clean", [],
    function(){
        del(["assets/common.css", "assets/common.min.js"]);
    });

gulp.task("import:slice2js", [],
    function(){
        return gulp.src(["test/Ice/import/Demo/Point.ice",
                         "test/Ice/import/Demo/Circle.ice",
                         "test/Ice/import/Demo/Square.ice",
                         "test/Ice/import/Demo/Canvas.ice"])
            .pipe(slice2js(
                {dest: "test/Ice/import/Demo",
                 args:["-Itest/Ice/import"]}))
            .pipe(gulp.dest("test/Ice/import/Demo"));
    });

gulp.task("import:bundle", ["import:slice2js"],
    function()
    {
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
    function()
    {
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
            function(){
                return gulp.src(path.join(name, "*.ice"))
                    .pipe(
                        slice2js({
                            args: ["-I" + name],
                            dest: name
                        }))
                    .pipe(gulp.dest(name));
            });

        gulp.task(testBabelTask(name), [testTask(name)],
            function(){
                return gulp.src([path.join(name, "*.js")])
                    .pipe(babel({compact: false}))
                    .pipe(gulp.dest(name.replace("test/", "test/es5/")));
            });

        gulp.task(testCleanDependTask(name), [],
            function(){
                return gulp.src(path.join(name, ".depend"))
                    .pipe(paths(del));
            });

        gulp.task(testCleanTask(name), [testCleanDependTask(name)],
            function(){
                return gulp.src(path.join(name, "*.ice"))
                    .pipe(extreplace(".js"))
                    .pipe(paths(del));
            });

        gulp.task(testBabelCleanTask(name), [testCleanTask(name)],
            function(){
                var s = name.replace("test/", "test/es5/");
                return gulp.src([path.join(s, "*.js")]).pipe(paths(del));
            });
    });

gulp.task("test", tests.map(testBabelTask).concat(
    ["common:slice-babel", "common:js", "common:js-babel", "common:css", "import:bundle"]));

gulp.task("test:clean", tests.map(testBabelCleanTask).concat(["common:slice:clean", "import:clean"]));

//
// Tasks to build IceJS Distribution
//
var root = path.resolve(path.join('__dirname', '..'));
var libs = ["Ice", "Glacier2", "IceStorm", "IceGrid"];

function generateTask(name){ return name.toLowerCase() + ":generate"; }
function libTask(name){ return name.toLowerCase() + ":lib"; }
function minLibTask(name){ return libTask(name) + "-min"; }
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
            function(){
                return gulp.src(sources.slice.map(sliceFile))
                    .pipe(slice2js({dest: srcDir(lib)}))
                    .pipe(gulp.dest(srcDir(lib)));
            });

        gulp.task(libTask(lib), [generateTask(lib)],
            function(){
                return gulp.src(libSources(lib, sources))
                    .pipe(sourcemaps.init())
                    .pipe(
                        bundle(
                            {
                                srcDir: srcDir(lib),
                                modules: sources.modules,
                                target: libFile(lib)
                            }))
                    .pipe(sourcemaps.write("../lib", {sourceRoot:"/src", addComment: false}))
                    .pipe(gulp.dest("lib"))
                    .pipe(gzip())
                    .pipe(gulp.dest("lib"));
            });

        gulp.task(minLibTask(lib), [libTask(lib)],
            function(){
                return gulp.src(libFile(lib))
                    .pipe(newer(libFileMin(lib)))
                    .pipe(sourcemaps.init({loadMaps:true}))
                    //.pipe(uglify({compress:false})) // TODO: uglify doesn't support ES6
                    .pipe(extreplace(".min.js"))
                    .pipe(sourcemaps.write("../lib", {includeContent: false, addComment: false}))
                    .pipe(gulp.dest("lib"))
                    .pipe(gzip())
                    .pipe(gulp.dest("lib"));
            });

        gulp.task(babelTask(lib), [generateTask(lib)],
            function(){
                return gulp.src(path.join("src", lib, "*.js"))
                    .pipe(babel({compact:false}))
                    .pipe(gulp.dest(path.join("src", "es5", lib)));
            });

        gulp.task(babelLibTask(lib), [libTask(lib)],
            function(){
                return gulp.src(libFile(lib))
                    .pipe(newer(babelLibFile(lib)))
                    .pipe(sourcemaps.init())
                    .pipe(babel({compact:false}))
                    .pipe(sourcemaps.write("."))
                    .pipe(gulp.dest("lib/es5"))
                    .pipe(gzip())
                    .pipe(gulp.dest("lib/es5"));
            });

        gulp.task(babelMinLibTask(lib), [babelLibTask(lib)],
            function(){
                return gulp.src(babelLibFile(lib))
                    .pipe(newer(babelLibFileMin(lib)))
                    .pipe(sourcemaps.init({loadMaps:true, sourceRoot:"./"}))
                    .pipe(uglify({compress:false}))
                    .pipe(extreplace(".min.js"))
                    .pipe(sourcemaps.write(".", {includeContent: false, addComment: false}))
                    .pipe(gulp.dest("lib/es5"))
                    .pipe(gzip())
                    .pipe(gulp.dest("lib/es5"));
            });

        gulp.task(libCleanTask(lib), [], function(){ del(libGeneratedFiles(lib, sources)); });
    });

gulp.task("bower", [],
    function(cb){
        bower.commands.install().on("end", function(){ cb(); });
    });

gulp.task("dist:libs", ["bower"],
    function(){
        return gulp.src(["bower_components/ice/lib/*", "bower_components/ice/lib/**/*"])
            .pipe(gulp.dest("lib"));
    });

gulp.task("dist", useBinDist ? ["dist:libs"] : libs.map(minLibTask).concat(libs.map(babelMinLibTask)).concat(libs.map(babelTask)));
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

gulp.task("test:run-with-browser", useBinDist ? ["test"] : ["build"],
    function(url){
        return runTestsWithBrowser("http://127.0.0.1:8080/test/Ice/acm/index.html");
    });

gulp.task("test:run-with-browser-es5", useBinDist ? ["test"] : ["build"],
    function(url){
        return runTestsWithBrowser("http://127.0.0.1:8080/test/es5/Ice/acm/index.html");
    });

gulp.task("test:run-with-node", (useBinDist ? ["test"] : ["build"]),
    function(){
        var p  = require("child_process").spawn("python", ["allTests.py", "--all"], {stdio: "inherit"});
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
    });

gulp.task("lint:html", ["build"],
    function(){
        return gulp.src(["**/*.html",
                         "!bower_components/**/*.html",
                         "!node_modules/**/*.html",
                         "!test/**/index.html"])
            .pipe(jshint.extract("auto"))
            .pipe(jshint())
            .pipe(jshint.reporter('default'));
    });

gulp.task("lint:js", ["build"],
    function(){
        return gulp.src(["gulpfile.js",
                         "gulp/**/*.js",
                         "src/**/*.js",
                         "src/**/browser/*.js",
                         "test/**/*.js",
                         "!src/es5/**/*.js",
                         "!test/es5/**/**/*.js",
                         "!**/Client.min.js"])
            .pipe(jshint())
            .pipe(jshint.reporter("default"));
    });


var buildDepends = ["dist", "test"];
var cleanDepends = ["test:clean", "common:clean"];

if(!useBinDist)
{
gulp.task("ice-module:package", ["dist"],
    function()
    {
        return gulp.src(['package.json']).pipe(
            gulp.dest(path.join("node_modules", "ice")));
    });
gulp.task("ice-module", ["ice-module:package"],
    function()
    {
        gulp.src(['src/**/*']).pipe(
               gulp.dest(path.join("node_modules", "ice", "src")));
    });
    buildDepends.push("ice-module");

gulp.task("ice-module:clean", [],
    function()
    {
        return gulp.src(['node_modules/ice']).pipe(paths(del));
    });
    cleanDepends.push("ice-module:clean")
}

gulp.task("lint", ["lint:js", "lint:html"]);
gulp.task("build", buildDepends);
gulp.task("clean", cleanDepends.concat(useBinDist ? [] : ["dist:clean"]));
gulp.task("default", ["build"]);
