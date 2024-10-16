#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

from DataStormUtil import Writer
from Util import ClientTestCase, TestSuite

traceProps = {
    "DataStorm.Trace.Topic" : 1,
    "DataStorm.Trace.Session" : 3,
    "DataStorm.Trace.Data" : 2,
}

TestSuite(
    __file__,
    [ ClientTestCase(client = Writer(), traceProps=traceProps) ],
    runOnMainThread=True)