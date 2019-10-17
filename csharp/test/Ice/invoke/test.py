#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

TestSuite(__file__,
          [ClientServerTestCase(),
           ClientServerTestCase(name = "client/server async", servers=[Server(args = ["--async"])])])
