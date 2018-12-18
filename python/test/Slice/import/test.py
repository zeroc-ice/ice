# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

class SliceImportTestCase(ClientTestCase):

    def setupClientSide(self, current):

        testdir = current.testsuite.getPath()
        if os.path.exists(os.path.join(testdir, "Test1_ice.py")):
            os.remove(os.path.join(testdir, "Test1_ice.py"))
        if os.path.exists(os.path.join(testdir, "Test2_ice.py")):
            os.remove(os.path.join(testdir, "Test2_ice.py"))
        if os.path.exists(os.path.join(testdir, "Test")):
            shutil.rmtree(os.path.join(testdir, "Test"))

        slice2py = SliceTranslator("slice2py")
        slice2py.run(current, args=["Test1.ice"])
        slice2py.run(current, args=["Test2.ice"])

TestSuite(__name__, [ SliceImportTestCase() ])
