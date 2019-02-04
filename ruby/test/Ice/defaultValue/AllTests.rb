#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

def allTests()
    print "testing default values... "
    STDOUT.flush

    v = Test::Struct1.new
    test(!v.boolFalse)
    test(v.boolTrue)
    test(v.b == 254)
    test(v.s == 16000)
    test(v.i == 3)
    test(v.l == 4)
    test(v.f == 5.1)
    test(v.d == 6.2)
    test(v.str == "foo \\ \"bar\n \r\n\t\v\f\a\b? \007 \x07")
    test(v.c1 == Test::Color::Red)
    test(v.c2 == Test::Color::Green)
    test(v.c3 == Test::Color::Blue)
    test(v.nc1 == Test::Nested::Color::Red)
    test(v.nc2 == Test::Nested::Color::Green)
    test(v.nc3 == Test::Nested::Color::Blue)
    test(v.noDefault == '')
    test(v.zeroI == 0)
    test(v.zeroL == 0)
    test(v.zeroF == 0)
    test(v.zeroDotF == 0)
    test(v.zeroD == 0)
    test(v.zeroDotD == 0)

    v = Test::Struct2.new
    test(v.boolTrue == Test::ConstBool)
    test(v.b == Test::ConstByte)
    test(v.s == Test::ConstShort)
    test(v.i == Test::ConstInt)
    test(v.l == Test::ConstLong)
    test(v.f == Test::ConstFloat)
    test(v.d == Test::ConstDouble)
    test(v.str == Test::ConstString)
    test(v.c1 == Test::ConstColor1)
    test(v.c2 == Test::ConstColor2)
    test(v.c3 == Test::ConstColor3)
    test(v.nc1 == Test::ConstNestedColor1)
    test(v.nc2 == Test::ConstNestedColor2)
    test(v.nc3 == Test::ConstNestedColor3)
    test(v.zeroI == Test::ConstZeroI)
    test(v.zeroL == Test::ConstZeroL)
    test(v.zeroF == Test::ConstZeroF)
    test(v.zeroDotF == Test::ConstZeroDotF)
    test(v.zeroD == Test::ConstZeroD)
    test(v.zeroDotD == Test::ConstZeroDotD)

    v = Test::Base.new
    test(!v.boolFalse)
    test(v.boolTrue)
    test(v.b == 1)
    test(v.s == 2)
    test(v.i == 3)
    test(v.l == 4)
    test(v.f == 5.1)
    test(v.d == 6.2)
    test(v.str == "foo \\ \"bar\n \r\n\t\v\f\a\b? \007 \x07")
    test(v.noDefault == '')
    test(v.zeroI == 0)
    test(v.zeroL == 0)
    test(v.zeroF == 0)
    test(v.zeroDotF == 0)
    test(v.zeroD == 0)
    test(v.zeroDotD == 0)

    v = Test::Derived.new
    test(!v.boolFalse)
    test(v.boolTrue)
    test(v.b == 1)
    test(v.s == 2)
    test(v.i == 3)
    test(v.l == 4)
    test(v.f == 5.1)
    test(v.d == 6.2)
    test(v.str == "foo \\ \"bar\n \r\n\t\v\f\a\b? \007 \x07")
    test(v.c1 == Test::Color::Red)
    test(v.c2 == Test::Color::Green)
    test(v.c3 == Test::Color::Blue)
    test(v.nc1 == Test::Nested::Color::Red)
    test(v.nc2 == Test::Nested::Color::Green)
    test(v.nc3 == Test::Nested::Color::Blue)
    test(v.noDefault == '')
    test(v.zeroI == 0)
    test(v.zeroL == 0)
    test(v.zeroF == 0)
    test(v.zeroDotF == 0)
    test(v.zeroD == 0)
    test(v.zeroDotD == 0)

    v = Test::BaseEx.new
    test(!v.boolFalse)
    test(v.boolTrue)
    test(v.b == 1)
    test(v.s == 2)
    test(v.i == 3)
    test(v.l == 4)
    test(v.f == 5.1)
    test(v.d == 6.2)
    test(v.str == "foo \\ \"bar\n \r\n\t\v\f\a\b? \007 \x07")
    test(v.noDefault == '')
    test(v.zeroI == 0)
    test(v.zeroL == 0)
    test(v.zeroF == 0)
    test(v.zeroDotF == 0)
    test(v.zeroD == 0)
    test(v.zeroDotD == 0)

    v = Test::DerivedEx.new
    test(!v.boolFalse)
    test(v.boolTrue)
    test(v.b == 1)
    test(v.s == 2)
    test(v.i == 3)
    test(v.l == 4)
    test(v.f == 5.1)
    test(v.d == 6.2)
    test(v.str == "foo \\ \"bar\n \r\n\t\v\f\a\b? \007 \x07")
    test(v.noDefault == '')
    test(v.c1 == Test::Color::Red)
    test(v.c2 == Test::Color::Green)
    test(v.c3 == Test::Color::Blue)
    test(v.nc1 == Test::Nested::Color::Red)
    test(v.nc2 == Test::Nested::Color::Green)
    test(v.nc3 == Test::Nested::Color::Blue)
    test(v.noDefault == '')
    test(v.zeroI == 0)
    test(v.zeroL == 0)
    test(v.zeroF == 0)
    test(v.zeroDotF == 0)
    test(v.zeroD == 0)
    test(v.zeroDotD == 0)

    puts "ok"

    print "testing default constructor... "
    STDOUT.flush

    v = Test::StructNoDefaults.new
    test(v.bo == false)
    test(v.b == 0)
    test(v.s == 0)
    test(v.i == 0)
    test(v.l == 0)
    test(v.f == 0.0)
    test(v.d == 0.0)
    test(v.str == '')
    test(v.c1 == Test::Color::Red)
    test(v.bs == nil)
    test(v.is == nil)
    test(v.st.instance_of?(Test::InnerStruct));
    test(v.dict == nil);

    e = Test::ExceptionNoDefaults.new
    test(e.str == '')
    test(e.c1 == Test::Color::Red)
    test(e.bs == nil)
    test(e.st.instance_of?(Test::InnerStruct));
    test(e.dict == nil);

    c = Test::ClassNoDefaults.new
    test(c.str == '')
    test(c.c1 == Test::Color::Red)
    test(c.bs == nil)
    test(c.st.instance_of?(Test::InnerStruct));
    test(c.dict == nil);

    puts("ok")

end
