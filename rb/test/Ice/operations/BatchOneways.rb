# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

def batchOneways(p)
    bs1 = "\0" * (10 * 1024);
    bs2 = "\0" * (99 * 1024);

    begin
        p.opByteSOneway(bs1)
    rescue Ice::MemoryLimitException
        test(false)
    end

    begin
        p.opByteSOneway(bs2)
    rescue Ice::MemoryLimitException
        test(false)
    end

    batch = Test::MyClassPrx::uncheckedCast(p.ice_batchOneway())

    p.opByteSOnewayCallCount() # Reset the call count

    for i in (0...30)
      batch.opByteSOneway(bs1)
    end

    count = 0
    while (count < 27) # 3 * 9 requests auto-flushed.
        count += p.opByteSOnewayCallCount()
        sleep(0.01)
    end

    batch.ice_getConnection().flushBatchRequests()

    batch2 = Test::MyClassPrx::uncheckedCast(p.ice_batchOneway())

    batch.ice_ping()
    batch2.ice_ping()
    batch.ice_flushBatchRequests()
    batch.ice_getConnection().close(false)
    batch.ice_ping()
    batch2.ice_ping()

    batch.ice_getConnection()
    batch2.ice_getConnection()

    batch.ice_ping()
    batch.ice_getConnection().close(false)
    begin
        batch.ice_ping()
        test(false)
    rescue Ice::CloseConnectionException
    end

    begin
        batch2.ice_ping()
        test(false)
    rescue Ice::CloseConnectionException
    end

    batch.ice_ping()
    batch2.ice_ping()

    identity = Ice::Identity.new()
    identity.name = "invalid";
    batch3 = batch.ice_identity(identity)
    batch3.ice_ping()
    batch3.ice_flushBatchRequests()

    # Make sure that a bogus batch request doesn't cause troubles to other ones.
    batch3.ice_ping()
    batch.ice_ping()
    batch.ice_flushBatchRequests()
    batch.ice_ping()
end
