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

Ice.loadSlice('Hello.ice')
import Demo

def menu():
    print("""
usage:
t: send greeting as twoway
o: send greeting as oneway
O: send greeting as batch oneway
d: send greeting as datagram
D: send greeting as batch datagram
f: flush all batch requests
T: set a timeout
P: set a server delay
S: switch secure mode on/off
s: shutdown server
x: exit
?: help
""")

class Client(Ice.Application):
    def run(self, args):
        if len(args) > 1:
            print(self.appName() + ": too many arguments")
            return 1

        twoway = Demo.HelloPrx.checkedCast(\
            self.communicator().propertyToProxy('Hello.Proxy').ice_twoway().ice_timeout(-1).ice_secure(False))
        if not twoway:
            print(args[0] + ": invalid proxy")
            return 1

        oneway = Demo.HelloPrx.uncheckedCast(twoway.ice_oneway())
        batchOneway = Demo.HelloPrx.uncheckedCast(twoway.ice_batchOneway())
        datagram = Demo.HelloPrx.uncheckedCast(twoway.ice_datagram())
        batchDatagram = Demo.HelloPrx.uncheckedCast(twoway.ice_batchDatagram())

        secure = False
        timeout = -1
        delay = 0

        menu()

        c = None
        while c != 'x':
            try:
                sys.stdout.write("==> ")
                sys.stdout.flush()
                c = sys.stdin.readline().strip()
                if c == 't':
                    twoway.sayHello(delay)
                elif c == 'o':
                    oneway.sayHello(delay)
                elif c == 'O':
                    batchOneway.sayHello(delay)
                elif c == 'd':
                    if secure:
                        print("secure datagrams are not supported")
                    else:
                        datagram.sayHello(delay)
                elif c == 'D':
                    if secure:
                        print("secure datagrams are not supported")
                    else:
                        batchDatagram.sayHello(delay)
                elif c == 'f':
                    self.communicator().flushBatchRequests()
                elif c == 'T':
                    if timeout == -1:
                        timeout = 2000
                    else:
                        timeout = -1

                    twoway = Demo.HelloPrx.uncheckedCast(twoway.ice_timeout(timeout))
                    oneway = Demo.HelloPrx.uncheckedCast(oneway.ice_timeout(timeout))
                    batchOneway = Demo.HelloPrx.uncheckedCast(batchOneway.ice_timeout(timeout))

                    if timeout == -1:
                        print("timeout is now switched off")
                    else:
                        print("timeout is now set to 2000ms")
                elif c == 'P':
                    if delay == 0:
                        delay = 2500
                    else:
                        delay = 0

                    if delay == 0:
                        print("server delay is now deactivated")
                    else:
                        print("server delay is now set to 2500ms")
                elif c == 'S':
                    secure = not secure

                    twoway = Demo.HelloPrx.uncheckedCast(twoway.ice_secure(secure))
                    oneway = Demo.HelloPrx.uncheckedCast(oneway.ice_secure(secure))
                    batchOneway = Demo.HelloPrx.uncheckedCast(batchOneway.ice_secure(secure))
                    datagram = Demo.HelloPrx.uncheckedCast(datagram.ice_secure(secure))
                    batchDatagram = Demo.HelloPrx.uncheckedCast(batchDatagram.ice_secure(secure))

                    if secure:
                        print("secure mode is now on")
                    else:
                        print("secure mode is now off")
                elif c == 's':
                    twoway.shutdown()
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
