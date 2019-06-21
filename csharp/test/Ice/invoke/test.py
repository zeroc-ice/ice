#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

TestSuite(__file__,
          [ClientServerTestCase(),
           ClientServerTestCase(name = "client/server array", servers=[Server(args = ["--async"])])])
