# Copyright (c) ZeroC, Inc.

from __future__ import annotations

import json
import os
import xml.etree.ElementTree as ElementTree

from Util import ClientTestCase, Driver, SliceTranslator, TestSuite

# e.ice and f.ice sit in a directory whose name holds an XML-significant character, so both the source and the
# dependency entries of e.ice are only parsable when the compiler escapes the file names it writes.
ampersandDirectory = "a&b"

# The expected dependencies of the compiled sources, keyed by the directory and file name of the source.
expectedDependencies = {
    os.path.join("slices", "c.ice"): [os.path.join("slices", "a.ice"), os.path.join("slices", "b.ice")],
    os.path.join("slices", "d.ice"): [os.path.join("slices", "a.ice")],
    os.path.join(ampersandDirectory, "e.ice"): [
        os.path.join("slices", "a.ice"),
        os.path.join(ampersandDirectory, "f.ice"),
    ],
}


class SliceDependenciesTestCase(ClientTestCase):
    def runClientSide(self, current: Driver.Current) -> None:
        slice2js = SliceTranslator("slice2js")

        sources = [
            os.path.join("slices", "c.ice"),
            os.path.join("slices", "d.ice"),
            os.path.join(ampersandDirectory, "e.ice"),
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
        self.checkDependencies(
            {source.attrib["name"]: [dependsOn.attrib["name"] for dependsOn in source] for source in root}
        )
        os.remove("depend.xml")
        current.writeln("ok")

    def checkDependencies(self, dependencies: dict[str, list[str]]) -> None:
        # The sources and their dependencies are reported as absolute paths; compare the directory and file name,
        # so that a directory holding an XML-significant character must come back spelled exactly as it is on disk.
        actual = {
            self.tail(source): sorted(self.tail(dependency) for dependency in dependsOn)
            for source, dependsOn in dependencies.items()
        }
        expected = {source: sorted(dependsOn) for source, dependsOn in expectedDependencies.items()}
        if actual != expected:
            raise RuntimeError("failed! expected {0} but got {1}".format(expected, actual))

    def tail(self, path: str) -> str:
        return os.path.join(os.path.basename(os.path.dirname(path)), os.path.basename(path))


TestSuite(__name__, [SliceDependenciesTestCase()], chdir=True)
