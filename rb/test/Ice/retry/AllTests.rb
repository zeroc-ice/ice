# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

def test(b)
    if !b
        raise RuntimeError, 'test assertion failed'
    end
end

def allTests(communicator)
    print "testing stringToProxy... "
    STDOUT.flush
    ref = "retry:default -p 12010 -t 10000"
    base1 = communicator.stringToProxy(ref)
    test(base1)
    base2 = communicator.stringToProxy(ref)
    test(base2)
    puts "ok"

    print "testing checked cast... "
    STDOUT.flush
    retry1 = Test::RetryPrx::checkedCast(base1)
    test(retry1)
    test(retry1 == base1)
    retry2 = Test::RetryPrx::checkedCast(base2)
    test(retry2 != nil)
    test(retry2 == base2)
    puts "ok"

    print "calling regular operation with first proxy... "
    STDOUT.flush
    retry1.op(false)
    puts "ok"

    print "calling operation to kill connection with second proxy... "
    STDOUT.flush
    begin
        retry2.op(true)
        test(false)
    rescue Ice::ConnectionLostException
        puts "ok"
    end

    print "calling regular operation with first proxy again... "
    STDOUT.flush
    retry1.op(false)
    puts "ok"

    return retry1
end
