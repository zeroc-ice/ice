# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

import shutil, locale

class SliceUnicodePathsTestCase(ClientTestCase):

    def runClientSide(self, current):

        if isinstance(platform, Linux) or isinstance(platform, AIX):
            encoding = locale.getdefaultlocale()[1]
            if encoding != "UTF-8":
                current.writeln("Please set LC_ALL to xx_xx.UTF-8, for example FR_FR.UTF-8")
                current.writeln("Skipping test")
                return
        elif isinstance(platform, Windows) and isPython2:
            current.writeln("To run this test on Windows you need to be using Python 3.x")
            current.writeln("Python 2.x subprocess module doesn't support unicode on Windows")
            current.writeln("Skipping test")
            return

        current.write("testing Slice compiler and unicode file paths... ")

        srcPath = "./\xe4\xb8\xad\xe5\x9b\xbd" if isPython2 else "./\u4e2d\u56fd"

        if os.path.exists(srcPath): shutil.rmtree(srcPath)
        os.mkdir(srcPath)

        current.createFile("%s/Test.ice" % srcPath,
                           ["module Test { ",
                            "class Point{int x; int y; };",
                            "interface Canvas{ void draw(Point p); };",
                            "};"], "utf-8")

        tests = [
            ("cpp", ["Test.cpp", "Test.h", "TestI.cpp", "TestI.h"], "--impl"),
            ("cs", ["Test.cs", "TestI.cs"], "--impl"),
            ("html", ["index.html"], ""),
            ("java", ["Test/Point.java", "Test/CanvasI.java"], "--impl"),
            ("js", ["Test.js"], ""),
            ("objc", ["Test.m"], "")]

        try:
            for language, generated, args in tests:
                compiler = SliceTranslator('slice2%s' % language)
                if not os.path.isfile(compiler.getCommandLine(current)):
                    continue

                args = [srcPath + "/Test.ice", "--output-dir", srcPath] + args.split(" ")
                compiler.run(current, args=args)

                for f in generated:
                    if not os.path.isfile(os.path.join(srcPath, f)):
                        raise RuntimeError("failed! (can't find {0})".format(os.path.join(srcPath, f)))
                    os.remove(os.path.join(srcPath, f))

            current.writeln("ok")

        finally:
            if os.path.exists(srcPath): shutil.rmtree(srcPath)

TestSuite(__name__, [ SliceUnicodePathsTestCase() ], chdir=True)
