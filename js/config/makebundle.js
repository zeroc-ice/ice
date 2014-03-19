// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var fs = require('fs');
var path = require('path');
var esprima = require('esprima');

var usage = function()
{
    console.log("usage:");
    console.log("" + process.argv[0] + " " + path.basename(process.argv[1]) + " <files>"); 
}

if(process.argv.length < 3)
{
    usage();
    process.exit(1);
}

var files = [];
for(var i = 2; i < process.argv.length; ++i)
{
    files.push(process.argv[i]);
}

var Depends = function()
{
    this.depends = [];
};

Depends.prototype.get = function(file)
{
    for(var i = 0; i < this.depends.length; ++i)
    {
        var obj = this.depends[i];
        if(obj.file === file)
        {
            return obj.depends;
        }
    }
    return [];
};

Depends.prototype.expand = function(o)
{
    if(o === undefined)
    {
        for(i = 0; i < this.depends.length; ++i)
        {
            this.expand(this.depends[i]);
        }
    }
    else
    {
        var newDepends = o.depends.slice();
        for(var j = 0; j < o.depends.length; ++j)
        {
            var depends = this.get(o.depends[j]);
            for(var k = 0; k < depends.length; ++k)
            {
                if(newDepends.indexOf(depends[k]) === -1)
                {
                    newDepends.push(depends[k]);
                }
            }
        }
        
        if(o.depends.length != newDepends.length)
        {

            o.depends = newDepends;
            this.expand(o);
        }
    }
    return this;
}

Depends.comparator = function(a, b)
{
    // B depends on A
    var i;
    var result = 0;
    for(i = 0; i < b.depends.length; ++i)
    {
        if(b.depends[i] === a.file)
        {
            result = -1;
        }
    }
    // A depends on B
    for(i = 0; i < a.depends.length; ++i)
    {
        if(a.depends[i] === b.file)
        {
            if(result == -1)
            {
                process.stderr.write("warning: circulary dependency between: " + a.file + " and " + b.file + "\n");
                return result;
            }
            result = 1;
        }
    }

    return result;
};

Depends.prototype.sort = function()
{
    var objects = this.depends.slice();
    for(var i = 0; i < objects.length; ++i)
    {
        for(var j = 0; j < objects.length; ++j)
        {
            if(j === i) { continue; }
            var v = Depends.comparator(objects[i], objects[j]);
            if(v < 0)
            {
                var tmp = objects[j];
                objects[j] = objects[i];
                objects[i] = tmp;
            }
        }
    }
    return objects;
};

var Parser = {};

Parser.traverse = function(object, depend, file)
{
    for(key in object)
    {
        var value = object[key];
        if(value !== null && typeof value == "object") 
        {
            Parser.traverse(value, depend, file);

            if(value.type === "CallExpression")
            {
                if(value.callee.name === "require")
                {
                    var includedFile = value.arguments[0].value + ".js";
                    if(includedFile.indexOf("Ice/") === 0 ||
                       includedFile.indexOf("IceWS/") === 0 ||
                       includedFile.indexOf("IceMX/") === 0 ||
                       includedFile.indexOf("IceGrid/")  === 0 ||
                       includedFile.indexOf("IceStorm/") === 0 ||
                       includedFile.indexOf("Glacier2/") === 0)
                    {
                        includedFile = path.resolve("../" + includedFile);
                        if(depend.depends.indexOf(includedFile) === -1)
                        {
                            depend.depends.push(includedFile);
                        }
                    }
                }
            }
        }
    }
};

Parser.dir = function(base, depends)
{
    var d = depends || new Depends();
    for(var i = 0; i < files.length; ++i)
    {
        var file = files[i];
        var stats = fs.statSync(file);
        if(path.extname(file) == ".js" && stats.isFile())
        {
            try
            {
                var dirname = path.basename(path.dirname(file));
                var fullpath;
                if(dirname === "browser")
                {
                    fullpath = path.resolve(path.dirname(file) + "/../" + path.basename(file));
                    if(!fs.existsSync(fullpath))
                    {
                        fullpath = path.resolve(file);
                    }
                }
                else
                {
                    fullpath = path.resolve(file);
                }
                var depend = { realpath: file, file: fullpath, depends: [] };
                d.depends.push(depend);
                var ast = esprima.parse(fs.readFileSync(file, 'utf-8'));
                Parser.traverse(ast, depend, file);
            }
            catch(e)
            {
                throw e;
            }
        }
    }
    return d;
};

var d = Parser.dir("");
d.depends = d.expand().sort();

var file, i, length = d.depends.length, line;
var optimize = process.env.OPTIMIZE && process.env.OPTIMIZE == "yes";

for(i = 0;  i < length; ++i)
{
    file = d.depends[i].realpath;
    data = fs.readFileSync(file); 
    lines = data.toString().split("\n");
    var skip = false;
    for(j in lines)
    {
        line = lines[j].trim();
        //
        // Get rid of require statements, the bundle include all required files, 
        // so require statements are not required.
        //
        if(line == "var require = typeof(module) !== \"undefined\" ? module.require : function(){};")
        {
            continue;
        }
        else if(line.match(/require\(".*"\);/))
        {
            continue;
        }
        
        if(optimize && line.match(/Debug\.assert\(/))
        {
            if(line.lastIndexOf(";") === -1)
            {
                // skip until next semicolon
                skip = true;
            }
            continue;
        }
        if(skip)
        {
            if(line.lastIndexOf(";") !== -1)
            {
                skip = false;
            }
            continue;
        }
        process.stdout.write(lines[j] + "\n");
    }
}
