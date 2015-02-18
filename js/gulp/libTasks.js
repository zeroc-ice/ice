// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var browserSync = require("browser-sync");
var del         = require("del");
var extreplace  = require("gulp-ext-replace");
var fs          = require("fs");
var gulp        = require('gulp');
var gzip        = require('gulp-gzip');
var newer       = require('gulp-newer');
var path        = require('path');
var sourcemaps  = require('gulp-sourcemaps');
var uglify      = require("gulp-uglify");

var bundle      = require("./bundle");
var util        = require('./util');

var root = path.resolve(path.join('__dirname', '..'));
var sliceDir = null;
var libs = ["Ice", "Glacier2", "IceStorm", "IceGrid"];

function generateTask(name)
{
    return name.toLowerCase() + ":generate";
}
function libTask(name)
{
    return name.toLowerCase() + ":lib";
}
function minLibTask(name)
{
    return name.toLowerCase() + ":lib-min";
}
function libFile(name)
{
    return path.join(root, "lib", name + ".js");
}
function libFileMin(name)
{
    return path.join(root, "lib", name + ".min.js");
}
function srcDir(name)
{
    return "src/" + name;
}
function libCleanTask(lib)
{
    return lib + ":clean";
}
function libWatchTask(lib)
{
    return lib + ":watch";
}

function libFiles(name)
{
    return [
        path.join(root, "lib", name + ".js"),
        path.join(root, "lib", name + ".js.gz"),
        path.join(root, "lib", name + ".min.js"),
        path.join(root, "lib", name + ".min.js.gz")];
}

function mapFiles(name)
{
    return [
        path.join(root, "lib", name + ".js.map"),
        path.join(root, "lib", name + ".js.map.gz"),
        path.join(root, "lib", name + ".min.js.map"),
        path.join(root, "lib", name + ".min.js.map.gz")];
}

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

function libGeneratedFiles(lib, sources)
{
    return sources.slice.map(function(f)
            {
                return path.join(srcDir(lib), path.basename(f, ".ice") + ".js");
            })
        .concat(libFiles(lib))
        .concat(mapFiles(lib))
        .concat([path.join(srcDir(lib), ".depend", "*")]);
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

function generateGulpTasks(gulp) {
    function sliceFile(f)
    {
        return path.join(util.sliceDir, f);
    }

    libs.forEach(
        function(lib)
        {
            var sources = JSON.parse(fs.readFileSync(path.join(srcDir(lib), "sources.json"), {encoding: "utf8"}));

            gulp.task(generateTask(lib),
                function()
                {
                    return gulp.src(sources.slice.map(sliceFile))
                    .pipe(util.slice2js({args: ["--ice","--icejs"], dest: srcDir(lib)}))
                    .pipe(gulp.dest(srcDir(lib)));
                });

            gulp.task(libTask(lib), [generateTask(lib)],
                function()
                {
                    return gulp.src(libSources(lib, sources))
                    .pipe(sourcemaps.init())
                    .pipe(bundle(
                    {
                        srcDir: srcDir(lib),
                        modules: sources.modules,
                        target: libFile(lib)
                    }))
                    .pipe(sourcemaps.write("../lib", {sourceRoot:"/src"}))
                    .pipe(gulp.dest("lib"))
                    .pipe(gzip())
                    .pipe(gulp.dest("lib"));
                });

            gulp.task(minLibTask(lib), [libTask(lib)],
                function()
                {
                    return gulp.src(libFile(lib))
                    .pipe(newer(libFileMin(lib)))
                    .pipe(sourcemaps.init({loadMaps:true, sourceRoot:"./"}))
                    .pipe(uglify({compress:false}))
                    .pipe(extreplace(".min.js"))
                    .pipe(sourcemaps.write("../lib", {includeContent: false}))
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
}

module.exports = function(gulp) {
    generateGulpTasks(gulp);
    return {
        buildTasks : libs.map(minLibTask),
        cleanTasks: libs.map(libCleanTask),
        watchTasks: libs.map(libCleanTask)
    };
};


