#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

# TODO review this once the new interceptor test is ported to all mappings
if isinstance(Mapping.getByPath(__name__), CSharpMapping):
    TestSuite(__name__, [ClientServerTestCase(), ClientAMDServerTestCase()])
else:
    TestSuite(__name__, [ClientTestCase(client=Client(props={"Ice.Warn.Dispatch" : 0}))], libDirs=["interceptortest"])
