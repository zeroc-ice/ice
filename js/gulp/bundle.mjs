//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import PluginError from "plugin-error";
import through from "through2";
import fs from "fs";
import path from "path";
import sourcemap from "source-map";
import Vinyl from "vinyl";
import esprima from "esprima";

const PLUGIN_NAME = "gulp-slice2js-bundle";

function isnewer(input, output) {
    return fs.statSync(input).mtime.getTime() > fs.statSync(output).mtime.getTime();
}

function isfile(path) {
    try {
        return fs.statSync(path).isFile();
    }
    catch (e) {
        if (e.code == "ENOENT") {
            return false;
        }
        throw e;
    }
}

class Depends {
    constructor() {
        this.depends = [];
    }

    get(file) {
        for (let i = 0; i < this.depends.length; ++i) {
            const obj = this.depends[i];
            if (obj.file.path === file) {
                return obj.depends;
            }
        }
        return [];
    }

    expand(o) {
        if (o === undefined) {
            for (let i = 0; i < this.depends.length; ++i) {
                this.expand(this.depends[i]);
            }
        }
        else {
            const newDepends = o.depends.slice();
            for (let j = 0; j < o.depends.length; ++j) {
                const depends = this.get(o.depends[j]);
                for (let k = 0; k < depends.length; ++k) {
                    if (newDepends.indexOf(depends[k]) === -1) {
                        newDepends.push(depends[k]);
                    }
                }
            }

            if (o.depends.length != newDepends.length) {
                o.depends = newDepends;
                this.expand(o);
            }
        }
        return this;
    }

    static comparator(a, b) {
        // B depends on A
        let result = 0;

        for (let i = 0; i < b.depends.length; ++i) {
            if (b.depends[i] === a.file.path) {
                result = -1;
            }
        }
        // A depends on B
        for (let i = 0; i < a.depends.length; ++i) {
            if (a.depends[i] === b.file.path) {
                if (result == -1) {
                    process.stderr.write("warning: circulary dependency between: " + a.file.path + " and " + b.file.path + "\n");
                    return result;
                }
                result = 1;
            }
        }

        return result;
    }

    sort() {
        const objects = this.depends.slice();
        for (let i = 0; i < objects.length; ++i) {
            for (let j = 0; j < objects.length; ++j) {
                if (j === i) { continue; }
                const v = Depends.comparator(objects[i], objects[j]);
                if (v < 0) {
                    const tmp = objects[j];
                    objects[j] = objects[i];
                    objects[i] = tmp;
                }
            }
        }
        return objects;
    }
}

const Parser = {};

Parser.add = function (depend, file, srcDir) {
    if (file.indexOf("../Ice/") === 0 ||
        file.indexOf("../IceGrid/") === 0 ||
        file.indexOf("../IceStorm/") === 0 ||
        file.indexOf("../Glacier2/") === 0) {
        file = isfile(path.join(srcDir, path.dirname(file), "browser", path.basename(file))) ?
            path.resolve(path.join(srcDir, path.dirname(file), "browser", path.basename(file))) :
            path.resolve(path.join(srcDir, file));

        if (depend.depends.indexOf(file) === -1) {
            depend.depends.push(file);
        }
    }
};

Parser.transverse = function (object, depend, srcDir) {
    function appendfile(arg) {
        Parser.add(depend, arg.value + ".js", srcDir);
    }

    for (var key in object) {
        var value = object[key];
        if (value !== null && typeof value == "object") {
            Parser.transverse(value, depend, srcDir);

            if (value.type === "CallExpression") {
                if (value.callee.name === "require") {
                    Parser.add(depend, value.arguments[0].value + ".js", srcDir);
                }
                else if (value.callee.type == "MemberExpression" &&
                    value.callee.property.name == "require" &&
                    (value.callee.object.name == "_ModuleRegistry" ||
                        (value.callee.object.property && value.callee.object.property.name == "_ModuleRegistry"))) {
                    value.arguments[1].elements.forEach(appendfile);
                }
            }
        }
    }
};

class StringBuffer {
    constructor() {
        this.buffer = Buffer.alloc(0);
    }

    write(data) {
        this.buffer = Buffer.concat([this.buffer, Buffer.from(data, "utf8")]);
    }
}

function sourceMapRelativePath(file) {
    var modules = ["Ice", "Glacier2", "IceStorm", "IceGrid"],
        rel = "",
        filepath = file,
        basename;
    while (true) {
        basename = path.basename(filepath);
        rel = path.join(basename, rel);
        filepath = path.dirname(filepath);
        if (modules.indexOf(basename) != -1) {
            break;
        }
    }
    return rel;
}

