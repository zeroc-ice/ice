# Copyright (c) ZeroC, Inc.

import os
import shutil

from Util import ClientTestCase, Linux, SliceTranslator, TestSuite, platform


class SliceUtf8BomTestCase(ClientTestCase):
    def runClientSide(self, current):
        test_file_name = "Test.ice"
        slice2cpp = SliceTranslator("slice2cpp", quiet=True)

        # Temporary output directory for 'slice2cpp' to use.
        outdir = "{0}/tmp".format(os.getcwd())
        if os.path.exists(outdir):
            shutil.rmtree(outdir)
        os.mkdir(outdir)

        try:
            # Make sure the leading BOM marker wasn't accidentally deleted by over-zealous reformatting.
            current.write("Checking for UTF8-BOM marker... ")
            with open(test_file_name, "rb") as test_file:
                BOM = test_file.read(3)
                if BOM != b"\xef\xbb\xbf":
                    raise RuntimeError("BOM marker is missing from 'Slice/utf8BOM/Test.ice'!")
            current.writeln("ok")

            # Run 'slice2cpp' over the file and collect its output so we can make sure the BOMs were properly handled.
            current.write("Compiling Slice file with UTF8-BOM marker... ")
            args = [test_file_name, "--output-dir", "tmp"]
            slice2cpp.run(current, args, exitstatus=1)
            output = slice2cpp.getOutput(current)
            lines = output.strip().splitlines()

            # There is a known issue with MCPP on certain Linux platforms which causes the Slice compiler to erroneously
            # report valid UTF-8 BOMS. See https://github.com/zeroc-ice/ice/issues/3940
            if len(lines) == 2 and isinstance(platform, Linux):
                if lines[0].endswith(
                    "Test.ice:2: encountered unexpected UTF-8 BOM in input; BOMs can only appear at the beginning of files"
                ):
                    lines.pop(0)

            # Other than the above-mentioned MCPP issue, we only expect 1 error for the BOM in the middle of the file.
            if len(lines) != 1 or not lines[0].endswith(
                "Test.ice:9: encountered unexpected UTF-8 BOM in input; BOMs can only appear at the beginning of files"
            ):
                raise RuntimeError("Unexpected output from BOM test: {0}".format(lines))

            current.writeln("ok")
        finally:
            if os.path.exists(outdir):
                shutil.rmtree(outdir)


TestSuite(__name__, [SliceUtf8BomTestCase()], chdir=True)
