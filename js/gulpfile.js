// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var bower       = require("bower");
var browserSync = require("browser-sync");
var concat      = require('gulp-concat');
var del         = require("del");
var extreplace  = require("gulp-ext-replace");
var fs          = require("fs");
var gulp        = require("gulp");
var gzip        = require("gulp-gzip");
var jshint      = require('gulp-jshint');
var minifycss   = require('gulp-minify-css');
var newer       = require('gulp-newer');
var open        = require("gulp-open");
var path        = require("path");
var paths       = require('vinyl-paths');
var spawn       = require("child_process").spawn;
var uglify      = require("gulp-uglify");

var HttpServer  = require("./bin/HttpServer");
var slice2js    = require("./gulp/gulp-slice2js");
var bundle      = require("./gulp/gulp-bundle");

var useBinDist  = process.env.USE_BIN_DIST == "yes";

function getSliceArgs(options)
{
    var defaults = {};
    var opts = options || {};

    defaults.args = opts.args || [];
    defaults.dest = opts.dest;

    if(useBinDist)
    {
        defaults.exe = undefined;
    }
    else
    {
        defaults.args = defaults.args.concat(["-I" + path.resolve("../slice/")]);
        defaults.exe = opts.exe || path.resolve(
            path.join("../cpp/bin", process.platform == "win32" ? "slice2js.exe" : "slice2js"));
    }
    return defaults;
}

function sliceFile(f){ return path.join("../slice/", f); }

function libSources(lib, sources)
{
    var srcs = sources.common || [];
    if(sources.browser)
    {
        srcs = sources.common.concat(sources.browser);
    }

    srcs = srcs.map(function(f)
        {
            return path.join(srcDir(lib), f);
        });

    if(sources.slice)
    {
        srcs = srcs.concat(sources.slice.map(function(f)
            {
                return path.join(srcDir(lib), path.basename(f, ".ice") + ".js");
            }));
    }

    return srcs;
}

function watchSources(lib, sources)
{
    var srcs = sources.common || [];
    if(sources.browser)
    {
        srcs = sources.common.concat(sources.browser);
    }

    srcs = srcs.map(function(f)
        {
            return path.join(srcDir(lib), f);
        });
    return srcs;
}

function generateTask(name) { return name.toLowerCase() + ":generate"; }
function libTask(name) { return name.toLowerCase() + ":lib"; }
function minLibTask(name) { return name.toLowerCase() + ":lib-min"; }
function libFile(name) { return path.join("lib", name + ".js"); }
function libFileMin(name) { return path.join("lib", name + ".min.js"); }
function srcDir(name) { return "src/" + name; }
function libCleanTask(lib){ return lib + ":clean"; }
function libWatchTask(lib){ return lib + ":watch"; }

function libFiles(name)
{
    return [
        path.join("lib", name + ".js"),
        path.join("lib", name + ".js.gz"),
        path.join("lib", name + ".min.js"),
        path.join("lib", name + ".min.js.gz")];
}

function libGeneratedFiles(lib, sources)
{
    return sources.slice.map(function(f)
            {
                return path.join(srcDir(lib), path.basename(f, ".ice") + ".js");
            })
        .concat(libFiles(lib))
        .concat([path.join(srcDir(lib), ".depend", "*")]);
}

var libs = ["Ice", "Glacier2", "IceStorm", "IceGrid"];

