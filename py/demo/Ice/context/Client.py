#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, traceback, Ice

Ice.loadSlice('Context.ice')
import Demo

def menu():
    print("""
usage:
1: call with no request context
2: call with explicit request context
3: call with per-proxy request context
4: call with implicit request context
s: shutdown server
x: exit
?: help
""")

class Client(Ice.Application):
    def run(self, args):
        if len(args) > 1:
            print(self.appName() + ": too many arguments")
            return 1

        proxy = Demo.ContextPrx.checkedCast(self.communicator().propertyToProxy('Context.Proxy'))
        if not proxy:
            print(args[0] + ": invalid proxy")
            return 1

        menu()

        c = None
        while c != 'x':
            try:
                sys.stdout.write("==> ")
                sys.stdout.flush()
                c = sys.stdin.readline().strip()
                if c == '1':
                    proxy.call()
                elif c == '2':
                    ctx = {'type': 'Explicit'}
                    proxy.call(ctx)
                elif c == '3':
                    ctx = {'type': 'Per-Proxy'}
                    proxy2 = Demo.ContextPrx.uncheckedCast(proxy.ice_context(ctx))
                    proxy2.call()
                elif c == '4':
                    ic = self.communicator().getImplicitContext()
                    ctx = {'type': 'Implicit'}
                    ic.setContext(ctx)
                    proxy.call()
                    ic.setContext({})
                elif c == 's':
                    proxy.shutdown()
                elif c == 'x':
                    pass # Nothing to do
                elif c == '?':
                    menu()
                else:
                    print("unknown command `" + c + "'")
                    menu()
            except KeyboardInterrupt:
                break
            except EOFError:
                break
            except Ice.Exception as ex:
                print(ex)

        return 0

app = Client()
sys.exit(app.main(sys.argv, "config.client"))
