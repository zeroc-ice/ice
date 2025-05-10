# Copyright (c) ZeroC, Inc.

import glob
import os
import re
import shutil

from Util import ClientTestCase, SliceTranslator, TestSuite


class SliceErrorDetectionTestCase(ClientTestCase):
    def runClientSide(self, current):
        testdir = current.testsuite.getPath()

        # Create a temporary directory for all the Slice compilers to write their output to.
        outdir = "{0}/tmp".format(testdir)
        if os.path.exists(outdir):
            shutil.rmtree(outdir)
        os.mkdir(outdir)

        try:
            self.testForExpectedErrors(current)
            self.testThatSliceCompilesWithNoErrors(current)
        finally:
            # Make sure we clean up the 'tmp' directory.
            if os.path.exists(outdir):
                shutil.rmtree(outdir)

    def testForExpectedErrors(self, current):
        testdir = current.testsuite.getPath()
        slice2cpp = SliceTranslator("slice2cpp", quiet=True)

        # Get all the '.ice' files in this script's directory.
        files = glob.glob("{0}/*.ice".format(testdir))
        files.sort()

        for file in files:
            current.write(os.path.basename(file) + "... ")

            args = ["-I.", file, "--output-dir", "tmp"]

            # Don't print out slice2cpp output and expect failures
            slice2cpp.run(
                current, args=args, exitstatus=0 if file.find("Warning") >= 0 else 1
            )
            output = slice2cpp.getOutput(current)

            regex1 = re.compile(r"\.ice$", re.IGNORECASE)
            lines1 = output.strip().splitlines()
            with open(os.path.join(testdir, regex1.sub(".err", file)), "r") as f:
                lines2 = f.readlines()
                if len(lines1) != len(lines2):
                    current.writeln("lines1 = {0}".format(lines1))
                    current.writeln("lines2 = {0}".format(lines2))
                    raise RuntimeError("failed (lines1 = {0}, lines2 = {1})!".format(len(lines1), len(lines2)))

                regex2 = re.compile("^.*(?=" + os.path.basename(file) + ")")
                i = 0
                while i < len(lines1):
                    line1 = regex2.sub("", lines1[i]).strip() # Actual output from slice2cpp
                    line2 = regex2.sub("", lines2[i]).strip() # Expected output from
                    if line1 != line2:
                        raise RuntimeError('failed! (line1 = "{0}", line2 = "{1}"'.format(line1, line2))
                    i = i + 1
                else:
                    current.writeln("ok")


    def testThatSliceCompilesWithNoErrors(self, current):
        testdir = current.testsuite.getPath()

        # Get all '.ice' files in the top-level 'slice' folder.
        sliceDir = "{0}/../../../../slice".format(testdir)
        files = glob.glob(sliceDir + "/**/*.ice", recursive=True)
        files.sort()

        # Verify that each compiler can successfully compile those Slice files.
        for compilerName in [
            "slice2cpp",
            "slice2cs",
            "slice2java",
            "slice2js",
            "slice2matlab",
            "slice2php",
            "slice2py",
            "slice2rb",
            "slice2swift",
            "ice2slice"
        ]:
            compiler = SliceTranslator(compilerName)
            failures = []

            current.write("testing " + compilerName + "... " )
            for file in files:
                try:
                    compiler.run(current, args=[file, "--output-dir", "tmp", "-I" + sliceDir])
                except RuntimeError:
                    failures.append(file)

            current.writeln("failed!" if failures else "ok")
            if failures:
                for failure in failures:
                    current.writeln("    failed to compile '" + failure + "'")
                raise RuntimeError(compilerName + " failed to compile files in './slice'")


TestSuite(__name__, [SliceErrorDetectionTestCase()])
