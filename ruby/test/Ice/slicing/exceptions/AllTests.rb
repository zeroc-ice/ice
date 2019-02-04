#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

def test(b)
    if !b
        raise RuntimeError, 'test assertion failed'
    end
end

def allTests(helper, communicator)
    obj = communicator.stringToProxy("Test:#{helper.getTestEndpoint()}")
    t = Test::TestIntfPrx::checkedCast(obj)

    print "base... "
    STDOUT.flush
    begin
        t.baseAsBase()
        test(false)
    rescue Test::Base => b
        test(b.b == "Base.b")
        test(b.ice_id() == "::Test::Base")
    rescue
        test(false)
    end
    puts "ok"

    print "slicing of unknown derived... "
    STDOUT.flush
    begin
        t.unknownDerivedAsBase()
        test(false)
    rescue Test::Base => b
        test(b.b == "UnknownDerived.b")
        test(b.ice_id() == "::Test::Base")
    rescue
        test(false)
    end
    puts "ok"

    print "non-slicing of known derived as base... "
    STDOUT.flush
    begin
        t.knownDerivedAsBase()
        test(false)
    rescue Test::KnownDerived => k
        test(k.b == "KnownDerived.b")
        test(k.kd == "KnownDerived.kd")
        test(k.ice_id() == "::Test::KnownDerived")
    rescue
        test(false)
    end
    puts "ok"

    print "non-slicing of known derived as derived... "
    STDOUT.flush
    begin
        t.knownDerivedAsKnownDerived()
        test(false)
    rescue Test::KnownDerived => k
        test(k.b == "KnownDerived.b")
        test(k.kd == "KnownDerived.kd")
        test(k.ice_id() == "::Test::KnownDerived")
    rescue
        test(false)
    end
    puts "ok"

    print "slicing of unknown intermediate as base... "
    STDOUT.flush
    begin
        t.unknownIntermediateAsBase()
        test(false)
    rescue Test::Base => b
        test(b.b == "UnknownIntermediate.b")
        test(b.ice_id() == "::Test::Base")
    rescue
        test(false)
    end
    puts "ok"

    print "slicing of known intermediate as base... "
    STDOUT.flush
    begin
        t.knownIntermediateAsBase()
        test(false)
    rescue Test::KnownIntermediate => ki
        test(ki.b == "KnownIntermediate.b")
        test(ki.ki == "KnownIntermediate.ki")
        test(ki.ice_id() == "::Test::KnownIntermediate")
    rescue
        test(false)
    end
    puts "ok"

    print "slicing of known most derived as base... "
    STDOUT.flush
    begin
        t.knownMostDerivedAsBase()
        test(false)
    rescue Test::KnownMostDerived => kmd
        test(kmd.b == "KnownMostDerived.b")
        test(kmd.ki == "KnownMostDerived.ki")
        test(kmd.kmd == "KnownMostDerived.kmd")
        test(kmd.ice_id() == "::Test::KnownMostDerived")
    rescue
        test(false)
    end
    puts "ok"

    print "non-slicing of known intermediate as intermediate... "
    STDOUT.flush
    begin
        t.knownIntermediateAsKnownIntermediate()
        test(false)
    rescue Test::KnownIntermediate => ki
        test(ki.b == "KnownIntermediate.b")
        test(ki.ki == "KnownIntermediate.ki")
        test(ki.ice_id() == "::Test::KnownIntermediate")
    rescue
        test(false)
    end
    puts "ok"

    print "non-slicing of known most derived exception as intermediate... "
    STDOUT.flush
    begin
        t.knownMostDerivedAsKnownIntermediate()
        test(false)
    rescue Test::KnownMostDerived => kmd
        test(kmd.b == "KnownMostDerived.b")
        test(kmd.ki == "KnownMostDerived.ki")
        test(kmd.kmd == "KnownMostDerived.kmd")
        test(kmd.ice_id() == "::Test::KnownMostDerived")
    rescue
        test(false)
    end
    puts "ok"

    print "non-slicing of known most derived as most derived... "
    STDOUT.flush
    begin
        t.knownMostDerivedAsKnownMostDerived()
        test(false)
    rescue Test::KnownMostDerived => kmd
        test(kmd.b == "KnownMostDerived.b")
        test(kmd.ki == "KnownMostDerived.ki")
        test(kmd.kmd == "KnownMostDerived.kmd")
        test(kmd.ice_id() == "::Test::KnownMostDerived")
    rescue
        test(false)
    end
    puts "ok"

    print "slicing of unknown most derived, known intermediate as base... "
    STDOUT.flush
    begin
        t.unknownMostDerived1AsBase()
        test(false)
    rescue Test::KnownIntermediate => ki
        test(ki.b == "UnknownMostDerived1.b")
        test(ki.ki == "UnknownMostDerived1.ki")
        test(ki.ice_id() == "::Test::KnownIntermediate")
    rescue
        test(false)
    end
    puts "ok"

    print "slicing of unknown most derived, known intermediate as intermediate... "
    STDOUT.flush
    begin
        t.unknownMostDerived1AsKnownIntermediate()
        test(false)
    rescue Test::KnownIntermediate => ki
        test(ki.b == "UnknownMostDerived1.b")
        test(ki.ki == "UnknownMostDerived1.ki")
        test(ki.ice_id() == "::Test::KnownIntermediate")
    rescue
        test(false)
    end
    puts "ok"

    print "slicing of unknown most derived, unknown intermediate as base... "
    STDOUT.flush
    begin
        t.unknownMostDerived2AsBase()
        test(false)
    rescue Test::Base => b
        test(b.b == "UnknownMostDerived2.b")
        test(b.ice_id() == "::Test::Base")
    rescue
        test(false)
    end
    puts "ok"

    print "unknown most derived in compact format... "
    STDOUT.flush
    begin
        t.unknownMostDerived2AsBaseCompact()
        test(false)
    rescue Test::Base
        #
        # For the 1.0 encoding, the unknown exception is sliced to Base.
        #
        test(t.ice_getEncodingVersion() == Ice::Encoding_1_0)
    rescue Ice::UnknownUserException
        #
        # An UnkonwnUserException is raised for the compact format because the
        # most-derived type is unknown and the exception cannot be sliced.
        #
        test(t.ice_getEncodingVersion() != Ice::Encoding_1_0)
    rescue
        test(false)
    end
    puts "ok"

    print "preserved exceptions... "
    STDOUT.flush

    begin
        t.knownPreservedAsBase()
        test(false)
    rescue Test::KnownPreservedDerived => ex
        test(ex.b == "base")
        test(ex.kp == "preserved")
        test(ex.kpd == "derived")
    rescue
        test(false)
    end

    begin
        t.knownPreservedAsKnownPreserved()
        test(false)
    rescue Test::KnownPreservedDerived => ex
        test(ex.b == "base")
        test(ex.kp == "preserved")
        test(ex.kpd == "derived")
    rescue
        test(false)
    end

    begin
        t.unknownPreservedAsBase()
        test(false)
    rescue Test::KnownPreservedDerived => ex
        test(ex.b == "base")
        test(ex.kp == "preserved")
        test(ex.kpd == "derived")
    rescue
        test(false)
    end

    begin
        t.unknownPreservedAsKnownPreserved()
        test(false)
    rescue Test::KnownPreservedDerived => ex
        test(ex.b == "base")
        test(ex.kp == "preserved")
        test(ex.kpd == "derived")
    rescue
        test(false)
    end

    puts "ok"

    return t
end
