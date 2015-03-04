#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, traceback, time, Ice

Ice.loadSlice('Greet.ice')
import Demo

def decodeString(str):
    ret = ""
    for i in range(0, len(str)):
        n = ord(str[i])
        if n < 32 or n > 126:
            ret += "\\" + oct(n)[1:] # Skip leading '0'
        else:
            ret += str[i]
    return ret

class GreatI(Demo.Greet):
    def exchangeGreeting(self, msg, current=None):
       
        print("Received (UTF-8): \"" + decodeString(msg) + "\"")
        return "Bonne journ\303\251e"

    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()

class Server(Ice.Application):
    def run(self, args):
        if len(args) > 1:
            print(self.appName() + ": too many arguments")
            return 1

        adapter = self.communicator().createObjectAdapter("Greet")
        adapter.add(GreatI(), self.communicator().stringToIdentity("greet"))
        adapter.activate()
        self.communicator().waitForShutdown()
        return 0

sys.stdout.flush()
app = Server()
sys.exit(app.main(sys.argv, "config.server"))
