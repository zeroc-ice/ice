# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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
    ref = "test:default -p 12010"
    base = communicator.stringToProxy(ref)
    test(base)

    proxy = Test::TestIntfPrx::checkedCast(base)
    test(proxy)

    print "testing enum values... "
    STDOUT.flush

    test(Test::ByteEnum::Benum1.to_i == 0);
    test(Test::ByteEnum::Benum2.to_i == 1);
    test(Test::ByteEnum::Benum3.to_i == Test::ByteConst1);
    test(Test::ByteEnum::Benum4.to_i == Test::ByteConst1 + 1);
    test(Test::ByteEnum::Benum5.to_i == Test::ShortConst1);
    test(Test::ByteEnum::Benum6.to_i == Test::ShortConst1 + 1);
    test(Test::ByteEnum::Benum7.to_i == Test::IntConst1);
    test(Test::ByteEnum::Benum8.to_i == Test::IntConst1 + 1);
    test(Test::ByteEnum::Benum9.to_i == Test::LongConst1);
    test(Test::ByteEnum::Benum10.to_i == Test::LongConst1 + 1);
    test(Test::ByteEnum::Benum11.to_i == Test::ByteConst2);

    test(Test::ByteEnum::from_int(0) == Test::ByteEnum::Benum1);
    test(Test::ByteEnum::from_int(1) == Test::ByteEnum::Benum2);
    test(Test::ByteEnum::from_int(Test::ByteConst1) == Test::ByteEnum::Benum3);
    test(Test::ByteEnum::from_int(Test::ByteConst1 + 1) == Test::ByteEnum::Benum4);
    test(Test::ByteEnum::from_int(Test::ShortConst1) == Test::ByteEnum::Benum5);
    test(Test::ByteEnum::from_int(Test::ShortConst1 + 1) == Test::ByteEnum::Benum6);
    test(Test::ByteEnum::from_int(Test::IntConst1) == Test::ByteEnum::Benum7);
    test(Test::ByteEnum::from_int(Test::IntConst1 + 1) == Test::ByteEnum::Benum8);
    test(Test::ByteEnum::from_int(Test::LongConst1) == Test::ByteEnum::Benum9);
    test(Test::ByteEnum::from_int(Test::LongConst1 + 1) == Test::ByteEnum::Benum10);
    test(Test::ByteEnum::from_int(Test::ByteConst2) == Test::ByteEnum::Benum11);

    test(Test::ShortEnum::Senum1.to_i == 3);
    test(Test::ShortEnum::Senum2.to_i == 4);
    test(Test::ShortEnum::Senum3.to_i == Test::ByteConst1);
    test(Test::ShortEnum::Senum4.to_i == Test::ByteConst1 + 1);
    test(Test::ShortEnum::Senum5.to_i == Test::ShortConst1);
    test(Test::ShortEnum::Senum6.to_i == Test::ShortConst1 + 1);
    test(Test::ShortEnum::Senum7.to_i == Test::IntConst1);
    test(Test::ShortEnum::Senum8.to_i == Test::IntConst1 + 1);
    test(Test::ShortEnum::Senum9.to_i == Test::LongConst1);
    test(Test::ShortEnum::Senum10.to_i == Test::LongConst1 + 1);
    test(Test::ShortEnum::Senum11.to_i == Test::ShortConst2);

    test(Test::ShortEnum::from_int(3) == Test::ShortEnum::Senum1);
    test(Test::ShortEnum::from_int(4) == Test::ShortEnum::Senum2);
    test(Test::ShortEnum::from_int(Test::ByteConst1) == Test::ShortEnum::Senum3);
    test(Test::ShortEnum::from_int(Test::ByteConst1 + 1) == Test::ShortEnum::Senum4);
    test(Test::ShortEnum::from_int(Test::ShortConst1) == Test::ShortEnum::Senum5);
    test(Test::ShortEnum::from_int(Test::ShortConst1 + 1) == Test::ShortEnum::Senum6);
    test(Test::ShortEnum::from_int(Test::IntConst1) == Test::ShortEnum::Senum7);
    test(Test::ShortEnum::from_int(Test::IntConst1 + 1) == Test::ShortEnum::Senum8);
    test(Test::ShortEnum::from_int(Test::LongConst1) == Test::ShortEnum::Senum9);
    test(Test::ShortEnum::from_int(Test::LongConst1 + 1) == Test::ShortEnum::Senum10);
    test(Test::ShortEnum::from_int(Test::ShortConst2) == Test::ShortEnum::Senum11);

    test(Test::IntEnum::Ienum1.to_i == 0);
    test(Test::IntEnum::Ienum2.to_i == 1);
    test(Test::IntEnum::Ienum3.to_i == Test::ByteConst1);
    test(Test::IntEnum::Ienum4.to_i == Test::ByteConst1 + 1);
    test(Test::IntEnum::Ienum5.to_i == Test::ShortConst1);
    test(Test::IntEnum::Ienum6.to_i == Test::ShortConst1 + 1);
    test(Test::IntEnum::Ienum7.to_i == Test::IntConst1);
    test(Test::IntEnum::Ienum8.to_i == Test::IntConst1 + 1);
    test(Test::IntEnum::Ienum9.to_i == Test::LongConst1);
    test(Test::IntEnum::Ienum10.to_i == Test::LongConst1 + 1);
    test(Test::IntEnum::Ienum11.to_i == Test::IntConst2);
    test(Test::IntEnum::Ienum12.to_i == Test::LongConst2);

    test(Test::IntEnum::from_int(0) == Test::IntEnum::Ienum1);
    test(Test::IntEnum::from_int(1) == Test::IntEnum::Ienum2);
    test(Test::IntEnum::from_int(Test::ByteConst1) == Test::IntEnum::Ienum3);
    test(Test::IntEnum::from_int(Test::ByteConst1 + 1) == Test::IntEnum::Ienum4);
    test(Test::IntEnum::from_int(Test::ShortConst1) == Test::IntEnum::Ienum5);
    test(Test::IntEnum::from_int(Test::ShortConst1 + 1) == Test::IntEnum::Ienum6);
    test(Test::IntEnum::from_int(Test::IntConst1) == Test::IntEnum::Ienum7);
    test(Test::IntEnum::from_int(Test::IntConst1 + 1) == Test::IntEnum::Ienum8);
    test(Test::IntEnum::from_int(Test::LongConst1) == Test::IntEnum::Ienum9);
    test(Test::IntEnum::from_int(Test::LongConst1 + 1) == Test::IntEnum::Ienum10);
    test(Test::IntEnum::from_int(Test::IntConst2) == Test::IntEnum::Ienum11);
    test(Test::IntEnum::from_int(Test::LongConst2) == Test::IntEnum::Ienum12);

    test(Test::SimpleEnum::Red.to_i == 0);
    test(Test::SimpleEnum::Green.to_i == 1);
    test(Test::SimpleEnum::Blue.to_i == 2);

    test(Test::SimpleEnum::from_int(0) == Test::SimpleEnum::Red);
    test(Test::SimpleEnum::from_int(1) == Test::SimpleEnum::Green);
    test(Test::SimpleEnum::from_int(2) == Test::SimpleEnum::Blue);

    puts "ok"

    print "testing enum operations... "
    STDOUT.flush

    test(proxy.opByte(Test::ByteEnum::Benum1) == [Test::ByteEnum::Benum1, Test::ByteEnum::Benum1]);
    test(proxy.opByte(Test::ByteEnum::Benum11) == [Test::ByteEnum::Benum11, Test::ByteEnum::Benum11]);

    test(proxy.opShort(Test::ShortEnum::Senum1) == [Test::ShortEnum::Senum1, Test::ShortEnum::Senum1]);
    test(proxy.opShort(Test::ShortEnum::Senum11) == [Test::ShortEnum::Senum11, Test::ShortEnum::Senum11]);

    test(proxy.opInt(Test::IntEnum::Ienum1) == [Test::IntEnum::Ienum1, Test::IntEnum::Ienum1]);
    test(proxy.opInt(Test::IntEnum::Ienum11) == [Test::IntEnum::Ienum11, Test::IntEnum::Ienum11]);
    test(proxy.opInt(Test::IntEnum::Ienum12) == [Test::IntEnum::Ienum12, Test::IntEnum::Ienum12]);

    test(proxy.opSimple(Test::SimpleEnum::Green) == [Test::SimpleEnum::Green, Test::SimpleEnum::Green]);

    puts "ok"

    return proxy
end
