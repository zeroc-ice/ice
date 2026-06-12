#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import ast
import os
import py_compile
import sys
import tempfile

from TestHelper import TestHelper, test

import Ice
import IcePy


def importsModule(nodes: list[ast.stmt], module: str) -> bool:
    return any(
        isinstance(node, ast.Import) and any(alias.name == module for alias in node.names) for node in nodes
    )


def typeCheckingSuite(tree: ast.Module) -> list[ast.stmt]:
    for node in tree.body:
        if isinstance(node, ast.If) and isinstance(node.test, ast.Name) and node.test.id == "TYPE_CHECKING":
            return node.body
    return []


class Client(TestHelper):
    def run(self, args: list[str]):
        sys.stdout.write("testing placement of typing-only imports... ")
        sys.stdout.flush()

        sliceFile = os.path.join(os.path.dirname(os.path.abspath(__file__)), "Test.ice")
        with tempfile.TemporaryDirectory() as outputDir:
            rc = IcePy.compileSlice(["slice2py", f"-I{Ice.getSliceDir()}", "--output-dir", outputDir, sliceFile])
            test(rc == 0)

            interfaceFiles = []
            for root, _, files in os.walk(outputDir):
                for name in files:
                    if name.endswith(".py"):
                        path = os.path.join(root, name)
                        # Every generated file must compile (e.g. no empty TYPE_CHECKING suite).
                        py_compile.compile(path, doraise=True)
                        if name == "TypingImports.py":
                            interfaceFiles.append(path)
            test(len(interfaceFiles) == 1)

            with open(interfaceFiles[0], encoding="utf-8") as f:
                tree = ast.parse(f.read())

            # numpy is only used in type hints, so it must be imported in the TYPE_CHECKING
            # block, not at run time.
            test(not importsModule(tree.body, "numpy"))
            test(importsModule(typeCheckingSuite(tree), "numpy"))

        print("ok")
