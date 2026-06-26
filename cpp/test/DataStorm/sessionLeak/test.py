# Copyright (c) ZeroC, Inc.

#
# Regression test for a DataStorm session leak.
#
# A node that loses its session to an endpoint-less peer schedules a "wait then remove" retry timer. The timer task
# captures a strong reference to the session, and neither the timer firing (remove -> destroyImpl) nor node
# destruction cleared it, leaving a self-reference cycle that leaked the session -- and, through it, the node and
# instance. This single-process test drives that path and asserts the node's communicator is reclaimed.
#

from DataStormUtil import Writer
from Util import ClientTestCase, TestSuite

TestSuite(__file__, [ClientTestCase(client=Writer())])
