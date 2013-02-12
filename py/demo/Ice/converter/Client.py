#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, traceback, Ice

Ice.loadSlice('Greet.ice')
import Demo

def menu():
    print("""
usage:
t: send greeting with conversion
u: send greeting without conversion
s: shutdown server
x: exit
?: help
""")

def decodeString(str):
    ret = ""
    for i in range(0, len(str)):
        n = ord(str[i])
        if n < 32 or n > 126:
            ret += "\\" + oct(n)[1:] # Skip leading '0'
        else:
            ret += str[i]
    return ret

communicator1 = None
communicator2 = None

class Client:
    def run(self, args):
        if len(args) > 1:
            print(self.appName() + ": too many arguments")
            return 1

        greet1 = Demo.GreetPrx.checkedCast(communicator1.propertyToProxy('Greet.Proxy'))
        if not greet1:
            print(args[0] + ": invalid proxy")
            return 1
        
        greet2 = Demo.GreetPrx.checkedCast(communicator2.propertyToProxy('Greet.Proxy'))
        if not greet2:
            print(args[0] + ": invalid proxy")
            return 1
        
        menu()

        greeting = "Bonne journ\351e";

        c = None
        while c != 'x':
            try:
                sys.stdout.write("==> ")
                sys.stdout.flush()
                c = sys.stdin.readline().strip()
                if c == 't':
                    ret = greet1.exchangeGreeting(greeting)
                    print("Received: \"" + decodeString(ret) + "\"")
                elif c == 'u':
                    ret = greet2.exchangeGreeting(greeting)
                    print("Received: \"" + decodeString(ret) + "\"")
                elif c == 's':
                    greet1.shutdown()
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

status = 0

try:

    initData = Ice.InitializationData()
    initData.properties = Ice.createProperties(None, initData.properties)
    initData.properties.load("config.client")
    #
    # With converter
    #
    communicator1 = Ice.initialize(sys.argv, initData)

    #
    # Clear plug-in, so no converter
    #
    initData.properties.setProperty("Ice.Plugin.StringConverter", "")
    communicator2 = Ice.initialize(sys.argv, initData)

    app = Client()
    status = app.run(sys.argv)

except:
    traceback.print_exc()
    status = 1

if communicator1:
    try:
        communicator1.destroy()
    except:
        traceback.print_exc()
        status = 1

if communicator2:
    try:
        communicator2.destroy()
    except:
        traceback.print_exc()
        status = 1

sys.exit(status)
