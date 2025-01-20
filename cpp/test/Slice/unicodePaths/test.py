# Copyright (c) ZeroC, Inc.

import os
import shutil
import locale

from Util import ClientTestCase, platform, TestSuite, SliceTranslator, Linux


class SliceUnicodePathsTestCase(ClientTestCase):
    def runClientSide(self, current):
        if isinstance(platform, Linux):
            encoding = locale.getdefaultlocale()[1]
            if encoding != "UTF-8":
                current.writeln(
                    "Please set LC_ALL to xx_xx.UTF-8, for example FR_FR.UTF-8"
                )
                current.writeln("Skipping test")
                return

        current.write("testing Slice compiler and unicode file paths... ")

        srcPath = "./\u4e2d\u56fd"

        if os.path.exists(srcPath):
            shutil.rmtree(srcPath)
        os.mkdir(srcPath)

        current.createFile(
            "%s/Test.ice" % srcPath,
            [
                "module Test { ",
                "class Point{int x; int y; };",
                "interface Canvas{ void draw(Point p); };",
                "};",
            ],
            "utf-8",
        )

        tests = [
            ("cpp", ["Test.cpp", "Test.h"]),
            ("cs", ["Test.cs"]),
            ("html", ["index.html"]),
            ("java", ["Test/Point.java"]),
            ("js", ["Test.js"]),
            ("php", ["Test.php"]),
        ]

        try:
            for language, generated in tests:
                compiler = SliceTranslator("slice2%s" % language)
                if not os.path.isfile(compiler.getCommandLine(current)):
                    continue

                args = [srcPath + "/Test.ice", "--output-dir", srcPath]
                compiler.run(current, args=args)

                for f in generated:
                    if not os.path.isfile(os.path.join(srcPath, f)):
                        raise RuntimeError(
                            "failed! (can't find {0})".format(os.path.join(srcPath, f))
                        )
                    os.remove(os.path.join(srcPath, f))

            current.writeln("ok")

        finally:
            if os.path.exists(srcPath):
                shutil.rmtree(srcPath)


TestSuite(__name__, [SliceUnicodePathsTestCase()], chdir=True)
