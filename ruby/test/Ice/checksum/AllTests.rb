# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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

def allTests(helper, communicator)
    ref = "test:#{helper.getTestEndpoint()}"
    base = communicator.stringToProxy(ref)
    test(base)

    checksum = Test::ChecksumPrx::checkedCast(base)
    test(checksum)

    #
    # Verify that no checksums are present for local types.
    #
    print "testing checksums... "
    STDOUT.flush
    test(Ice::SliceChecksums.length > 0)
    for i in Ice::SliceChecksums.keys
        test(!i.include?("Local"))
    end

    #
    # Get server's Slice checksums.
    #
    d = checksum.getSliceChecksums()

    #
    # Compare the checksums. For a type FooN whose name ends in an integer N,
    # we assume that the server's type does not change for N = 1, and does
    # change for N > 1.
    #
    patt = Regexp.new("\\d+")
    for i in d.keys
        n = 0
        m = patt.match(i)
        if m
            n = i[m.begin(0)...i.length].to_i
        end

        test(Ice::SliceChecksums.has_key?(i))

        if n <= 1
            test(Ice::SliceChecksums[i] == d[i])
        else
            test(Ice::SliceChecksums[i] != d[i])
        end
    end

    puts "ok"

    return checksum
end
