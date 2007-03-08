#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, os, traceback, threading, Ice

Ice.loadSlice('Hello.ice')
import Demo

class AMI_Hello_sayHelloI:
    def ice_response(self):
        pass

    def ice_exception(self, ex):
        if isinstance(ex, Demo.RequestCanceledException):
            print "Request canceled"
        else:
            print "sayHello AMI call failed:"
            print ex

def menu():
    print """
usage:
i: send immediate greeting
d: send delayed greeting
s: shutdown server
x: exit
?: help
"""

class Client(Ice.Application):
    def run(self, args):
        hello = Demo.HelloPrx.checkedCast(self.communicator().propertyToProxy('Hello.Proxy'))
        if not hello:
            print args[0] + ": invalid proxy"
            return 1

        menu()

        c = None
        while c != 'x':
            try:
                c = raw_input("==> ")
                if c == 'i':
                    hello.sayHello(0)
                elif c == 'd':
                    hello.sayHello_async(AMI_Hello_sayHelloI(), 5000)
                elif c == 's':
                    hello.shutdown()
                elif c == 'x':
                    pass # Nothing to do
                elif c == '?':
                    menu()
                else:
                    print "unknown command `" + c + "'"
                    menu()
            except EOFError:
                break
            except KeyboardInterrupt:
                break
            except Ice.Exception, ex:
                print ex

        return 0

app = Client()
sys.exit(app.main(sys.argv, "config.client"))
