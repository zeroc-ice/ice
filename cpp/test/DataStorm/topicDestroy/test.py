# Copyright (c) ZeroC, Inc.

from DataStormUtil import Reader, Writer
from Util import ClientServerTestCase, TestSuite

traceProps = {
    "DataStorm.Trace.Topic": 1,
    "DataStorm.Trace.Session": 3,
    "DataStorm.Trace.Data": 2,
}

TestSuite(
    __file__,
    [ClientServerTestCase(name="Writer/Reader", client=Writer(), server=Reader(), traceProps=traceProps)],
)
