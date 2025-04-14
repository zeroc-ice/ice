# Copyright (c) ZeroC, Inc.

import os
import shutil

from Util import ClientTestCase, SliceTranslator, TestSuite


class SliceUtf8BomTestCase(ClientTestCase):
    def runClientSide(self, current):
        test_file_name = "Test.ice"
        slice2cpp = SliceTranslator("slice2cpp")

        # Temporary output directory for 'slice2cpp to use.
        outdir = "{0}/tmp".format(os.getcwd())
        if os.path.exists(outdir):
            shutil.rmtree(outdir)
        os.mkdir(outdir)

        try:
            # Make sure the BOM marker wasn't accidentally deleted by over-zealous reformatting.
            current.write("Checking for UTF8-BOM marker... ")
            with open(test_file_name, 'rb') as test_file:
                BOM = test_file.read(3)
                if (BOM[0] != 0xEF or BOM[1] != 0xBB or BOM[2] != 0xBF):
                    raise RuntimeError("BOM marker is missing from 'Slice/utf8BOM/Test.ice'!")
            current.writeln("ok")

            # Run 'slice2cpp' over the file to make sure the parser can correctly handle the BOM.
            # We expect a '0' exit code (indicating success) and nothing to be output to the console.
            current.write("Compiling Slice file with UTF8-BOM marker... ")
            args = [test_file_name, "--output-dir", "tmp"]
            slice2cpp.run(current, args, exitstatus=0)
            if slice2cpp.getOutput(current):
                raise RuntimeError("Unexpected output when compiling 'Slice/utf8BOM/Test.ice'!")
            current.writeln("ok")
        finally:
            if os.path.exists(outdir):
                shutil.rmtree(outdir)


TestSuite(__name__, [SliceUtf8BomTestCase()], chdir=True)
