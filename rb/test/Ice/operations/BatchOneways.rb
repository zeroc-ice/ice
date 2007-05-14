# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

def batchOneways(p)
    bs1 = "\0" * (10 * 1024);
    bs2 = "\0" * (99 * 1024);
    bs3 = "\0" * (100 * 1024);

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

    begin
        p.opByteSOneway(bs3)
        test(false)
    rescue Ice::MemoryLimitException
    end

    batch = Test::MyClassPrx::uncheckedCast(p.ice_batchOneway())

    for i in (0...30)
        begin
            batch.opByteSOneway(bs1)
        rescue Ice::MemoryLimitException
            test(false)
        end
    end

    batch.ice_getConnection().flushBatchRequests()
end
