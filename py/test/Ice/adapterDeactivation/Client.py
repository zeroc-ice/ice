# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, Ice, Test, _Top

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

class TestClient(Ice.Application):
    def run(self, args):
        print "testing stringToProxy... ",
        sys.stdout.flush()
        base = self.communicator().stringToProxy("test:default -p 12345 -t 10000")
        test(base)
        print "ok"

        print "testing checked cast... ",
        sys.stdout.flush()
        obj = _Top.TestPrx.checkedCast(base)
        test(obj)
        test(obj == base)
        print "ok"

        print "creating/activating/deactivating object adapter in one operation... ",
        sys.stdout.flush()
        obj.transient()
        print "ok"

        print "deactivating object adapter in the server... ",
        sys.stdout.flush()
        obj.deactivate()
        print "ok"

        print "testing whether server is gone... ",
        sys.stdout.flush()
        try:
            obj.ice_ping()
            test(False)
        except Ice.LocalException:
            print "ok"

        return 0

app = TestClient()
sys.exit(app.main(sys.argv))
