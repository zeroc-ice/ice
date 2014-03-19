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

slice_dir = Ice.getSliceDir()
if not slice_dir:
    print(sys.argv[0] + ': Slice directory not found.')
    sys.exit(1)

Ice.loadSlice("'-I" + slice_dir + "' Props.ice")
import Demo

def menu():
    print("""
usage:
1: set properties (batch 1)
2: set properties (batch 2)
c: show current properties
s: shutdown server
x: exit
?: help
""")

def show(admin):
    props = admin.getPropertiesForPrefix("Demo")
    print "Server's current settings:"
    for k,v in props.items():
        print "  " + k + "=" + v

class Client(Ice.Application):
    def run(self, args):
        if len(args) > 1:
            print(self.appName() + ": too many arguments")
            return 1

        props = Demo.PropsPrx.checkedCast(self.communicator().propertyToProxy("Props.Proxy"))
        if props == None:
            print("invalid proxy")
            return 1

        admin = Ice.PropertiesAdminPrx.checkedCast(self.communicator().propertyToProxy("Admin.Proxy"))

        batch1 = {}
        batch1["Demo.Prop1"] = "1"
        batch1["Demo.Prop2"] = "2"
        batch1["Demo.Prop3"] = "3"

        batch2 = {}
        batch2["Demo.Prop1"] = "10"
        batch2["Demo.Prop2"] = "" # An empty value removes this property
        batch2["Demo.Prop3"] = "30"

        show(admin)
        menu()

        c = None
        while c != 'x':
            try:
                sys.stdout.write("==> ")
                sys.stdout.flush()
                c = sys.stdin.readline().strip()
                if c == "1" or c == "2":
                    propsDict = c == "1" and batch1 or batch2
                    print("Sending:")
                    for k, v in propsDict.items():
                        if k.startswith("Demo"):
                            print("  " + k + "=" + v)
                    print

                    admin.setProperties(propsDict)

                    print("Changes:")
                    changes = props.getChanges()
                    if len(changes) == 0:
                        print("  None.")
                    else:
                        for k, v in changes.items():
                            sys.stdout.write("  " + k)
                            if len(v) == 0:
                                print(" was removed")
                            else:
                                print(" is now " + v)
                elif c == "c":
                    show(admin)
                elif c == "s":
                    props.shutdown()
                elif c == "x":
                    # Nothing to do
                    pass
                elif c == "?":
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
