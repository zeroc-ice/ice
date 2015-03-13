// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var gutil = require("gulp-util");
var PluginError = gutil.PluginError;
var PLUGIN_NAME = "gulp-slice2js";

var through = require("through2");
var spawn = require("child_process").spawn;
var fs = require("fs");
var path = require("path");

function rmfile(path)
{
    try
    {
        fs.unlinkSync(path);
    }
    catch(e)
    {
    }
}

function mkdir(path)
{
    try
    {
        fs.mkdirSync(path);
    }
    catch(e)
    {
        if(e.code != "EEXIST")
        {
            throw e;
        }
    }
}

function isfile(path)
{
    try
    {
        return fs.statSync(path).isFile();
    }
    catch(e)
    {
        if(e.code == "ENOENT")
        {
            return false;
        }
        throw e;
    }
    return false;
}

var defaultCompileArgs = ["--stdout"];
var defaultDependArgs = ["--depend-json"];

function isnewer(input, output)
{
    return fs.statSync(input).mtime.getTime() > fs.statSync(output).mtime.getTime();
}

function isBuildRequired(inputFile, outputFile, dependFile)
{
    if(![inputFile, outputFile, dependFile].every(isfile) || isnewer(inputFile, outputFile))
    {
        return true;
    }

    function isnewerthan(f)
    {
        return isnewer(f, outputFile);
    }

    var depend = JSON.parse(fs.readFileSync(dependFile, {encoding: "utf8"}));
    for(var key in depend)
    {
        if(path.normalize(key) == path.normalize(inputFile))
        {
            return depend[key].some(isnewerthan);
        }
    }
    return false;
}

function compile(slice2js, file, args, cb)
{
    //
    // Let non Slice files pass-through
    //
    if(path.extname(file.path) != ".ice")
    {
        cb(null, file);
        return;
    }

    var p  = slice2js(args.concat(defaultCompileArgs).concat([file.path]));

    var buffer = new Buffer(0);
    p.stdout.on("data", function(data)
        {
            buffer = Buffer.concat([buffer, data]);
        });

    p.stderr.on("data", function(data)
        {
            gutil.log("'slice2js error'", data.toString());
        });

    p.on('close', function(code)
        {
            if(code === 0)
            {
                file.path = gutil.replaceExtension(file.path, ".js");
                file.contents = buffer;
                cb(null, file);
            }
            else
            {
                cb(new PluginError(PLUGIN_NAME, 'slice2js exit with error code: ' + code));
            }
        });
}

module.exports.compile = function(options)
{
    var opts = options || {};
    var slice2js;
    var args = opts.args || [];

    if(!opts.exe)
    {
        try
        {
            slice2js = require("zeroc-slice2js").compile;
        }
        catch(e)
        {
        }
    }

    if(!slice2js)
    {
        slice2js = function(args)
            {
                return spawn(opts.exe || "slice2js", args);
            };
    }

    return through.obj(function(file, enc, cb)
        {
            if(file.isNull())
            {
                cb();
            }
            else if(file.isStream())
            {
                cb(new PluginError(PLUGIN_NAME, 'Streaming not supported'));
            }
            else if(opts.dest)
            {
                var outputFile = path.join(file.cwd, opts.dest, path.basename(file.path, ".ice") + ".js");
                var dependFile = path.join(path.dirname(outputFile), ".depend", path.basename(outputFile, ".js") + ".d");

                if(isBuildRequired(file.path, outputFile, dependFile))
                {
                    [outputFile, dependFile].forEach(rmfile);
                    var build  = slice2js(args.concat(defaultDependArgs).concat([file.path]));
                    mkdir(path.dirname(dependFile));
                    var buffer = new Buffer(0);
                    build.stdout.on("data", function(data)
                        {
                            buffer = Buffer.concat([buffer, data]);
                        });

                    build.stderr.on("data", function(data)
                        {
                            gutil.log("'slice2js error'", data.toString());
                        });

                    build.on('close', function(code)
                        {
                            if(code === 0)
                            {
                                fs.writeFileSync(dependFile, buffer);
                                compile(slice2js, file, args, cb);
                            }
                            else
                            {
                                cb(new PluginError(PLUGIN_NAME, 'slice2js exit with error code: ' + code));
                            }
                        });
                }
                else
                {
                    cb();
                }
            }
            else
            {
                compile(slice2js, file, args, cb);
            }
        });
};

module.exports.sliceDir = (function() {
    try
    {
        return require('zeroc-slice2js').sliceDir;
    }
    catch(e)
    {
        return null;
    }
})();
