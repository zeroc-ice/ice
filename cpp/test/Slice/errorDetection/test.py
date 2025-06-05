# Copyright (c) ZeroC, Inc.

import glob
import os
import re
import shutil

from Util import ClientTestCase, SliceTranslator, TestSuite


class SliceErrorDetectionTestCase(ClientTestCase):
    def runClientSide(self, current):
        testdir = current.testsuite.getPath()
        slice2cpp = SliceTranslator("slice2cpp", quiet=True)

        # Create a temporary directory for all the Slice compilers to write their output to.
        outdir = "{0}/tmp".format(testdir)
        if os.path.exists(outdir):
            shutil.rmtree(outdir)
        os.mkdir(outdir)

        # Get all the '.ice' files in this script's directory.
        files = glob.glob("{0}/*.ice".format(testdir))
        files.sort()
        try:
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
                        raise RuntimeError(
                            "failed (lines1 = {0}, lines2 = {1})!".format(
                                len(lines1), len(lines2)
                            )
                        )

                    regex2 = re.compile("^.*(?=" + os.path.basename(file) + ")")
                    i = 0
                    while i < len(lines1):
                        line1 = regex2.sub("", lines1[i]).strip() # Actual output from slice2cpp
                        line2 = lines2[i].strip()                 # Expected output from .err file
                        if line1 != line2:
                            raise RuntimeError(
                                'failed! (line1 = "{0}", line2 = "{1}"'.format(
                                    line1, line2
                                )
                            )
                        i = i + 1
                    else:
                        current.writeln("ok")
        finally:
            # Make sure we clean up the 'tmp' directory.
            if os.path.exists(outdir):
                shutil.rmtree(outdir)


TestSuite(__name__, [SliceErrorDetectionTestCase()])
