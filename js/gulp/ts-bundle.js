// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

const fs = require("fs");
const ts = require("typescript");
const path = require("path");
const os = require("os");
const PluginError = require("plugin-error");
const PLUGIN_NAME = "gulp-ts-bundle";
const through = require("through2");

const namespaces = [""];
const definitions = new Map();
definitions.set("", "");
let namespace = "";

const copyrightHeader = `// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************`;

function visitFile(file, data)
{
    visitNode(ts.createSourceFile(file, data, ts.ScriptTarget.ES7, true));
}

function exportDefinition(node)
{
    let out = definitions.get(namespace);
    const data = node.getText().trim();
    if(data != "")
    {
        out += os.EOL;
        if(node.jsDoc)
        {
            out += "/**" + os.EOL;
            for(const l of node.jsDoc[0].comment.split(os.EOL))
            {
                out += " * " + l + os.EOL;
            }
            out += " */" + os.EOL;
        }
        if(data.indexOf("export ") == -1)
        {
            out += "export ";
        }
        out += data;
        out += os.EOL;
        definitions.set(namespace, out);
    }
}

function visitNode(node, parent)
{
    switch(node.kind)
    {
        case ts.SyntaxKind.SourceFile:
        {

            for(const s of node.referencedFiles)
            {
                const f = path.normalize(path.join(path.dirname(node.fileName), s.fileName));
                visitFile(f, fs.readFileSync(f).toString());
            }
            if(node.referencedFiles == 0)
            {
                ts.forEachChild(node, child =>
                                {
                                    visitNode(child, node);
                                });
            }
            break;
        }
        case ts.SyntaxKind.ModuleDeclaration:
        {
            if(node.modifiers && node.modifiers.some(n => n.kind == ts.SyntaxKind.DeclareKeyword))
            {
                if(node.name.text == "ice")
                {
                    ts.forEachChild(node.body, child =>
                                    {
                                        visitNode(child, node);
                                    });
                }
            }
            else if((parent.modifiers && parent.modifiers.some(n => n.kind == ts.SyntaxKind.DeclareKeyword)))
            {
                namespace = node.name.text;
                if(namespaces.indexOf(namespace) == -1)
                {
                    namespaces.push(namespace);
                    definitions.set(namespace, "");
                }
                ts.forEachChild(node.body, child =>
                {
                    visitNode(child, node);
                });
                namespace = "";
            }
            else
            {
                exportDefinition(node);
            }
            break;
        }
        case ts.SyntaxKind.ClassDeclaration:
        {
            exportDefinition(node);
            break;
        }
        default:
        {
            exportDefinition(node);
            break;
        }
    }
}

class StringBuffer
{
    constructor()
    {
        // Use new Buffer.alloc(string, encoding) if available, Buffer constructor are deprecated.
        this.buffer = typeof Buffer.alloc === 'function' ? Buffer.alloc(0) : new Buffer(0);
    }

    writeLine(data)
    {
        data += os.EOL;
        // Use new Buffer.from(string, encoding) if Buffer.from is avilable, Buffer constructors are deprecated.
        // NOTE: we don't check for Buffer.from which already exists but only accepts array.
        this.buffer = Buffer.concat([this.buffer,
                                     typeof Buffer.from === 'function' ? Buffer.from(data, "utf8") :
                                     new Buffer(data, "utf8")]);
    }
}

function bundle(args)
{
    return through.obj(
        function(file, enc, cb)
        {
            if(file.isNull())
            {
                return;
            }

            if(file.isStream())
            {
                return this.emit('error', new PluginError(PLUGIN_NAME, 'Streaming not supported'));
            }

            visitFile(file.path, file.contents.toString());
            const b = new StringBuffer();
            b.writeLine(copyrightHeader);
            b.writeLine("");
            b.writeLine(`export as namespace ice;`);
            for(const name of namespaces)
            {
                if(name == "")
                {
                    b.writeLine(definitions.get(name));
                }
                else
                {
                    b.writeLine(`export namespace ${name}`);
                    b.writeLine("{");
                    b.writeLine(definitions.get(name));
                    b.writeLine("}");
                }
            }
            file.contents = b.buffer;
            cb(null, file);
        });
}

module.exports = bundle;
