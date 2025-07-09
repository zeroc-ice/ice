# Copyright (c) ZeroC, Inc.

from Glacier2Util import Glacier2TestSuite

Glacier2TestSuite(__name__, routerProps={"Glacier2.Client.Connection.IdleTimeout": 30})
