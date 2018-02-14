// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var bower       = require("bower"),
    bundle      = require("./gulp/bundle"),
    concat      = require('gulp-concat'),
    del         = require("del"),
    extreplace  = require("gulp-ext-replace"),
    fs          = require("fs"),
    gulp        = require("gulp"),
    gzip        = require('gulp-gzip'),
    iceBuilder  = require('gulp-ice-builder'),
    jshint      = require('gulp-jshint'),
    minifycss   = require('gulp-minify-css'),
    newer       = require('gulp-newer'),
    open        = require("gulp-open"),
    path        = require('path'),
    paths       = require('vinyl-paths'),
    sourcemaps  = require('gulp-sourcemaps'),
    spawn       = require("child_process").spawn,
    uglify      = require("gulp-uglify");

var sliceDir   = path.resolve(__dirname, '..', 'slice');

var useBinDist = process.env.USE_BIN_DIST == "yes";

function slice2js(options) {
    var defaults = {};
    var opts = options || {};

    defaults.args = opts.args || [];
    defaults.dest = opts.dest;
    defaults.exe = useBinDist ? undefined : (opts.exe || path.resolve(
            path.join("../cpp/bin", process.platform == "win32" ? "slice2js.exe" : "slice2js")));
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
    "test/Glacier2/router"
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
        return gulp.src(["test/Common/Controller.ice"])
            .pipe(slice2js({dest: "test/Common"}))
            .pipe(gulp.dest("test/Common"));
    });

gulp.task("common:slice:clean", [],
    function(){
        del(["test/Common/Controller.js", "test/Common/.depend"]);
    });

gulp.task("common:slice:watch", ["common:slice"],
    function(){
        gulp.watch(["test/Common/Controller.ice"],
            function(){
                gulp.start("common:slice");
            });
    });

gulp.task("common:js", ["bower"],
    function(){
        return gulp.src(common.scripts)
            .pipe(newer("assets/common.min.js"))
            .pipe(concat("common.min.js"))
            .pipe(uglify())
            .pipe(gulp.dest("assets"))
            .pipe(gzip())
            .pipe(gulp.dest("assets"));
    });

gulp.task("common:js:watch", ["common:js"],
    function(){
        gulp.watch(common.scripts,
            function(){
                gulp.start("common:js");
            });
    });

gulp.task("common:css", ["bower"],
    function(){
        return gulp.src(common.styles)
            .pipe(newer("assets/common.css"))
            .pipe(concat("common.css"))
            .pipe(minifycss())
            .pipe(gulp.dest("assets"))
            .pipe(gzip())
            .pipe(gulp.dest("assets"));
    });

gulp.task("common:css:watch", ["common:css"],
    function(){
        gulp.watch(common.styles,
            function(){
                gulp.start("common:css");
            });
    });

gulp.task("common:clean", [],
    function(){
        del(["assets/common.css", "assets/common.min.js"]);
    });

function testTask(name) { return name.replace("/", "_"); }
function testWatchTask(name) { return testTask(name) + ":watch"; }
function testCleanDependTask(name) { return testTask(name) + "-depend:clean"; }
function testCleanTask(name) { return testTask(name) + ":clean"; }

