# Copyright (c) ZeroC, Inc.

from __future__ import annotations

import json
import os
import re
import xml.etree.ElementTree as ElementTree

from Util import ClientTestCase, Driver, SliceTranslator, TestSuite

# e.ice and f.ice sit in a directory whose name holds an XML-significant character, so both the source and the
# dependency entries of e.ice are only parsable when the compiler escapes the file names it writes.
ampersandDirectory = "a&b"

# g.ice sits in a directory whose name holds a space, which separates prerequisites in Makefile rules, so its rule
# only parses back into complete paths when the compiler escapes the file names it writes.
spaceDirectory = "my project"

# The expected dependencies of the compiled sources, keyed by the directory and file name of the source.
expectedDependencies = {
    os.path.join("slices", "c.ice"): [os.path.join("slices", "a.ice"), os.path.join("slices", "b.ice")],
    os.path.join("slices", "d.ice"): [os.path.join("slices", "a.ice")],
    os.path.join(ampersandDirectory, "e.ice"): [
        os.path.join("slices", "a.ice"),
        os.path.join(ampersandDirectory, "f.ice"),
    ],
    os.path.join(spaceDirectory, "g.ice"): [os.path.join("slices", "a.ice")],
}


class SliceDependenciesTestCase(ClientTestCase):
    def runClientSide(self, current: Driver.Current) -> None:
        slice2js = SliceTranslator("slice2js")

        sources = [
            os.path.join("slices", "c.ice"),
            os.path.join("slices", "d.ice"),
            os.path.join(ampersandDirectory, "e.ice"),
            os.path.join(spaceDirectory, "g.ice"),
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

        current.write("testing dependencies in Makefile format... ")
        slice2js.run(current, args=["--depend", "--depend-file", "depend.mk"] + sources)
        with open("depend.mk", encoding="utf-8") as file:
            rules = self.parseMakefileRules(file.read())

        # Each source produces one rule: the target is derived from the source's file name, and the source itself is
        # the rule's first prerequisite.
        dependencies = {}
        for target, prerequisites in rules.items():
            if not prerequisites:
                raise RuntimeError("failed! the rule for {0} has no prerequisites".format(target))
            source = prerequisites[0]
            if target != os.path.basename(source).replace(".ice", ".js"):
                raise RuntimeError("failed! expected a rule for {0} but got {1}".format(source, target))
            dependencies[source] = prerequisites[1:]
        self.checkDependencies(dependencies)
        os.remove("depend.mk")
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

    def parseMakefileRules(self, text: str) -> dict[str, list[str]]:
        # Join the continuation lines, then split each rule on its first ':' into a target and its prerequisites.
        # Prerequisites are separated by unescaped whitespace; unescape the file names once they are split apart.
        rules = {}
        for line in text.replace("\\\n", "").splitlines():
            if line.strip():
                target, _, prerequisites = line.partition(":")
                rules[self.unescape(target)] = [
                    self.unescape(prerequisite) for prerequisite in re.split(r"(?<!\\)\s+", prerequisites.strip())
                ]
        return rules

    def unescape(self, path: str) -> str:
        return path.replace("\\ ", " ").replace("\\#", "#").replace("$$", "$")


TestSuite(__name__, [SliceDependenciesTestCase()], chdir=True)
