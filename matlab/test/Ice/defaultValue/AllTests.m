%{
**********************************************************************

Copyright (c) 2003-present ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef AllTests
    methods(Static)
        function allTests(helper)
            import Test.*;

            communicator = helper.communicator();

            fprintf('testing default values... ');

            v = Struct1();
            assert(~v.boolFalse);
            assert(v.boolTrue);
            assert(v.b == 254);
            assert(v.s == 16000);
            assert(v.i == 3);
            assert(v.l == 4);
            assert(v.f == single(5.1));
            assert(v.d == 6.2);
            assert(strcmp(v.str, sprintf('foo \\ "bar\n \r\n\t\v\f\a\b? \a \a')));
            assert(v.c1 == Color.red);
            assert(v.c2 == Color.green);
            assert(v.c3 == Color.blue);
            assert(v.nc1 == Test.Nested.Color.red);
            assert(v.nc2 == Test.Nested.Color.green);
            assert(v.nc3 == Test.Nested.Color.blue);
            assert(strcmp(v.noDefault, ''));
            assert(v.zeroI == 0);
            assert(v.zeroL == 0);
            assert(v.zeroF == 0);
            assert(v.zeroDotF == 0);
            assert(v.zeroD == 0);
            assert(v.zeroDotD == 0);

            v = Struct2();
            assert(v.boolTrue == ConstBool.value);
            assert(v.b == ConstByte.value);
            assert(v.s == ConstShort.value);
            assert(v.i == ConstInt.value);
            assert(v.l == ConstLong.value);
            assert(v.f == ConstFloat.value);
            assert(v.d == ConstDouble.value);
            assert(strcmp(v.str, ConstString.value));
            assert(v.c1 == ConstColor1.value);
            assert(v.c2 == ConstColor2.value);
            assert(v.c3 == ConstColor3.value);
            assert(v.nc1 == ConstNestedColor1.value);
            assert(v.nc2 == ConstNestedColor2.value);
            assert(v.nc3 == ConstNestedColor3.value);

            v = Base();
            assert(~v.boolFalse);
            assert(v.boolTrue);
            assert(v.b == 1);
            assert(v.s == 2);
            assert(v.i == 3);
            assert(v.l == 4);
            assert(v.f == single(5.1));
            assert(v.d == 6.2);
            assert(strcmp(v.str, sprintf('foo \\ "bar\n \r\n\t\v\f\a\b? \a \a')));
            assert(strcmp(v.noDefault, ''));
            assert(v.zeroI == 0);
            assert(v.zeroL == 0);
            assert(v.zeroF == 0);
            assert(v.zeroDotF == 0);
            assert(v.zeroD == 0);
            assert(v.zeroDotD == 0);

            v = Derived();
            assert(~v.boolFalse);
            assert(v.boolTrue);
            assert(v.b == 1);
            assert(v.s == 2);
            assert(v.i == 3);
            assert(v.l == 4);
            assert(v.f == single(5.1));
            assert(v.d == 6.2);
            assert(strcmp(v.str, sprintf('foo \\ "bar\n \r\n\t\v\f\a\b? \a \a')));
            assert(v.c1 == Color.red);
            assert(v.c2 == Color.green);
            assert(v.c3 == Color.blue);
            assert(v.nc1 == Test.Nested.Color.red);
            assert(v.nc2 == Test.Nested.Color.green);
            assert(v.nc3 == Test.Nested.Color.blue);
            assert(strcmp(v.noDefault, ''));
            assert(v.zeroI == 0);
            assert(v.zeroL == 0);
            assert(v.zeroF == 0);
            assert(v.zeroDotF == 0);
            assert(v.zeroD == 0);
            assert(v.zeroDotD == 0);

            v = BaseEx();
            assert(~v.boolFalse);
            assert(v.boolTrue);
            assert(v.b == 1);
            assert(v.s == 2);
            assert(v.i == 3);
            assert(v.l == 4);
            assert(v.f == single(5.1));
            assert(v.d == 6.2);
            assert(strcmp(v.str, sprintf('foo \\ "bar\n \r\n\t\v\f\a\b? \a \a')));
            assert(strcmp(v.noDefault, ''));
            assert(v.zeroI == 0);
            assert(v.zeroL == 0);
            assert(v.zeroF == 0);
            assert(v.zeroDotF == 0);
            assert(v.zeroD == 0);
            assert(v.zeroDotD == 0);

            v = DerivedEx();
            assert(~v.boolFalse);
            assert(v.boolTrue);
            assert(v.b == 1);
            assert(v.s == 2);
            assert(v.i == 3);
            assert(v.l == 4);
            assert(v.f == single(5.1));
            assert(v.d == 6.2);
            assert(strcmp(v.str, sprintf('foo \\ "bar\n \r\n\t\v\f\a\b? \a \a')));
            assert(strcmp(v.noDefault, ''));
            assert(v.c1 == Color.red);
            assert(v.c2 == Color.green);
            assert(v.c3 == Color.blue);
            assert(v.nc1 == Test.Nested.Color.red);
            assert(v.nc2 == Test.Nested.Color.green);
            assert(v.nc3 == Test.Nested.Color.blue);
            assert(v.zeroI == 0);
            assert(v.zeroL == 0);
            assert(v.zeroF == 0);
            assert(v.zeroDotF == 0);
            assert(v.zeroD == 0);
            assert(v.zeroDotD == 0);

            fprintf('ok\n');

            fprintf('testing default constructor... ');

            v = StructNoDefaults();
            assert(v.bo == false);
            assert(v.b == 0);
            assert(v.s == 0);
            assert(v.i == 0);
            assert(v.l == 0);
            assert(v.f == 0.0);
            assert(v.d == 0.0);
            assert(strcmp(v.str, ''));
            assert(v.c1 == Color.red);
            assert(isempty(v.bs));
            assert(isempty(v.is));
            assert(~isempty(v.st));
            assert(isempty(v.dict));

            e = ExceptionNoDefaults();
            assert(strcmp(e.str, ''));
            assert(e.c1 == Color.red);
            assert(isempty(e.bs));
            assert(~isempty(e.st));
            assert(isempty(e.dict));

            cl = ClassNoDefaults();
            assert(strcmp(cl.str, ''));
            assert(cl.c1 == Color.red);
            assert(isempty(cl.bs));
            assert(~isempty(cl.st));
            assert(isempty(cl.dict));

            fprintf('ok\n');
        end
    end
end