libs.forEach(
    function(lib)
    {
        var sources = JSON.parse(fs.readFileSync(path.join(srcDir(lib), "sources.json"), {encoding: "utf8"}));

        gulp.task(generateTask(lib),
            function()
            {
                return gulp.src(sources.slice.map(sliceFile))
                    .pipe(slice2js(getSliceArgs({args: ["--ice","--icejs"], dest: srcDir(lib)})))
                    .pipe(gulp.dest(srcDir(lib)));
            });

        gulp.task(libTask(lib), [generateTask(lib)],
            function()
            {
                return gulp.src(libSources(lib, sources))
                    .pipe(bundle(
                        {
                            srcDir: srcDir(lib),
                            modules: sources.modules,
                            target: libFile(lib)
                        }))
                    .pipe(gulp.dest("lib"))
                    .pipe(gzip())
                    .pipe(gulp.dest("lib"));
            });

        gulp.task(minLibTask(lib), [libTask(lib)],
            function()
            {
                return gulp.src(libFile(lib))
                    .pipe(newer(libFileMin(lib)))
                    .pipe(extreplace(".min.js"))
                    .pipe(uglify())
                    .pipe(gulp.dest("lib"))
                    .pipe(gzip())
                    .pipe(gulp.dest("lib"));
            });

        gulp.task(libCleanTask(lib), [],
            function()
            {
                del(libGeneratedFiles(lib, sources));
            });

        gulp.task(libWatchTask(lib), [minLibTask(lib)],
            function()
            {
                gulp.watch(sources.slice.map(sliceFile).concat(watchSources(lib, sources)),
                    function(){
                        gulp.start(minLibTask(lib), function(){
                            browserSync.reload(libFileMin(lib));
                        });
                    });
            });
    });

gulp.task("bower", [],
    function(cb)
    {
        bower.commands.install().on("end", function(){ cb(); });
    });

gulp.task("dist:libs", ["bower"],
    function()
    {
        return gulp.src(["bower_components/zeroc-icejs/lib/*"])
            .pipe(gulp.dest("lib"));
    });

gulp.task("dist", useBinDist ? ["dist:libs"] : libs.map(minLibTask));
gulp.task("dist:watch", libs.map(libWatchTask));
gulp.task("dist:clean", libs.map(libCleanTask));

var common =
{
    "scripts": [
        "bower_components/foundation/js/vendor/modernizr.js",
        "bower_components/foundation/js/vendor/jquery.js",
        "bower_components/foundation/js/foundation.min.js",
        "bower_components/nouislider/distribute/jquery.nouislider.all.js",
        "bower_components/animo.js/animo.js",
        "bower_components/spin.js/spin.js",
        "bower_components/spin.js/jquery.spin.js",
        "bower_components/highlightjs/highlight.pack.js",
        "assets/icejs.js"],
    "styles":
        ["bower_components/foundation/css/foundation.css",
         "bower_components/animo.js/animate+animo.css",
         "bower_components/highlightjs/styles/vs.css",
         "bower_components/nouislider/distribute/jquery.nouislider.min.css",
         "assets/icejs.css"]
};

gulp.task("common:slice", [],
    function()
    {
        return gulp.src(["test/Common/Controller.ice"])
            .pipe(slice2js(getSliceArgs({dest: "test/Common"})))
            .pipe(gulp.dest("test/Common"));
    });

gulp.task("common:slice:watch", ["common:slice"],
    function()
    {
        gulp.watch(["test/Common/Controller.ice"], function(){
            gulp.start("common:slice", function(){
                browserSync.reload("test/Common/Controller.js");
            });
        });
    });

gulp.task("common:js", ["bower"],
    function()
    {
        return gulp.src(common.scripts)
            .pipe(newer("assets/common.min.js"))
            .pipe(concat("common.min.js"))
            .pipe(uglify())
            .pipe(gulp.dest("assets"))
            .pipe(gzip())
            .pipe(gulp.dest("assets"));
    });

gulp.task("common:js:watch", ["common:js"],
    function()
    {
        gulp.watch(common.scripts,
            function(){
                gulp.start("common:js", function(){
                    browserSync.reload("assets/common.min.js");
                });
            });
    });

gulp.task("common:css", ["bower"],
    function()
    {
        return gulp.src(common.styles)
            .pipe(newer("assets/common.css"))
            .pipe(concat("common.css"))
            .pipe(minifycss())
            .pipe(gulp.dest("assets"))
            .pipe(gzip())
            .pipe(gulp.dest("assets"));
    });

gulp.task("common:css:watch", ["common:css"],
    function()
    {
        gulp.watch(common.styles,
            function(){
                gulp.start("common:css", function(){
                    browserSync.reload("assets/common.css");
                });
            });
    });

gulp.task("common:clean", [],
    function()
    {
        del(["assets/common.css", "assets/common.min.js"]);
    });