function bundle(args) {
    var files = [];

    return through.obj(
        function (file, enc, cb) {
            if (file.isNull()) {
                return;
            }

            if (file.isStream()) {
                return this.emit('error', new PluginError(PLUGIN_NAME, 'Streaming not supported'));
            }

            files.push(file);
            cb();
        },
        function (cb) {
            if (!isfile(args.target) ||
                files.some(function (f) { return isnewer(f.path, args.target); })) {
                var d = new Depends();
                files.forEach(
                    function (file) {
                        var depend = { file: file, depends: [] };
                        d.depends.push(depend);
                        try {
                            Parser.transverse(esprima.parse(file.contents.toString()), depend, args.srcDir);
                        }
                        catch (e) {
                            throw new Error(file.path + ": " + e.toString());
                        }
                    });

                d.depends = d.expand().sort();

                var sourceMap = new sourcemap.SourceMapGenerator(
                    {
                        file: path.basename(args.target)
                    });
                var lineOffset = 0;

                //
                // Wrap the library in a closure to hold the private Slice module.
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

                var sb = new StringBuffer();

                sb.write(preamble);
                sb.write("    var root = typeof(window) !== \"undefined\" ? window : typeof(global) !== \"undefined\" ? global : typeof(self) !== \"undefined\" ? self : {};\n");
                sb.write("    var ice = root.ice || {};\n");
                lineOffset += 4;
                args.modules.forEach(
                    function (m) {
                        sb.write("    root." + m + " = root." + m + " || {};\n");
                        lineOffset++;
                        sb.write("    ice." + m + " = root." + m + ";\n");
                        lineOffset++;
                        if (m == "Ice") {
                            sb.write("    Ice.Slice = Ice.Slice || {};\n");
                            lineOffset++;
                        }
                    });
                sb.write("    var Slice = Ice.Slice;\n");
                lineOffset++;

                for (var i = 0; i < d.depends.length; ++i) {
                    sb.write(modulePreamble);
                    lineOffset += 3;

                    var data = d.depends[i].file.contents.toString();
                    var file = d.depends[i].file;
                    var lines = data.toString().split("\n");

                    var skip = false;
                    var skipUntil;
                    var skipAuto = false;
                    var line;
                    var out;

                    var j = 0;
                    for (j = 0; j < lines.length; j++) {
                        out = lines[j];
                        line = out.trim();

                        if (line == "/* slice2js browser-bundle-skip */") {
                            skipAuto = true;
                            continue;
                        }
                        if (line == "/* slice2js browser-bundle-skip-end */") {
                            skipAuto = false;
                            continue;
                        }
                        else if (skipAuto) {
                            continue;
                        }

                        //
                        // Get rid of require statements, the bundle include all required files,
                        // so require statements are not required.
                        //
                        if (line.match(/const .* require\(".*"\).*;/)) {
                            continue;
                        }
                        if (line.match(/require\(".*"\).*;/)) {
                            continue;
                        }
                        if (line.match(/_ModuleRegistry\.require\(/)) {
                            if (line.lastIndexOf(";") === -1) {
                                // skip until next semicolon
                                skip = true;
                                skipUntil = ";";
                            }
                            continue;
                        }

                        //
                        // Get rid of _ModuleRegistry.module statements, in browser top level modules are
                        // global.
                        //
                        if (line.match(/const .* = _ModuleRegistry.module\(/)) {
                            if (line.lastIndexOf(";") === -1) {
                                // skip until next semicolon
                                skip = true;
                                skipUntil = ";";
                            }
                            continue;
                        }

                        if (skip) {
                            if (line.lastIndexOf(skipUntil) !== -1) {
                                skip = false;
                            }
                            continue;
                        }

                        if (line.indexOf("module.exports.") === 0) {
                            continue;
                        }
                        else if (line.indexOf("exports.") === 0) {
                            continue;
                        }
                        else if (line.indexOf("exports =") === 0) {
                            continue;
                        }

                        sb.write("        " + out + "\n");

                        sourceMap.addMapping(
                            {
                                generated:
                                {
                                    line: lineOffset + 1,
                                    column: 8
                                },
                                original:
                                {
                                    line: j + 1,
                                    column: 0
                                },
                                source: sourceMapRelativePath(file.path)
                            });
                        lineOffset++;
                    }
                    sb.write(moduleEpilogue);
                    lineOffset++;
                }
                sb.write("\n");
                lineOffset++;

                //
                // Now exports the modules to the global object.
                //
                args.modules.forEach(
                    function (m) {
                        sb.write("    root." + m + " = " + m + ";\n");
                        lineOffset++;
                    });

                sb.write("    root.ice = ice;\n");
                lineOffset++;

                sb.write(epilogue);
                lineOffset++;
                var target = new Vinyl(
                    {
                        cwd: "",
                        base: path.dirname(args.target),
                        path: args.target,
                        contents: sb.buffer
                    });
                target.sourceMap = JSON.parse(sourceMap.toString());
                this.push(target);
            }
            cb();
        });
}

export default bundle;
