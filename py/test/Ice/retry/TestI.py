# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test

class RetryI(Test.Retry):

    def op(self, kill, current=None):
        if kill:
            current.con.close(True)

    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()
