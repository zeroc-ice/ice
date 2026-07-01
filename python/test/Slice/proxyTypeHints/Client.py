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


def classNamed(tree: ast.Module, name: str) -> ast.ClassDef:
    nodes = [node for node in ast.walk(tree) if isinstance(node, ast.ClassDef) and node.name == name]
    test(len(nodes) == 1)
    return nodes[0]


def methodNamed(classNode: ast.ClassDef, name: str) -> ast.FunctionDef:
    methods = [node for node in classNode.body if isinstance(node, ast.FunctionDef) and node.name == name]
    test(len(methods) == 1)
    return methods[0]


def paramAnnotation(method: ast.FunctionDef, name: str) -> str:
    for arg in method.args.args:
        if arg.arg == name and arg.annotation is not None:
            return ast.unparse(arg.annotation)
    return ""


class Client(TestHelper):
    def run(self, args: list[str]):
        sys.stdout.write("testing that proxy method signatures honor parameter metadata... ")
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
                        py_compile.compile(path, doraise=True)
                        if name == "ProxyHints.py":
                            interfaceFiles.append(path)
            test(len(interfaceFiles) == 1)

            with open(interfaceFiles[0], encoding="utf-8") as f:
                tree = ast.parse(f.read())

            # The "python:numpy.ndarray" metadata is on the parameter, so the generated proxy method
            # signature must include the NumPy type hint, just like the servant abstract method.
            for className in ["ProxyHintsPrx", "ProxyHints"]:
                op = methodNamed(classNamed(tree, className), "op")
                test("NDArray" in paramAnnotation(op, "values"))

        print("ok")
