// Copyright (c) ZeroC, Inc.

// A helper script to bundle TypeScript declarations into a single file and remove __global_ alias.
// This process is necessary for generating Ice for JavaScript documentation using TypeDoc.

import fs from "fs";
import path from "path";
import * as ts from "typescript";
import { fileURLToPath } from "url";

const __dirname = path.dirname(fileURLToPath(import.meta.url));

const entryFilePath = path.resolve(__dirname, "src/index.d.ts");
const outputFilePath = path.resolve(__dirname, "dist/bundle.d.ts");

const visitedFiles = new Set();
const moduleDeclarations = new Map();

function collectReferences(filePath) {
    if (visitedFiles.has(filePath)) {
        return;
    }
    visitedFiles.add(filePath);

    const fileContent = fs.readFileSync(filePath, "utf-8");
    const sourceFile = ts.createSourceFile(filePath, fileContent, ts.ScriptTarget.Latest, true);

    sourceFile.referencedFiles.forEach(reference => {
        const referencePath = path.resolve(path.dirname(filePath), reference.fileName);
        collectReferences(referencePath);
    });

    sourceFile.forEachChild(node => {
        if (ts.isModuleDeclaration(node)) {
            const moduleName = node.name.getText();
            const moduleBody = node.body;

            if (moduleBody && ts.isModuleBlock(moduleBody)) {
                const moduleContent = moduleBody.statements
                    .filter(stmt => !ts.isImportEqualsDeclaration(stmt))
                    .map(stmt => stmt.getText())
                    .join("\n");

                if (!moduleDeclarations.has(moduleName)) {
                    moduleDeclarations.set(moduleName, []);
                }
                moduleDeclarations.get(moduleName).push(moduleContent);
            }
        }
    });
}

function mergeModules() {
    let output = "";

    for (const [moduleName, moduleContents] of moduleDeclarations) {
        output += `declare module ${moduleName} {\n`;
        for (const moduleContent of moduleContents) {
            output += `  ${moduleContent.replace(/__global_/g, "")}\n`;
        }
        output += "}\n";
    }

    return output;
}

collectReferences(entryFilePath);
const mergedContent = mergeModules();

fs.writeFileSync(outputFilePath, mergedContent, "utf-8");
