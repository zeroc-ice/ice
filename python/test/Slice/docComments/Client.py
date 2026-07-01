#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import os
import py_compile
import sys
import tempfile

from TestHelper import TestHelper, test

import Ice
import IcePy


class Client(TestHelper):
    def run(self, args: list[str]):
        sys.stdout.write("testing that doc-comments are escaped in generated docstrings... ")
        sys.stdout.flush()

        sliceFile = os.path.join(os.path.dirname(os.path.abspath(__file__)), "Test.ice")
        with tempfile.TemporaryDirectory() as outputDir:
            rc = IcePy.compileSlice(["slice2py", f"-I{Ice.getSliceDir()}", "--output-dir", outputDir, sliceFile])
            test(rc == 0)

            sawEscapedQuotes = False
            for root, _, files in os.walk(outputDir):
                for name in files:
                    if name.endswith(".py"):
                        path = os.path.join(root, name)
                        # The doc-comments contain '"""' and backslash escapes; if they were emitted
                        # verbatim the generated file would not be valid Python.
                        py_compile.compile(path, doraise=True)
                        if '\\"\\"\\"' in open(path, encoding="utf-8").read():
                            sawEscapedQuotes = True

            # Make sure we actually exercised the escaping path rather than silently dropping the text.
            test(sawEscapedQuotes)

        print("ok")
