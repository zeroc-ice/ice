# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

import glob

class SliceHeadersTestCase(ClientTestCase):

    def runClientSide(self, current):
        self.clean()

        slice2cpp = SliceTranslator("slice2cpp")

        os.symlink("slices", "linktoslices")
        os.symlink("dir1", os.path.join("slices", "linktodir1"))
        os.symlink("a3.ice", os.path.join("slices", "dir1", "linktoa3.ice"))
        os.symlink("dir2", os.path.join("slices", "linktodir2"))

        basedir = self.testsuite.getPath()
        slicedir = component.getSliceDir(self.mapping, current)
        os.symlink(slicedir, "iceslices")

        def runTest(args):
            slice2cpp.run(current, args=args.split(" "))
            f = open("b.h")
            if not re.search('#include <dir1\/a1\.h>\n'
                             '#include <linktodir1\/a2\.h>\n'
                             '#include <linktodir1\/linktoa3\.h>\n'
                             '#include <Ice\/Identity\.h>\n', f.read(), re.S):
                raise RuntimeError("failed!")

            os.unlink("b.h")
            os.unlink("b.cpp")

        current.write("compiling slice files and checking headers... ")
        runTest("-Iiceslices -Islices slices/dir2/b.ice")
        runTest("-Iiceslices -I../headers/slices slices/dir2/b.ice")
        runTest("-Iiceslices -Ilinktoslices slices/dir2/b.ice")
        runTest("-Iiceslices -Ilinktoslices/../linktoslices slices/dir2/b.ice")
        runTest("-I%s -Islices linktoslices/dir2/b.ice" % slicedir)
        runTest("-I%s -Ilinktoslices linktoslices/linktodir2/b.ice" % slicedir)

        # Also ensure it works with case insensitive file system
        if os.path.exists("SLICES"):
            runTest("-IICESLICES -ISLICES SLICES/DIR2/B.ice")
            runTest("-IICESLICES -ILINKTOSLICES LINKTOSLICES/LINKTODIR2/B.ice")

        slice2cpp = slice2cpp.getCommandLine(current)

        #
        # Slice files are symlinks, include dir is a regular directory
        #
        os.system("mkdir -p project1/git/services.settings.slices")
        os.system("mkdir -p project1/src/services/settings")
        os.system("cd project1/src/services/settings &&  ln -s ../../../git/services.settings.slices slices")

        f = open("project1/git/services.settings.slices/A.ice", "w")
        f.write("// dumy file")
        f.close()
        f = open("project1/git/services.settings.slices/B.ice", "w")
        f.write("#include <services/settings/slices/A.ice>")
        f.close()

        os.system("cd project1 && %s -Isrc src/services/settings/slices/B.ice" % slice2cpp)
        f = open("project1/B.h")
        if not re.search(re.escape('#include <services/settings/slices/A.h>'), f.read()):
            raise RuntimeError("failed!")

        self.clean()

        #
        # Slice file is regular file, include dir is a symlink to a second symlink
        #
        os.system("mkdir -p tmp/Ice-x.y.z/share")
        os.system("cd tmp/Ice-x.y.z/share && ln -s %s" % slicedir)

        os.system("mkdir -p project1/share")
        os.system("cd project1/share && ln -s %s/tmp/Ice-x.y.z/share/slice" % basedir)
        f = open("project1/A.ice", "w")
        f.write("#include <Ice/Identity.ice>")
        f.close()
        os.system("cd project1 && %s -Ishare/slice A.ice" % slice2cpp)
        f = open("project1/A.h")
        if not re.search(re.escape('#include <Ice/Identity.h>'), f.read()):
            raise RuntimeError("failed!")

        self.clean()

        #
        # Typical Ice install with symlink Ice-x.y -> Ice-x.y.z
        #
        os.system("mkdir -p tmp/Ice-x.y.z/slice/Ice")
        os.system("cd tmp && ln -s Ice-x.y.z Ice-x.y")
        f = open("tmp/Ice-x.y.z/slice/Ice/Identity.ice", "w")
        f.write("// dumy file")

        os.system("mkdir -p project1")
        f = open("project1/A.ice", "w")
        f.write("#include <Ice/Identity.ice>")
        f.close()
        os.system("cd project1 && %s -I%s/tmp/Ice-x.y/slice A.ice" % (slice2cpp, basedir))
        f = open("project1/A.h")
        if not re.search(re.escape('#include <Ice/Identity.h>'), f.read()):
            raise RuntimeError("failed!")

        self.clean()

        #
        # symlink directory with extra / at end
        #
        #
        os.system("mkdir -p tmp/Ice-x.y.z/slice/Ice")
        os.system("mkdir -p tmp/Ice")
        os.system("cd tmp/Ice && ln -s ../Ice-x.y.z/slice/ .")
        f = open("tmp/Ice-x.y.z/slice/Ice/Identity.ice", "w")
        f.write("// dumy file")
        f.close()
        os.system("mkdir -p project1")
        f = open("project1/A.ice", "w")
        f.write("#include <Ice/Identity.ice>")
        f.close()
        os.system("cd project1 && %s -I%s/tmp/Ice/slice A.ice" % (slice2cpp, basedir))
        f = open("project1/A.h")
        if not re.search(re.escape('#include <Ice/Identity.h>'), f.read()):
            raise RuntimeError("failed!")
        self.clean()

        current.writeln("ok")

    def teardownClientSide(self, current, success):
        self.clean()

    def clean(self):
        for f in ["iceslices",
                  "linktoslices",
                  os.path.join("slices", "linktodir2"),
                  os.path.join("slices", "linktodir1"),
                  os.path.join("slices", "dir1", "linktoa3.ice")]:
            if os.path.lexists(f):
                os.unlink(f)

        #
        # rmtree can fail when tests are running from a NFS volumen
        # with an error like:
        #
        # Device or resource busy: 'project1/.nfs00000000006216b500000024'
        #
        try:
            if os.path.exists("project1"): shutil.rmtree("project1")
        except:
            pass
        try:
            if os.path.exists("tmp"): shutil.rmtree("tmp")
        except:
            pass

if not isinstance(platform, Windows):
    TestSuite(__name__, [ SliceHeadersTestCase() ], chdir=True)
