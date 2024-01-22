# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

from Util import ClientServerTestCase, Server, TestSuite


TestSuite(__name__, [ClientServerTestCase(server=Server(readyCount=2))])