tests.forEach(
    function(name){
        gulp.task(testTask(name), (useBinDist ? [] : ["dist"]),
            function(){
                return gulp.src(path.join(name, "*.ice"))
                    .pipe(
                        slice2js({
                            args: ["-I" + name],
                            dest: name
                        }))
                    .pipe(gulp.dest(name));
            });

        gulp.task(testWatchTask(name), [testTask(name)],
            function(){
                gulp.watch([path.join(name, "*.ice")], [testTask(name)]);

                gulp.watch(
                    [path.join(name, "*.js"), path.join(name, "browser", "*.js")]);
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
    });

gulp.task("test", tests.map(testTask).concat(["common:slice", "common:js", "common:css"]));

gulp.task("test:watch", tests.map(testWatchTask).concat(
    ["common:slice:watch", "common:css:watch", "common:js:watch"]));

gulp.task("test:clean", tests.map(testCleanTask).concat(["common:slice:clean"]));

//
// Tasks to build IceJS Distribution
//
var root = path.resolve(path.join('__dirname', '..'));
var libs = ["Ice", "Glacier2", "IceStorm", "IceGrid"];

function generateTask(name){ return name.toLowerCase() + ":generate"; }
function libTask(name){ return name.toLowerCase() + ":lib"; }
function minLibTask(name){ return name.toLowerCase() + ":lib-min"; }
function libFile(name) { return path.join(root, "lib", name + ".js"); }
function libFileMin(name) { return path.join(root, "lib", name + ".min.js"); }
function srcDir(name) { return "src/" + name; }
function libCleanTask(lib) { return lib + ":clean"; }
function libWatchTask(lib) { return lib + ":watch"; }

function libFiles(name){
    return [
        path.join(root, "lib", name + ".js"),
        path.join(root, "lib", name + ".js.gz"),
        path.join(root, "lib", name + ".min.js"),
        path.join(root, "lib", name + ".min.js.gz")];
}

function mapFiles(name){
    return [
        path.join(root, "lib", name + ".js.map"),
        path.join(root, "lib", name + ".js.map.gz"),
        path.join(root, "lib", name + ".min.js.map"),
        path.join(root, "lib", name + ".min.js.map.gz")];
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
        .concat(libFiles(lib))
        .concat(mapFiles(lib))
        .concat([path.join(srcDir(lib), ".depend", "*")]);
}

function watchSources(lib, sources){
    var srcs = sources.common || [];
    if(sources.browser){
        srcs = sources.common.concat(sources.browser); }
    srcs = srcs.map(
        function(f){
            return path.join(srcDir(lib), f); });
    return srcs;
}

function sliceFile(f){ return path.join(sliceDir, f); }

libs.forEach(
    function(lib){
        var sources = JSON.parse(fs.readFileSync(path.join(srcDir(lib), "sources.json"), {encoding: "utf8"}));

        gulp.task(generateTask(lib),
            function(){
                return gulp.src(sources.slice.map(sliceFile))
                    .pipe(slice2js({args: ["--ice"], dest: srcDir(lib)}))
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
                    .pipe(uglify({compress:false}))
                    .pipe(extreplace(".min.js"))
                    .pipe(sourcemaps.write("../lib", {includeContent: false, addComment: false}))
                    .pipe(gulp.dest("lib"))
                    .pipe(gzip())
                    .pipe(gulp.dest("lib"));
            });

        gulp.task(libCleanTask(lib), [], function(){ del(libGeneratedFiles(lib, sources)); });
        gulp.task(libWatchTask(lib), [minLibTask(lib)],
            function(){
                gulp.watch(sources.slice.map(sliceFile).concat(watchSources(lib, sources)));
            });
    });

gulp.task("bower", [],
    function(cb){
        bower.commands.install().on("end", function(){ cb(); });
    });

gulp.task("dist:libs", ["bower"],
    function(){
        return gulp.src(["bower_components/ice/lib/*"])
            .pipe(gulp.dest("lib"));
    });

gulp.task("dist", useBinDist ? ["dist:libs"] : libs.map(minLibTask));
gulp.task("dist:watch", libs.map(libWatchTask));
gulp.task("dist:clean", libs.map(libCleanTask));
gulp.task("watch", ["test:watch"].concat(useBinDist ? [] : ["dist:watch"]));

gulp.task("test:run-with-browser", ["watch"].concat(useBinDist ? ["test"] : ["build"]),
    function(){
        require("./bin/HttpServer")();
        var cmd = ["../scripts/TestController.py"];
        cmd = cmd.concat(process.argv.slice(3));
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
        return gulp.src("")
               .pipe(open({uri: "http://127.0.0.1:8080/test/Ice/acm/index.html"}));
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
                         "!**/Client.min.js"])
            .pipe(jshint())
            .pipe(jshint.reporter("default"));
    });

gulp.task("lint", ["lint:js", "lint:html"]);
gulp.task("build", ["dist", "test"]);
gulp.task("clean", ["test:clean", "common:clean"].concat(useBinDist ? [] : ["dist:clean"]));
gulp.task("default", ["build"]);
