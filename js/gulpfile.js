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
var gulp        = require("gulp");
var jshint      = require('gulp-jshint');
var open        = require("gulp-open");
var spawn       = require("child_process").spawn;

var useBinDist = require('./gulp/util').useBinDist;
var HttpServer = require("./bin/HttpServer");

//
// Tasks to build IceJS Distribution
//
var libTasks   = require('./gulp/libTasks')(gulp);

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

gulp.task("dist", useBinDist ? ["dist:libs"] : libTasks.buildTasks);
gulp.task("dist:watch", libTasks.watchTasks);
gulp.task("dist:clean", libTasks.cleanTasks);

//
// Common Tasks for the tests and demos
//
require('./gulp/commonTasks')(gulp);

//
//  Test and demo tasks
//
require('./gulp/testAndDemoTasks')(gulp);

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

        var p  = require("child_process").spawn("python", ["../scripts/TestController.py"], {stdio: "inherit"});
        function exit() { p.kill(); }
        process.on("SIGINT", exit);
        process.on("exit", exit);
        return gulp.src("./index.html").pipe(open("", {url: "http://127.0.0.1:8080/index.html"}));
    });

gulp.task("test:run-with-node", (useBinDist ? ["test"] : ["build"]),
    function()
    {
        var p  = require("child_process").spawn("python", ["allTests.py", "--all"], {stdio: "inherit"});
        function exit() { p.kill(); }
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
