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
    console.log("" + process.argv[0] + " " + path.basename(process.argv[1]) + "\"<modules>\" <files>"); 
}

if(process.argv.length < 4)
{
    usage();
    process.exit(1);
}

var modules = process.argv[2].split(" ");

var files = [];
for(var i = 3; i < process.argv.length; ++i)
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

Parser.add = function(depend, file)
{
    if(file.indexOf("../Ice/") === 0 ||
       file.indexOf("../IceGrid/")  === 0 ||
       file.indexOf("../IceStorm/") === 0 ||
       file.indexOf("../Glacier2/") === 0)
    {
        file = path.resolve(file);
        if(depend.depends.indexOf(file) === -1)
        {
            depend.depends.push(file);
        }
    }    
};

Parser.transverse = function(object, depend, file)
{
    for(key in object)
    {
        var value = object[key];
        if(value !== null && typeof value == "object") 
        {
            Parser.transverse(value, depend, file);

            if(value.type === "CallExpression")
            {
                if(value.callee.name === "require")
                {
                    Parser.add(depend, value.arguments[0].value + ".js");
                }
                else if(value.callee.type == "MemberExpression" &&
                        value.callee.property.name == "require" &&
                        (value.callee.object.name == "__M" || 
                        (value.callee.object.property && value.callee.object.property.name == "__M")))
                {
                    value.arguments[1].elements.forEach(
                        function(arg){
                            Parser.add(depend, arg.value + ".js");
                        });
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
                Parser.transverse(ast, depend, file);
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

//
// Wrap the library in a closure to hold the private __Slice module.
//
var preamble =
    "(function()\n" +
    "{\n";
    
var epilogue =
    "}());\n\n";
    
//
// Wrap contents of each file in a closure to keep local variables local.
//
var modulePreamble =
    "\n" +
    "    (function()\n" +
    "    {\n";
    
var moduleEpilogue =
    "    }());\n";
    
process.stdout.write(preamble);

modules.forEach(
    function(m){
        process.stdout.write("    var " + m + " = window." + m + " || {};\n");
        if(m == "Ice")
        {
            process.stdout.write("    Ice.Slice = Ice.Slice || {};\n");
        }
    });
process.stdout.write("    var Slice = Ice.Slice;");

for(i = 0;  i < length; ++i)
{
    process.stdout.write(modulePreamble);
    file = d.depends[i].realpath;
    data = fs.readFileSync(file); 
    lines = data.toString().split("\n");
    
    var skip = false;
    var skipUntil = undefined;
    var skipAuto = false;
    
    for(j in lines)
    {
        line = lines[j].trim();
        
        if(line == "/* slice2js browser-bundle-skip */")
        {
            skipAuto = true;
            continue;
        }
        if(line == "/* slice2js browser-bundle-skip-end */")
        {
            skipAuto = false;
            continue;
        }
        else if(skipAuto)
        {
            continue;
        }
        
        //
        // Get rid of require statements, the bundle include all required files, 
        // so require statements are not required.
        //
        if(line.match(/var .* require\(".*"\).*;/))
        {
            continue;
        }
        if(line.match(/__M\.require\(/))
        {
            if(line.lastIndexOf(";") === -1)
            {
                // skip until next semicolon
                skip = true;
                skipUntil = ";";
            }
            continue;
        }
        
        //
        // Get rid of assert statements for optimized builds.
        //
        if(optimize && line.match(/Debug\.assert\(/))
        {
            if(line.lastIndexOf(";") === -1)
            {
                // skip until next semicolon
                skip = true;
                skipUntil = ";";
            }
            continue;
        }
        
        //
        // Get rid of __M.module statements, in browser top level modules are
        // global.
        //
        if(line.match(/var .* = __M.module\(/))
        {
            if(line.lastIndexOf(";") === -1)
            {
                // skip until next semicolon
                skip = true;
                skipUntil = ";";
            }
            continue;
        }
        
        if(skip)
        {
            if(line.lastIndexOf(skipUntil) !== -1)
            {
                skip = false;
            }
            continue;
        }
        
        var out = lines[j];
        if(line.indexOf("module.exports.") === 0)
        {
            continue;
        }
        
        if(line.indexOf("__M.type") !== -1)
        {
            out = out.replace(/__M\.type/g, "eval");
        }

        process.stdout.write("        " + out + "\n");
    }
    process.stdout.write(moduleEpilogue);
}
process.stdout.write("\n");
//
// Now exports the modules to the global Window object.
//
modules.forEach(
    function(m){
        process.stdout.write("    window." + m + " = " + m + ";\n");
    });

process.stdout.write(epilogue);