var subprojects =
{
    test: [
        "Ice/acm", "Ice/ami", "Ice/binding", "Ice/defaultValue", "Ice/enums", "Ice/exceptions",
        "Ice/exceptionsBidir", "Ice/facets", "Ice/facetsBidir", "Ice/hold", "Ice/inheritance",
        "Ice/inheritanceBidir", "Ice/location", "Ice/objects", "Ice/operations", "Ice/operationsBidir",
        "Ice/optional", "Ice/optionalBidir", "Ice/promise", "Ice/properties", "Ice/proxy", "Ice/retry",
        "Ice/slicing/exceptions", "Ice/slicing/objects", "Ice/timeout", "Glacier2/router"],
    demo: ["Ice/hello", "Ice/throughput", "Ice/minimal", "Ice/latency", "Ice/bidir", "Glacier2/chat",
            "ChatDemo"]
};

var minDemos =
{
    "Ice/minimal":
    {
        srcs: [
            "lib/Ice.min.js",
            "demo/Ice/minimal/Hello.js",
            "demo/Ice/minimal/browser/Client.js"],
        dest: "demo/Ice/minimal/browser/"
    },
    "ChatDemo":
    {
        srcs: [
            "lib/Ice.min.js",
            "lib/Glacier2.min.js",
            "demo/ChatDemo/Chat.js",
            "demo/ChatDemo/ChatSession.js",
            "demo/ChatDemo/Client.js"],
        dest: "demo/ChatDemo"
    }
};

function testHtmlTask(name) { return "test_" + name.replace("/", "_") + ":html"; }
function testHtmlCleanTask(name) { return "test_" + name.replace("/", "_") + ":html:clean"; }

subprojects.test.forEach(
    function(name)
    {
        gulp.task(testHtmlTask(name), [],
            function()
            {
                return gulp.src("test/Common/index.html")
                    .pipe(newer(path.join("test", name, "index.html")))
                    .pipe(gulp.dest(path.join("test", name)));
            });

        gulp.task(testHtmlCleanTask(name), [],
            function()
            {
                del(path.join("test", name, "index.html"));
            });
    });

gulp.task("html", subprojects.test.map(testHtmlTask));
gulp.task("html:watch", ["html"],
    function()
    {
        gulp.watch(["test/Common/index.html"], ["html"]);
    });
gulp.task("html:clean", subprojects.test.map(testHtmlCleanTask));

Object.keys(subprojects).forEach(
    function(group)
    {
        function groupTask(name) { return group + "_" + name.replace("/", "_"); }
        function groupGenerateTask(name) { return groupTask(name); }
        function groupWatchTask(name) { return groupTask(name) + ":watch"; }
        function groupCleanTask(name) { return groupTask(name) + ":clean"; }

        subprojects[group].forEach(
            function(name)
            {
                gulp.task(groupGenerateTask(name), (useBinDist ? [] : ["dist"]),
                    function()
                    {
                        return gulp.src(path.join(group, name, "*.ice"))
                            .pipe(slice2js(getSliceArgs(
                                {
                                    args: ["-I" + path.join(group, name)],
                                    dest: path.join(group, name)
                                })))
                            .pipe(gulp.dest(path.join(group, name)));
                    });

                gulp.task(groupWatchTask(name), 
                    (group == "test" ? [groupGenerateTask(name), "html"] : [groupGenerateTask(name)]),
                    function()
                    {
                        gulp.watch([path.join(group, name, "*.ice")], [groupGenerateTask(name)]);

                        gulp.watch([path.join(group, name, "*.js"),
                                    path.join(group, name, "browser", "*.js"),
                                    path.join(group, name, "*.html")], function(e){
                                        browserSync.reload(e.path);
                                    });
                    });

                gulp.task(groupCleanTask(name), [],
                    function()
                    {
                        return gulp.src(path.join(group, name, "*.ice"))
                            .pipe(extreplace(".js"))
                            .pipe(paths(del));
                    });
            });

        gulp.task(group, subprojects[group].map(groupGenerateTask).concat(
            group == "test" ? ["common:slice", "common:js", "common:css"].concat(subprojects.test.map(testHtmlTask)) :
                              ["common:slice", "common:js", "common:css", "demo_Ice_minimal:min", "demo_ChatDemo:min"]));

        gulp.task(group + ":watch", subprojects[group].map(groupWatchTask).concat(
            group == "test" ? ["common:slice:watch", "common:css:watch", "common:js:watch", "html:watch"] :
                              ["common:css:watch", "common:js:watch"].concat(Object.keys(minDemos).map(minDemoWatchTaskName))));
        
        gulp.task(group + ":clean", subprojects[group].map(groupCleanTask).concat(
            group == "test" ? subprojects.test.map(testHtmlCleanTask) : ["demo_Ice_minimal:min:clean", "demo_ChatDemo:min:clean"]));
    });

