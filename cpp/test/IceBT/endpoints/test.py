# Copyright (c) ZeroC, Inc.

import subprocess

from Util import ClientTestCase, Linux, TestSuite, platform


# The IceBT plug-in -- and therefore this test -- is only built on Linux when the Bluetooth system
# libraries (BlueZ/DBus) are available. Guard the test the same way the build does (see
# cpp/test/IceBT/endpoints/Makefile.mk and config/Make.rules.Linux) so the harness doesn't try to
# run a client that was never built.
def hasBluetoothLibraries():
    try:
        return (
            subprocess.call(
                ["pkg-config", "--exists", "bluez", "dbus-1"],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
            )
            == 0
        )
    except OSError:
        return False


# This test only parses bt/bts endpoint strings; it does not open any Bluetooth connection, so it
# runs under the default (tcp) configuration and needs no Bluetooth hardware.
if isinstance(platform, Linux) and hasBluetoothLibraries():
    TestSuite(__file__, [ClientTestCase()])
