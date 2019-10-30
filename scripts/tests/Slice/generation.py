#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

class SliceGenerationTestCase(ClientTestCase):

    def runClientSide(self, current):
        current.write("testing list-generated... ")

        slice2java = SliceTranslator("slice2java")
        current.mkdirs("classes")

        slice2java.run(current,
                       args=["--list-generated", "--output-dir", "classes", "File1.ice", "File2.ice"])

        lines1 = slice2java.getOutput(current).strip().split("\n")
        lines2 = open(os.path.join(current.testsuite.getPath(), "list-generated.out"), "r").readlines()
        if len(lines1) != len(lines2):
            raise RuntimeError("failed!")

        i = 0
        while i < len(lines1):
            line1 = lines1[i].strip()
            line2 = lines2[i].strip()
            if line1 != line2:
                raise RuntimeError("failed!")
            i = i + 1
        else:
            current.writeln("ok")

TestSuite(__name__, [ SliceGenerationTestCase() ])
