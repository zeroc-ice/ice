# Copyright (c) ZeroC, Inc.

#
# A key, a key filter or a sample filter criteria is decoded by the application's Decoder, which can throw. These
# decodings run while a session attaches its elements, so a Decoder that rejects one value used to abandon the whole
# request and leave the elements around it unattached: the peer never acknowledged them, so readers that had nothing
# to do with the rejected value silently received nothing.
#

from DataStormUtil import Reader, Writer
from Util import ClientServerTestCase, TestSuite

traceProps = {
    "DataStorm.Trace.Topic": 1,
    "DataStorm.Trace.Session": 3,
    "DataStorm.Trace.Data": 2,
}

TestSuite(
    __file__,
    [
        ClientServerTestCase(name="Writer/Reader", client=Writer(), server=Reader(), traceProps=traceProps),
    ],
)
