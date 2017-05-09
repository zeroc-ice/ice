# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import glob

class SliceErrorDetectionTestCase(ClientTestCase):

    def runClientSide(self, current):
        testdir = self.getPath()
        slice2cpp = SliceTranslator("slice2cpp")

        files = glob.glob("{0}/*.ice".format(testdir))
        files.sort()
        for file in files:
            current.write(os.path.basename(file) + "... ")

            args = ["-I.", file, "--output-dir", "tmp"]
            if file.find("Underscore") >= 0:
                args.append("--underscore")

            # Don't print out slice2cpp output and expect failures
            slice2cpp.run(current, args=args, exitstatus=0 if file.find("Warning") >= 0 else 1)
            output = slice2cpp.getOutput(current)

            regex1 = re.compile("\.ice$", re.IGNORECASE)
            lines1 = output.strip().splitlines()
            lines2 = open(os.path.join(testdir, regex1.sub(".err", file)), "r").readlines()
            if len(lines1) != len(lines2):
                raise RuntimeError("failed (lines1 = {0}, lines2 = {1})!".format(len(lines1), len(lines2)))

            regex2 = re.compile("^.*(?=" + os.path.basename(file) + ")")
            i = 0
            while i < len(lines1):
                line1 = regex2.sub("", lines1[i]).strip()
                line2 = regex2.sub("", lines2[i]).strip()
                if line1 != line2:
                    raise RuntimeError("failed! (line1 = \"{0}\", line2 = \"{1}\"".format(line1, line2))
                i = i + 1
            else:
                current.writeln("ok")

TestSuite(__name__, [ SliceErrorDetectionTestCase() ])
