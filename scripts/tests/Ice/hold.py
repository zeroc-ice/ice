# -*- coding: utf-8 -*-
# Copyright (c) ZeroC, Inc.

from Util import ClientServerTestCase, Server, TestSuite


TestSuite(__name__, [ClientServerTestCase(server=Server(readyCount=2))])
