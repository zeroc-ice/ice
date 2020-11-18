#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

#
# Use a server stack size of 512KB, this is in particular important for Java
# servers which don't implement Ice.ClassGraphMaxDepth and which could cause
# a client stack overflow if the client stack is too small compared to the
# Java server stack.
#
class ObjectClientServerTestCase(ClientServerTestCase):

    def getProps(self, process, current):
        props = ClientServerTestCase.getProps(self, process, current)
        if process.getMapping(current) in ["java"] and isinstance(process, Server):
            props["Ice.ThreadPool.Server.StackSize"] = 512 * 1024
        elif current.config.buildPlatform == "iphoneos":
            #
            # Use a 768KB thread stack size for the objects test. This is necessary when running the
            # test on arm64 devices with a debug Ice libraries which require lots of stack space.
            #
            props["Ice.ThreadPool.Server.StackSize"] = 768 * 1024
        elif current.config.buildPlatform == "ppc":
            #
            # The AIX ppc default stack size is too small
            #
            props["Ice.ThreadPool.Server.StackSize"] = 256 * 1024
        elif current.config.buildPlatform == "ppc64":
            props["Ice.ThreadPool.Server.StackSize"] = 768 * 1024
        return props

testcases = [
    ObjectClientServerTestCase("client/server with compact format"),
    ObjectClientServerTestCase("client/server with sliced format", props = {"Ice.Default.SlicedFormat" : True}),
]

if Mapping.getByPath(__name__).hasSource("Ice/objects", "collocated"):
    testcases += [ CollocatedTestCase() ]

TestSuite(__name__, testcases)
