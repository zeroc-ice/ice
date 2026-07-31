# Copyright (c) ZeroC, Inc.

import json
import os
import xml.etree.ElementTree as ElementTree

from Util import ClientTestCase, SliceTranslator, TestSuite

# The expected dependencies of the compiled sources, keyed by file name.
expectedDependencies = {"c.ice": ["a.ice", "b.ice"], "d.ice": ["a.ice"], "e.ice": ["a.ice"]}

# e.ice is compiled from a directory whose name holds an XML-significant character, so its dependency entry is
# only parsable when the compiler escapes the file names it writes.
escapedDirectory = "a&b"


class SliceDependenciesTestCase(ClientTestCase):
    def runClientSide(self, current):
        slice2js = SliceTranslator("slice2js")

        current.mkdirs(escapedDirectory)
        current.createFile(
            os.path.join(escapedDirectory, "e.ice"),
            [
                '#include "../slices/a.ice"',
                "module Test",
                "{",
                "    interface E",
                "    {",
                "        void op(A a);",
                "    }",
                "}",
            ],
        )

        sources = [
            os.path.join("slices", "c.ice"),
            os.path.join("slices", "d.ice"),
            os.path.join(escapedDirectory, "e.ice"),
        ]

        current.write("testing dependencies in JSON format... ")
        slice2js.run(current, args=["--depend-json", "--depend-file", "depend.json"] + sources)
        with open("depend.json", encoding="utf-8") as file:
            self.checkDependencies(json.load(file))
        os.remove("depend.json")
        current.writeln("ok")

        current.write("testing dependencies in XML format... ")
        slice2js.run(current, args=["--depend-xml", "--depend-file", "depend.xml"] + sources)
        root = ElementTree.parse("depend.xml").getroot()
        self.checkDependencies({source.get("name"): [dependsOn.get("name") for dependsOn in source] for source in root})
        os.remove("depend.xml")
        current.writeln("ok")

    def checkDependencies(self, dependencies):
        # The sources and their dependencies are reported as absolute paths; only compare the file names.
        actual = {
            os.path.basename(source): sorted(os.path.basename(dependency) for dependency in dependsOn)
            for source, dependsOn in dependencies.items()
        }
        if actual != expectedDependencies:
            raise RuntimeError("failed! expected {0} but got {1}".format(expectedDependencies, actual))


TestSuite(__name__, [SliceDependenciesTestCase()], chdir=True)
