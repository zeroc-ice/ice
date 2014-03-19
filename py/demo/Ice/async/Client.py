#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, os, traceback, threading, Ice

Ice.loadSlice('Hello.ice')
import Demo

class Callback:
    def response(self):
        pass

    def exception(self, ex):
        if isinstance(ex, Demo.RequestCanceledException):
            print("Demo.RequestCanceledException")
        else:
            print("sayHello AMI call failed:")
            print(ex)

def menu():
    print("""
usage:
i: send immediate greeting
d: send delayed greeting
s: shutdown server
x: exit
?: help
""")

class Client(Ice.Application):
    def run(self, args):
        if len(args) > 1:
            print(self.appName() + ": too many arguments")
            return 1

        hello = Demo.HelloPrx.checkedCast(self.communicator().propertyToProxy('Hello.Proxy'))
        if not hello:
            print(args[0] + ": invalid proxy")
            return 1

        menu()

        c = None
        while c != 'x':
            try:
                sys.stdout.write("==> ")
                sys.stdout.flush()
                c = sys.stdin.readline().strip()
                if c == 'i':
                    hello.sayHello(0)
                elif c == 'd':
                    cb = Callback()
                    hello.begin_sayHello(5000, cb.response, cb.exception)
                elif c == 's':
                    hello.shutdown()
                elif c == 'x':
                    pass # Nothing to do
                elif c == '?':
                    menu()
                else:
                    print("unknown command `" + c + "'")
                    menu()
            except EOFError:
                break
            except KeyboardInterrupt:
                break
            except Ice.Exception as ex:
                print(ex)

        return 0

app = Client()
sys.exit(app.main(sys.argv, "config.client"))
