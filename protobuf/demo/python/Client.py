#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice Protobuf is licensed to you under the terms
# described in the ICE_PROTOBUF_LICENSE file included in this
# distribution.
#
# **********************************************************************

import sys, traceback, Ice

Ice.loadSlice('Hello.ice')
import Demo
from Person_pb2 import Person

def menu():
    print """
usage:
t: send greeting
s: shutdown server
x: exit
?: help
"""

class Client(Ice.Application):
    def run(self, args):
        if len(args) > 1:
            print self.appName() + ": too many arguments"
            return 1

        hello = Demo.HelloPrx.checkedCast(self.communicator().propertyToProxy('Hello.Proxy'))
        if not hello:
            print args[0] + ": invalid proxy"
            return 1

        menu()

        p = Person()
        p.id = 1
        p.name = "Fred Jones"
        p.email = "fred@jones.com"

        c = None
        while c != 'x':
            try:
                c = raw_input("==> ")
                if c == 't':
                    hello.sayHello(p)
                elif c == 's':
                    hello.shutdown()
                elif c == 'x':
                    pass # Nothing to do
                elif c == '?':
                    menu()
                else:
                    print "unknown command `" + c + "'"
                    menu()
            except KeyboardInterrupt:
                break
            except EOFError:
                break
            except Ice.Exception, ex:
                print ex

        return 0

app = Client()
sys.exit(app.main(sys.argv, "config.client"))
