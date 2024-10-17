#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

from DataStormUtil import Reader, Writer
from Util import ClientServerTestCase, TestSuite

traceProps = {
    "DataStorm.Trace.Topic" : 1,
    "DataStorm.Trace.Session" : 3,
    "DataStorm.Trace.Data" : 2
}

TestSuite(
    __file__,
    [ ClientServerTestCase(client = Writer(), server = Reader(), traceProps=traceProps) ])