function demoTaskName(name) { return "demo_" + name.replace("/", "_"); }
function minDemoTaskName(name) { return demoTaskName(name) + ":min"; }
function minDemoWatchTaskName(name) { return minDemoTaskName(name) + ":watch"; }
function minDemoCleanTaskName(name) { return minDemoTaskName(name) + ":clean"; }

Object.keys(minDemos).forEach(
    function(name)
    {
        var demo = minDemos[name];

        gulp.task(minDemoTaskName(name), [demoTaskName(name)],
            function()
            {
                return gulp.src(demo.srcs)
                    .pipe(newer(path.join(demo.dest, "Client.min.js")))
                    .pipe(concat("Client.min.js"))
                    .pipe(uglify())
                    .pipe(gulp.dest(demo.dest))
                    .pipe(gzip())
                    .pipe(gulp.dest(demo.dest));
            });

        gulp.task(minDemoWatchTaskName(name), [minDemoTaskName(name)],
            function()
            {
                gulp.watch(demo.srcs, [minDemoTaskName(name)]);
            });

        gulp.task(minDemoCleanTaskName(name), [],
            function()
            {
                del([path.join(demo.dest, "Client.min.js"),
                     path.join(demo.dest, "Client.min.js.gz")]);
            });
    });

gulp.task("watch", ["test:watch", "demo:watch"].concat(useBinDist ? [] : ["dist:watch"]));

gulp.task("demo:run", ["watch"],
    function()
    {
        browserSync();
        HttpServer();
        
        return gulp.src("./index.html").pipe(open("", {url: "http://127.0.0.1:8080/index.html"}));
    });

gulp.task("test:run-with-browser", ["watch"].concat(useBinDist ? ["test", "demo"] : ["build"]),
    function()
    {
        browserSync();
        HttpServer();
        
        var p  = require("child_process").spawn("python", ["test/Common/run.py"], {stdio: "inherit"});
        function exit() { p.exit(); }
        process.on("SIGINT", exit);
        process.on("exit", exit);
        return gulp.src("./index.html").pipe(open("", {url: "http://127.0.0.1:8080/index.html"}));
    });

gulp.task("test:run-with-node", (useBinDist ? ["test"] : ["build"]),
    function()
    {
        var p  = require("child_process").spawn("python", ["allTests.py", "--all"], {stdio: "inherit"});
        function exit() { p.exit(); }
        process.on("SIGINT", exit);
        process.on("exit", exit);
    });

gulp.task("lint:html", ["build"],
    function()
    {
        return gulp.src([
                "**/*.html",
                "!bower_components/**/*.html",
                "!node_modules/**/*.html",
                "!test/**/index.html"])
            .pipe(jshint.extract("auto"))
            .pipe(jshint())
            .pipe(jshint.reporter('default'));
    });

gulp.task("lint:js", ["build"],
    function()
    {
        return gulp.src([
                "gulpfile.js",
                "gulp/**/*.js",
                "src/**/*.js",
                "src/**/browser/*.js",
                "test/**/*.js",
                "demo/**/*.js",
                "!**/Client.min.js"])
            .pipe(jshint())
            .pipe(jshint.reporter("default"));
    });

gulp.task("lint", ["lint:js", "lint:html"]);
gulp.task("build", ["dist", "test", "demo"]);
gulp.task("clean", ["test:clean", "demo:clean", "common:clean"].concat(useBinDist ? [] : ["dist:clean"]));
gulp.task("default", ["build"]);
