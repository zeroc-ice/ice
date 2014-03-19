#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, traceback, Ice, IceGrid

Ice.loadSlice('Hello.ice')
import Demo


def menu():
    print("""
usage:
t: send greeting as twoway
s: shutdown server
x: exit
?: help
""")

class Client(Ice.Application):
    def run(self, args):
        if len(args) > 1:
            print(self.appName() + ": too many arguments")
            return 1

        hello = None
        try:
            hello = Demo.HelloPrx.checkedCast(self.communicator().stringToProxy("hello"))
        except Ice.NotRegisteredException:
            query = IceGrid.QueryPrx.checkedCast(self.communicator().stringToProxy("DemoIceGrid/Query"))
            hello = Demo.HelloPrx.checkedCast(query.findObjectByType("::Demo::Hello"))

        if not hello:
            print(self.appName() + ": couldn't find a `::Demo::Hello' object.")
            return 1

        menu()

        c = None
        while c != 'x':
            try:
                sys.stdout.write("==> ")
                sys.stdout.flush()
                c = sys.stdin.readline().strip()
                if c == 't':
                    hello.sayHello()
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

        return 0

app = Client()
sys.exit(app.main(sys.argv, "config.client"))
