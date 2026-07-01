# Copyright (c) ZeroC, Inc.

from Util import ClientTestCase, TestSuite


# The iAP transport is iOS-only; these tests exercise it without accessory hardware and only run on the
# iOS simulator / device.
class IAPTestCase(ClientTestCase):
    def canRun(self, current):
        return "iphone" in current.config.buildPlatform


TestSuite(__name__, [IAPTestCase()], multihost=False)
