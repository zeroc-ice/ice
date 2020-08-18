#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

if isinstance(Mapping.getByPath(__name__), CSharpMapping):
    TestSuite(__name__, [ClientServerTestCase(), ClientAMDServerTestCase(), CollocatedTestCase()])
else:
    TestSuite(__name__, [ClientTestCase(client=Client(props={"Ice.Warn.Dispatch" : 0}))], libDirs=["interceptortest"])
