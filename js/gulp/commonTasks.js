// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var browserSync = require("browser-sync");
var concat      = require('gulp-concat');
var del         = require("del");
var gzip        = require('gulp-gzip');
var minifycss   = require('gulp-minify-css');
var newer       = require('gulp-newer');
var path        = require('path');
var uglify      = require("gulp-uglify");

var util = require('./util');

module.exports = function(gulp) {
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
          .pipe(util.slice2js({dest: "test/Common"}))
          .pipe(gulp.dest("test/Common"));
      });

    gulp.task("common:slice:clean", [],
        function()
        {
          del(["test/Common/Controller.js"]);
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
};
