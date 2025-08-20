# Copyright (c) ZeroC, Inc.

from Util import Client, ClientTestCase, TestSuite

TestSuite(
    __name__,
    [
        ClientTestCase(
            "PHP registered communicator",
            client=Client(),
        ),
    ],
)
