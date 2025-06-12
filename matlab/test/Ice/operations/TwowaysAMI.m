% Copyright (c) ZeroC, Inc.

classdef TwowaysAMI
    methods(Static)
        function twowaysAMI(helper, p)
            import Test.*;

            call(p, 'ice_ping');

            b = call(p, 'ice_isA', MyClassPrx.ice_staticId());
            assert(b);

            id = call(p, 'ice_id');
            assert(strcmp(id, MyDerivedClassPrx.ice_staticId()));

            ids = call(p, 'ice_ids');
            assert(length(ids) == 3);

            call(p, 'opVoid');

            [r, p3] = call(p, 'opByte', hex2dec('ff'), hex2dec('0f'));
            assert(p3 == hex2dec('f0'));
            assert(r == hex2dec('ff'));

            [r, p3] = call(p, 'opBool', true, false);
            assert(p3);
            assert(~r);

            [r, p4, p5, p6] = call(p, 'opShortIntLong', 10, 11, 12);
            assert(p4 == 10);
            assert(p5 == 11);
            assert(p6 == 12);
            assert(r == 12);

            [r, p3, p4] = call(p, 'opFloatDouble', 3.14, 1.1E10);
            assert(p3 == single(3.14));
            assert(p4 == 1.1E10);
            assert(r == 1.1E10);

            [r, p3] = call(p, 'opString', 'hello', 'world');
            assert(strcmp(p3, 'world hello'));
            assert(strcmp(r, 'hello world'));

            [r, p2] = call(p, 'opMyEnum', MyEnum.enum2);
            assert(p2 == MyEnum.enum2);
            assert(r == MyEnum.enum3);

            [r, p2, p3] = call(p, 'opMyClass', p);
            assert(isequal(p2.ice_getIdentity(), Ice.stringToIdentity('test')));
            assert(isequal(p3.ice_getIdentity(), Ice.stringToIdentity('noSuchIdentity')));
            assert(isequal(r.ice_getIdentity(), Ice.stringToIdentity('test')));

            si1 = Structure();
            si1.p = p;
            si1.e = MyEnum.enum3;
            si1.s = AnotherStruct();
            si1.s.s = 'abc';
            si2 = Structure();
            si2.p = [];
            si2.e = MyEnum.enum2;
            si2.s = AnotherStruct();
            si2.s.s = 'def';

            [r, p3] = call(p, 'opStruct', si1, si2);
            assert(isempty(r.p));
            assert(r.e == MyEnum.enum2);
            assert(strcmp(r.s.s, 'def'));
            assert(p3.p == p);
            assert(p3.e == MyEnum.enum3);
            assert(strcmp(p3.s.s, 'a new string'));
            p3.p.opVoid();

            bsi1 = ...
                    [ ...
                            hex2dec('01'), ...
                            hex2dec('11'), ...
                            hex2dec('12'), ...
                            hex2dec('22') ...
                    ];
            bsi2 = ...
                    [
                            hex2dec('f1'), ...
                            hex2dec('f2'), ...
                            hex2dec('f3'), ...
                            hex2dec('f4') ...
                    ];

            [r, p3] = call(p, 'opByteS', bsi1, bsi2);
            assert(length(p3) == 4);
            assert(p3(1) == hex2dec('22'));
            assert(p3(2) == hex2dec('12'));
            assert(p3(3) == hex2dec('11'));
            assert(p3(4) == hex2dec('01'));
            assert(length(r) == 8);
            assert(r(1) == hex2dec('01'));
            assert(r(2) == hex2dec('11'));
            assert(r(3) == hex2dec('12'));
            assert(r(4) == hex2dec('22'));
            assert(r(5) == hex2dec('f1'));
            assert(r(6) == hex2dec('f2'));
            assert(r(7) == hex2dec('f3'));
            assert(r(8) == hex2dec('f4'));

            bsi1 = [true, true, false];
            bsi2 = [false];

            [r, p3] = call(p, 'opBoolS', bsi1, bsi2);
            assert(length(p3) == 4);
            assert(p3(1));
            assert(p3(2));
            assert(~p3(3));
            assert(~p3(4));
            assert(length(r) == 3);
            assert(~r(1));
            assert(r(2));
            assert(r(3));

            ssi = [1, 2, 3];
            isi = [5, 6, 7, 8];
            lsi = [10, 30, 20];

            [r, p4, p5, p6] = call(p, 'opShortIntLongS', ssi, isi, lsi);
            assert(length(p4) == 3);
            assert(p4(1) == 1);
            assert(p4(2) == 2);
            assert(p4(3) == 3);
            assert(length(p5) == 4);
            assert(p5(1) == 8);
            assert(p5(2) == 7);
            assert(p5(3) == 6);
            assert(p5(4) == 5);
            assert(length(p6) == 6);
            assert(p6(1) == 10);
            assert(p6(2) == 30);
            assert(p6(3) == 20);
            assert(p6(4) == 10);
            assert(p6(5) == 30);
            assert(p6(6) == 20);
            assert(length(r) == 3);
            assert(r(1) == 10);
            assert(r(2) == 30);
            assert(r(3) == 20);

            fsi = [3.14, 1.11];
            dsi = [1.1E10, 1.2E10, 1.3E10];

            [r, p3, p4] = call(p, 'opFloatDoubleS', fsi, dsi);
            assert(length(p3) == 2);
            assert(p3(1) == single(3.14));
            assert(p3(2) == single(1.11));
            assert(length(p4) == 3);
            assert(p4(1) == 1.3E10);
            assert(p4(2) == 1.2E10);
            assert(p4(3) == 1.1E10);
            assert(length(r) == 5);
            assert(r(1) == 1.1E10);
            assert(r(2) == 1.2E10);
            assert(r(3) == 1.3E10);
            assert(r(4) == single(3.14));
            assert(r(5) == single(1.11));

            ssi1 = {'abc', 'de', 'fghi'};
            ssi2 = {'xyz'};

            [r, p3] = call(p, 'opStringS', ssi1, ssi2);
            assert(length(p3) == 4);
            assert(strcmp(p3{1}, 'abc'));
            assert(strcmp(p3{2}, 'de'));
            assert(strcmp(p3{3}, 'fghi'));
            assert(strcmp(p3{4}, 'xyz'));
            assert(length(r) == 3);
            assert(strcmp(r{1}, 'fghi'));
            assert(strcmp(r{2}, 'de'));
            assert(strcmp(r{3}, 'abc'));

            bsi1 = ...
                    { ...
                            [hex2dec('01'), hex2dec('11'), hex2dec('12')], ...
                            [hex2dec('ff')] ...
                    };
            bsi2 = ...
                    { ...
                            [hex2dec('0e')], ...
                            [hex2dec('f2'), hex2dec('f1')] ...
                    };

            [r, p3] = call(p, 'opByteSS', bsi1, bsi2);
            assert(length(p3) == 2);
            assert(length(p3{1}) == 1);
            assert(p3{1}(1) == hex2dec('ff'));
            assert(length(p3{2}) == 3);
            assert(p3{2}(1) == hex2dec('01'));
            assert(p3{2}(2) == hex2dec('11'));
            assert(p3{2}(3) == hex2dec('12'));
            assert(length(r) == 4);
            assert(length(r{1}) == 3);
            assert(r{1}(1) == hex2dec('01'));
            assert(r{1}(2) == hex2dec('11'));
            assert(r{1}(3) == hex2dec('12'));
            assert(length(r{2}) == 1);
            assert(r{2}(1) == hex2dec('ff'));
            assert(length(r{3}) == 1);
            assert(r{3}(1) == hex2dec('0e'));
            assert(length(r{4}) == 2);
            assert(r{4}(1) == hex2dec('f2'));
            assert(r{4}(2) == hex2dec('f1'));

            bsi1 = ...
                    { ...
                            [true], ...
                            [false], ...
                            [true, true] ...
                    };

            bsi2 = ...
                    { ...
                            [false, false, true] ...
                    };

            [r, p3] = call(p, 'opBoolSS', bsi1, bsi2);
            assert(length(p3) == 4);
            assert(length(p3{1}) == 1);
            assert(p3{1}(1));
            assert(length(p3{2}) == 1);
            assert(~p3{2}(1));
            assert(length(p3{3}) == 2);
            assert(p3{3}(1));
            assert(p3{3}(2));
            assert(length(p3{4}) == 3);
            assert(~p3{4}(1));
            assert(~p3{4}(2));
            assert(p3{4}(3));
            assert(length(r) == 3);
            assert(length(r{1}) == 2);
            assert(r{1}(1));
            assert(r{1}(2));
            assert(length(r{2}) == 1);
            assert(~r{2}(1));
            assert(length(r{3}) == 1);
            assert(r{3}(1));

            ssi = ...
                    { ...
                            [1, 2, 5], ...
                            [13], ...
                            [] ...
                    };
            isi = ...
                    { ...
                            [24, 98], ...
                            [42] ...
                    };
            lsi = ...
                    { ...
                            [496, 1729], ...
                    };

            [r, p4, p5, p6] = call(p, 'opShortIntLongSS', ssi, isi, lsi);
            assert(length(r) == 1);
            assert(length(r{1}) == 2);
            assert(r{1}(1) == 496);
            assert(r{1}(2) == 1729);
            assert(length(p4) == 3);
            assert(length(p4{1}) == 3);
            assert(p4{1}(1) == 1);
            assert(p4{1}(2) == 2);
            assert(p4{1}(3) == 5);
            assert(length(p4{2}) == 1);
            assert(p4{2}(1) == 13);
            assert(length(p4{3}) == 0);
            assert(length(p5) == 2);
            assert(length(p5{1}) == 1);
            assert(p5{1}(1) == 42);
            assert(length(p5{2}) == 2);
            assert(p5{2}(1) == 24);
            assert(p5{2}(2) == 98);
            assert(length(p6) == 2);
            assert(length(p6{1}) == 2);
            assert(p6{1}(1) == 496);
            assert(p6{1}(2) == 1729);
            assert(length(p6{2}) == 2);
            assert(p6{2}(1) == 496);
            assert(p6{2}(2) == 1729);

            fsi = ...
                    { ...
                            [3.14], ...
                            [1.11], ...
                            [], ...
                    };
            dsi = ...
                    { ...
                            [1.1E10, 1.2E10, 1.3E10] ...
                    };

            [r, p3, p4] = call(p, 'opFloatDoubleSS', fsi, dsi);
            assert(length(p3) == 3);
            assert(length(p3{1}) == 1);
            assert(p3{1}(1) == single(3.14));
            assert(length(p3{2}) == 1);
            assert(p3{2}(1) == single(1.11));
            assert(length(p3{3}) == 0);
            assert(length(p4) == 1);
            assert(length(p4{1}) == 3);
            assert(p4{1}(1) == 1.1E10);
            assert(p4{1}(2) == 1.2E10);
            assert(p4{1}(3) == 1.3E10);
            assert(length(r) == 2);
            assert(length(r{1}) == 3);
            assert(r{1}(1) == 1.1E10);
            assert(r{1}(2) == 1.2E10);
            assert(r{1}(3) == 1.3E10);
            assert(length(r{2}) == 3);
            assert(r{2}(1) == 1.1E10);
            assert(r{2}(2) == 1.2E10);
            assert(r{2}(3) == 1.3E10);

            ssi1 = ...
                    { ...
                            {'abc'}, ...
                            {'de', 'fghi'} ...
                    };
            ssi2 = ...
                    { ...
                            {}, ...
                            {}, ...
                            {'xyz'} ...
                    };

            [r, p3] = call(p, 'opStringSS', ssi1, ssi2);
            assert(length(p3) == 5);
            assert(length(p3{1}) == 1);
            assert(strcmp(p3{1}(1), 'abc'));
            assert(length(p3{2}) == 2);
            assert(strcmp(p3{2}(1), 'de'));
            assert(strcmp(p3{2}(2), 'fghi'));
            assert(length(p3{3}) == 0);
            assert(length(p3{4}) == 0);
            assert(length(p3{5}) == 1);
            assert(strcmp(p3{5}(1), 'xyz'));
            assert(length(r) == 3);
            assert(length(r{1}) == 1);
            assert(strcmp(r{1}(1), 'xyz'));
            assert(length(r{2}) == 0);
            assert(length(r{3}) == 0);

            sssi1 = ...
                    { ...
                            { ...
                                    { ...
                                            'abc', 'de' ...
                                    }, ...
                                    { ...
                                            'xyz' ...
                                    } ...
                            }, ...
                            { ...
                                    { ...
                                            'hello' ...
                                    } ...
                            } ...
                    };

            sssi2 = ...
                    { ...
                            { ...
                                    { ...
                                            '', '' ...
                                    }, ...
                                    { ...
                                            'abcd' ...
                                    } ...
                            }, ...
                            { ...
                                    { ...
                                            '' ...
                                    } ...
                            }, ...
                            { ...
                            } ...
                    };

            [r, p3] = call(p, 'opStringSSS', sssi1, sssi2);
            assert(length(p3) == 5);
            assert(length(p3{1}) == 2);
            assert(length(p3{1}{1}) == 2);
            assert(length(p3{1}{2}) == 1);
            assert(length(p3{2}) == 1);
            assert(length(p3{2}{1}) == 1);
            assert(length(p3{3}) == 2);
            assert(length(p3{3}{1}) == 2);
            assert(length(p3{3}{2}) == 1);
            assert(length(p3{4}) == 1);
            assert(length(p3{4}{1}) == 1);
            assert(length(p3{5}) == 0);
            assert(strcmp(p3{1}{1}(1), 'abc'));
            assert(strcmp(p3{1}{1}(2), 'de'));
            assert(strcmp(p3{1}{2}(1), 'xyz'));
            assert(strcmp(p3{2}{1}(1), 'hello'));
            assert(strcmp(p3{3}{1}(1), ''));
            assert(strcmp(p3{3}{1}(2), ''));
            assert(strcmp(p3{3}{2}(1), 'abcd'));
            assert(strcmp(p3{4}{1}(1), ''));

            assert(length(r) == 3);
            assert(length(r{1}) == 0);
            assert(length(r{2}) == 1);
            assert(length(r{2}{1}) == 1);
            assert(length(r{3}) == 2);
            assert(length(r{3}{1}) == 2);
            assert(length(r{3}{2}) == 1);
            assert(strcmp(r{2}{1}(1), ''));
            assert(strcmp(r{3}{1}(1), ''));
            assert(strcmp(r{3}{1}(2), ''));
            assert(strcmp(r{3}{2}(1), 'abcd'));

            di1 = configureDictionary('uint8', 'logical');
            di1(10) = true;
            di1(100) = false;
            di2 = configureDictionary('uint8', 'logical');
            di2(10) = true;
            di2(11) = false;
            di2(101) = true;

            [r, p3] = call(p, 'opByteBoolD', di1, di2);

            assert(isequal(p3, di1));
            assert(r.numEntries == 4);
            assert(r(10));
            assert(~r(11));
            assert(~r(100));
            assert(r(101));

            di1 = configureDictionary('int16', 'int32');
            di1(110) = -1;
            di1(1100) = 123123;
            di2 = configureDictionary('int16', 'int32');
            di2(110) = -1;
            di2(111) = -100;
            di2(1101) = 0;

            [r, p3] = call(p, 'opShortIntD', di1, di2);

            assert(isequal(p3, di1));
            assert(r.numEntries == 4);
            assert(r(110) == -1);
            assert(r(111) == -100);
            assert(r(1100) == 123123);
            assert(r(1101) == 0);

            di1 = configureDictionary('int64', 'single');
            di1(999999110) = -1.1;
            di1(999999111) = 123123.2;
            di2 = configureDictionary('int64', 'single');
            di2(999999110) = -1.1;
            di2(999999120) = -100.4;
            di2(999999130) = 0.5;

            [r, p3] = call(p, 'opLongFloatD', di1, di2);

            assert(isequal(p3, di1));
            assert(r.numEntries == 4);
            assert(r(999999110) == single(-1.1));
            assert(r(999999120) == single(-100.4));
            assert(r(999999111) == single(123123.2));
            assert(r(999999130) == single(0.5));

            di1 = configureDictionary('char', 'char');
            di1('foo') = 'abc -1.1';
            di1('bar') = 'abc 123123.2';
            di2 = configureDictionary('char', 'char');
            di2('foo') = 'abc -1.1';
            di2('FOO') = 'abc -100.4';
            di2('BAR') = 'abc 0.5';

            [r, p3] = call(p, 'opStringStringD', di1, di2);

            assert(isequal(p3, di1));
            assert(r.numEntries == 4);
            assert(strcmp(r('foo'), 'abc -1.1'));
            assert(strcmp(r('FOO'), 'abc -100.4'));
            assert(strcmp(r('bar'), 'abc 123123.2'));
            assert(strcmp(r('BAR'), 'abc 0.5'));

            di1 = configureDictionary('char', 'Test.MyEnum');
            di1('abc') = MyEnum.enum1;
            di1('') = MyEnum.enum2;
            di2 = configureDictionary('char', 'Test.MyEnum');
            di2('abc') = MyEnum.enum1;
            di2('qwerty') = MyEnum.enum3;
            di2('Hello!!') = MyEnum.enum2;

            [r, p3] = call(p, 'opStringMyEnumD', di1, di2);

            assert(isequal(p3, di1));
            assert(r.numEntries == 4);
            assert(r('abc') == MyEnum.enum1);
            assert(r('qwerty') == MyEnum.enum3);
            assert(r('') == MyEnum.enum2);
            assert(r('Hello!!') == MyEnum.enum2);

            di1 = configureDictionary('Test.MyEnum', 'char');
            di1(int32(MyEnum.enum1)) = 'abc';
            di2 = configureDictionary('Test.MyEnum', 'char');
            di2(int32(MyEnum.enum2)) = 'Hello!!';
            di2(int32(MyEnum.enum3)) = 'qwerty';

            [r, p3] = call(p, 'opMyEnumStringD', di1, di2);

            assert(isequal(p3, di1));
            assert(r.numEntries == 3);
            assert(strcmp(r(int32(MyEnum.enum1)), 'abc'));
            assert(strcmp(r(int32(MyEnum.enum2)), 'Hello!!'));
            assert(strcmp(r(int32(MyEnum.enum3)), 'qwerty'));

            mys11 = MyStruct(1, 1);
            mys12 = MyStruct(1, 2);
            di1 = configureDictionary('Test.MyStruct', 'Test.MyEnum');
            di1(mys11) = MyEnum.enum1;
            di1(mys12) = MyEnum.enum2;

            mys22 = MyStruct(2, 2);
            mys23 = MyStruct(2, 3);
            di2 = configureDictionary('Test.MyStruct', 'Test.MyEnum');
            di2(mys11) = MyEnum.enum1;
            di2(mys22) = MyEnum.enum3;
            di2(mys23) = MyEnum.enum2;

            [r, p3] = call(p, 'opMyStructMyEnumD', di1, di2);

            assert(isequal(p3, di1));
            assert(r.numEntries == 4);
            assert(isequal(r(mys11), MyEnum.enum1));
            assert(isequal(r(mys12), MyEnum.enum2));
            assert(isequal(r(mys22), MyEnum.enum3));
            assert(isequal(r(mys23), MyEnum.enum2));

            di1 = configureDictionary('uint8', 'logical');
            di1(10) = true;
            di1(100) = false;
            di2 = configureDictionary('uint8', 'logical');
            di2(10) = true;
            di2(11) = false;
            di2(101) = true;
            di3 = configureDictionary('uint8', 'logical');
            di3(100) = false;
            di3(101) = false;

            dsi1 = {di1, di2};
            dsi2 = {di3};

            [r, p3] = call(p, 'opByteBoolDS', dsi1, dsi2);

            assert(length(r) == 2);
            assert(r{1}.numEntries == 3);
            assert(r{1}(10));
            assert(~r{1}(11));
            assert(r{1}(101));
            assert(r{2}.numEntries == 2);
            assert(r{2}(10));
            assert(~r{2}(100));

            assert(length(p3) == 3);
            assert(p3{1}.numEntries == 2);
            assert(~p3{1}(100));
            assert(~p3{1}(101));
            assert(p3{2}.numEntries == 2);
            assert(p3{2}(10));
            assert(~p3{2}(100));
            assert(p3{3}.numEntries == 3);
            assert(p3{3}(10));
            assert(~p3{3}(11));
            assert(p3{3}(101));

            di1 = configureDictionary('int16', 'int32');
            di1(110) = -1;
            di1(1100) = 123123;
            di2 = configureDictionary('int16', 'int32');
            di2(110) = -1;
            di2(111) = -100;
            di2(1101) = 0;
            di3 = configureDictionary('int16', 'int32');
            di3(100) = -1001;

            dsi1 = {di1, di2};
            dsi2 = {di3};

            [r, p3] = call(p, 'opShortIntDS', dsi1, dsi2);

            assert(length(r) == 2);
            assert(r{1}.numEntries == 3);
            assert(r{1}(110) == -1);
            assert(r{1}(111) == -100);
            assert(r{1}(1101) == 0);
            assert(r{2}.numEntries == 2);
            assert(r{2}(110) == -1);
            assert(r{2}(1100) == 123123);

            assert(length(p3) == 3);
            assert(p3{1}.numEntries == 1);
            assert(p3{1}(100) == -1001);
            assert(p3{2}.numEntries == 2);
            assert(p3{2}(110) == -1);
            assert(p3{2}(1100) == 123123);
            assert(p3{3}.numEntries == 3);
            assert(p3{3}(110) == -1);
            assert(p3{3}(111) == -100);
            assert(p3{3}(1101) == 0);

            di1 = configureDictionary('int64', 'single');
            di1(999999110) = -1.1;
            di1(999999111) = 123123.2;
            di2 = configureDictionary('int64', 'single');
            di2(999999110) = -1.1;
            di2(999999120) = -100.4;
            di2(999999130) = 0.5;
            di3 = configureDictionary('int64', 'single');
            di3(999999140) = 3.14;

            dsi1 = {di1, di2};
            dsi2 = {di3};

            [r, p3] = call(p, 'opLongFloatDS', dsi1, dsi2);

            assert(length(r) == 2);
            assert(r{1}.numEntries == 3);
            assert(r{1}(999999110) == single(-1.1));
            assert(r{1}(999999120) == single(-100.4));
            assert(r{1}(999999130) == single(0.5));
            assert(r{2}.numEntries == 2);
            assert(r{2}(999999110) == single(-1.1));
            assert(r{2}(999999111) == single(123123.2));

            assert(length(p3) == 3);
            assert(p3{1}.numEntries == 1);
            assert(p3{1}(999999140) == single(3.14));
            assert(p3{2}.numEntries == 2);
            assert(p3{2}(999999110) == single(-1.1));
            assert(p3{2}(999999111) == single(123123.2));
            assert(p3{3}.numEntries == 3);
            assert(p3{3}(999999110) == single(-1.1));
            assert(p3{3}(999999120) == single(-100.4));
            assert(p3{3}(999999130) == single(0.5));

            di1 = configureDictionary('char', 'char');
            di1('foo') = 'abc -1.1';
            di1('bar') = 'abc 123123.2';
            di2 = configureDictionary('char', 'char');
            di2('foo') = 'abc -1.1';
            di2('FOO') = 'abc -100.4';
            di2('BAR') = 'abc 0.5';
            di3 = configureDictionary('char', 'char');
            di3('f00') = 'ABC -3.14';

            dsi1 = {di1, di2};
            dsi2 = {di3};

            [r, p3] = call(p, 'opStringStringDS', dsi1, dsi2);

            assert(length(r) == 2);
            assert(r{1}.numEntries == 3);
            assert(strcmp(r{1}('foo'), 'abc -1.1'));
            assert(strcmp(r{1}('FOO'), 'abc -100.4'));
            assert(strcmp(r{1}('BAR'), 'abc 0.5'));
            assert(r{2}.numEntries == 2);
            assert(strcmp(r{2}('foo'), 'abc -1.1'));
            assert(strcmp(r{2}('bar'), 'abc 123123.2'));

            assert(length(p3) == 3);
            assert(p3{1}.numEntries == 1);
            assert(strcmp(p3{1}('f00'), 'ABC -3.14'));
            assert(p3{2}.numEntries == 2);
            assert(strcmp(p3{2}('foo'), 'abc -1.1'));
            assert(strcmp(p3{2}('bar'), 'abc 123123.2'));
            assert(p3{3}.numEntries == 3);
            assert(strcmp(p3{3}('foo'), 'abc -1.1'));
            assert(strcmp(p3{3}('FOO'), 'abc -100.4'));
            assert(strcmp(p3{3}('BAR'), 'abc 0.5'));

            di1 = configureDictionary('char', 'Test.MyEnum');
            di1('abc') = MyEnum.enum1;
            di1('') = MyEnum.enum2;
            di2 = configureDictionary('char', 'Test.MyEnum');
            di2('abc') = MyEnum.enum1;
            di2('qwerty') = MyEnum.enum3;
            di2('Hello!!') = MyEnum.enum2;
            di3 = configureDictionary('char', 'Test.MyEnum');
            di3('Goodbye') = MyEnum.enum1;

            dsi1 = {di1, di2};
            dsi2 = {di3};

            [r, p3] = call(p, 'opStringMyEnumDS', dsi1, dsi2);

            assert(length(r) == 2);
            assert(r{1}.numEntries == 3);
            assert(r{1}('abc') == MyEnum.enum1);
            assert(r{1}('qwerty') == MyEnum.enum3);
            assert(r{1}('Hello!!') == MyEnum.enum2);
            assert(r{2}.numEntries == 2);
            assert(r{2}('abc') == MyEnum.enum1);
            assert(r{2}('') == MyEnum.enum2);

            assert(length(p3) == 3);
            assert(p3{1}.numEntries == 1);
            assert(p3{1}('Goodbye') == MyEnum.enum1);
            assert(p3{2}.numEntries == 2);
            assert(p3{2}('abc') == MyEnum.enum1);
            assert(p3{2}('') == MyEnum.enum2);
            assert(p3{3}.numEntries == 3);
            assert(p3{3}('abc') == MyEnum.enum1);
            assert(p3{3}('qwerty') == MyEnum.enum3);
            assert(p3{3}('Hello!!') == MyEnum.enum2);

            di1 = configureDictionary('Test.MyEnum', 'char');
            di1(int32(MyEnum.enum1)) = 'abc';
            di2 = configureDictionary('Test.MyEnum', 'char');
            di2(int32(MyEnum.enum2)) = 'Hello!!';
            di2(int32(MyEnum.enum3)) = 'qwerty';
            di3 = configureDictionary('Test.MyEnum', 'char');
            di3(int32(MyEnum.enum1)) = 'Goodbye';

            dsi1 = {di1, di2};
            dsi2 = {di3};

            [r, p3] = call(p, 'opMyEnumStringDS', dsi1, dsi2);

            assert(length(r) == 2);
            assert(r{1}.numEntries == 2);
            assert(strcmp(r{1}(int32(MyEnum.enum2)), 'Hello!!'));
            assert(strcmp(r{1}(int32(MyEnum.enum3)), 'qwerty'));
            assert(r{2}.numEntries == 1);
            assert(strcmp(r{2}(int32(MyEnum.enum1)), 'abc'));

            assert(length(p3) == 3);
            assert(p3{1}.numEntries == 1);
            assert(strcmp(p3{1}(int32(MyEnum.enum1)), 'Goodbye'));
            assert(p3{2}.numEntries == 1);
            assert(strcmp(p3{2}(int32(MyEnum.enum1)), 'abc'));
            assert(p3{3}.numEntries == 2);
            assert(strcmp(p3{3}(int32(MyEnum.enum2)), 'Hello!!'));
            assert(strcmp(p3{3}(int32(MyEnum.enum3)), 'qwerty'));

            mys11 = MyStruct(1, 1);
            mys12 = MyStruct(1, 2);
            di1 = configureDictionary('Test.MyStruct', 'Test.MyEnum');
            di1(mys11) = MyEnum.enum1;
            di1(mys12) = MyEnum.enum2;

            mys22 = MyStruct(2, 2);
            mys23 = MyStruct(2, 3);
            di2 = configureDictionary('Test.MyStruct', 'Test.MyEnum');
            di2(mys11) = MyEnum.enum1;
            di2(mys22) = MyEnum.enum3;
            di2(mys23) = MyEnum.enum2;

            di3 = configureDictionary('Test.MyStruct', 'Test.MyEnum');
            di3(mys23) = MyEnum.enum2;

            dsi1 = {di1, di2};
            dsi2 = {di3};

            [r, p3] = call(p, 'opMyStructMyEnumDS', dsi1, dsi2);

            assert(length(r) == 2);
            assert(r{1}.numEntries == 3);
            assert(r{1}(mys11) == MyEnum.enum1);
            assert(r{1}(mys22) == MyEnum.enum3);
            assert(r{1}(mys23) == MyEnum.enum2);

            assert(r{2}.numEntries == 2);
            assert(r{2}(mys11) == MyEnum.enum1);
            assert(r{2}(mys12) == MyEnum.enum2);

            assert(length(p3) == 3);
            assert(p3{1}.numEntries == 1);
            assert(p3{1}(mys23) == MyEnum.enum2);
            assert(p3{2}.numEntries == 2);
            assert(p3{2}(mys11) == MyEnum.enum1);
            assert(p3{2}(mys12) == MyEnum.enum2);
            assert(p3{3}.numEntries == 3);
            assert(p3{3}(mys11) == MyEnum.enum1);
            assert(p3{3}(mys22) == MyEnum.enum3);
            assert(p3{3}(mys23) == MyEnum.enum2);

            sdi1 = configureDictionary('uint8', 'cell');
            sdi2 = configureDictionary('uint8', 'cell');

            si1 = [hex2dec('01'), hex2dec('11')];
            si2 = [hex2dec('12')];
            si3 = [hex2dec('f2'), hex2dec('f3')];

            sdi1{hex2dec('01')} = si1;
            sdi1{hex2dec('22')} = si2;
            sdi2{hex2dec('f1')} = si3;

            [r, p3] = call(p, 'opByteByteSD', sdi1, sdi2);

            assert(p3.numEntries == 1);
            a = p3{hex2dec('f1')}; % Need to use a temp
            assert(length(a) == 2);
            assert(a(1) == hex2dec('f2'));
            assert(a(2) == hex2dec('f3'));
            assert(r.numEntries == 3);
            a = r{hex2dec('01')}; % Need to use a temp
            assert(length(a) == 2);
            assert(a(1) == hex2dec('01'));
            assert(a(2) == hex2dec('11'));
            a = r{hex2dec('22')}; % Need to use a temp
            assert(length(a) == 1);
            assert(a(1) == hex2dec('12'));
            a = r{hex2dec('f1')}; % Need to use a temp
            assert(length(a) == 2);
            assert(a(1) == hex2dec('f2'));
            assert(a(2) == hex2dec('f3'));

            sdi1 = configureDictionary('logical', 'cell');
            sdi2 = configureDictionary('logical', 'cell');

            si1 = [true, false];
            si2 = [false, true, true];

            sdi1{false} = si1;
            sdi1{true} = si2;
            sdi2{false} = si1;

            [r, p3] = call(p, 'opBoolBoolSD', sdi1, sdi2);

            assert(p3.numEntries == 1);
            a = p3{false}; % Need to use temp
            assert(length(a) == 2);
            assert(a(1));
            assert(~a(2));
            assert(r.numEntries == 2);
            a = r{false}; % Need to use temp
            assert(length(a) == 2);
            assert(a(1));
            assert(~a(2));
            a = r{true}; % Need to use temp
            assert(length(a) == 3);
            assert(~a(1));
            assert(a(2));
            assert(a(3));

            sdi1 = configureDictionary('int16', 'cell');
            sdi2 =configureDictionary('int16', 'cell');

            si1 = [1, 2, 3];
            si2 = [4, 5];
            si3 = [6, 7];

            sdi1{1} = si1;
            sdi1{2} = si2;
            sdi2{4} = si3;

            [r, p3] = call(p, 'opShortShortSD', sdi1, sdi2);

            assert(p3.numEntries == 1);
            a = p3{4}; % Need to use temp
            assert(length(a) == 2);
            assert(a(1) == 6);
            assert(a(2) == 7);
            assert(r.numEntries == 3);
            a = r{1}; % Need to use temp
            assert(length(a) == 3);
            assert(a(1) == 1);
            assert(a(2) == 2);
            assert(a(3) == 3);
            a = r{2}; % Need to use temp
            assert(length(a) == 2);
            assert(a(1) == 4);
            assert(a(2) == 5);
            a = r{4}; % Need to use temp
            assert(length(a) == 2);
            assert(a(1) == 6);
            assert(a(2) == 7);

            sdi1 = configureDictionary('int32', 'cell');
            sdi2 = configureDictionary('int32', 'cell');

            si1 = [100, 200, 300];
            si2 = [400, 500];
            si3 = [600, 700];

            sdi1{100} = si1;
            sdi1{200} = si2;
            sdi2{400} = si3;

            [r, p3] = call(p, 'opIntIntSD', sdi1, sdi2);

            assert(p3.numEntries == 1);
            a = p3{400}; % Need to use temp
            assert(length(a) == 2);
            assert(a(1) == 600);
            assert(a(2) == 700);
            assert(r.numEntries == 3);
            a = r{100}; % Need to use temp
            assert(length(a) == 3);
            assert(a(1) == 100);
            assert(a(2) == 200);
            assert(a(3) == 300);
            a = r{200}; % Need to use temp
            assert(length(a) == 2);
            assert(a(1) == 400);
            assert(a(2) == 500);
            a = r{400}; % Need to use temp
            assert(length(a) == 2);
            assert(a(1) == 600);
            assert(a(2) == 700);

            sdi1 = configureDictionary('int64', 'cell');
            sdi2 = configureDictionary('int64', 'cell');

            si1 = [999999110, 999999111, 999999110];
            si2 = [999999120, 999999130];
            si3 = [999999110, 999999120];

            sdi1{999999990} = si1;
            sdi1{999999991} = si2;
            sdi2{999999992} = si3;

            [r, p3] = call(p, 'opLongLongSD', sdi1, sdi2);

            assert(p3.numEntries == 1);
            a = p3{999999992}; % Need to use temp
            assert(length(a) == 2);
            assert(a(1) == 999999110);
            assert(a(2) == 999999120);
            assert(r.numEntries == 3);
            a = r{999999990}; % Need to use temp
            assert(length(a) == 3);
            assert(a(1) == 999999110);
            assert(a(2) == 999999111);
            assert(a(3) == 999999110);
            a = r{999999991}; % Need to use temp
            assert(length(a) == 2);
            assert(a(1) == 999999120);
            assert(a(2) == 999999130);
            a = r{999999992}; % Need to use temp
            assert(length(a) == 2);
            assert(a(1) == 999999110);
            assert(a(2) == 999999120);

            sdi1 = configureDictionary('char', 'cell');
            sdi2 = configureDictionary('char', 'cell');

            si1 = [-1.1, 123123.2, 100.0];
            si2 = [42.24, -1.61];
            si3 = [-3.14, 3.14];

            sdi1{'abc'} = si1;
            sdi1{'ABC'} = si2;
            sdi2{'aBc'} = si3;

            [r, p3] = call(p, 'opStringFloatSD', sdi1, sdi2);

            assert(p3.numEntries == 1);
            a = p3{'aBc'}; % Need to use temp
            assert(length(a) == 2);
            assert(a(1) == single(-3.14));
            assert(a(2) == single(3.14));
            assert(r.numEntries == 3);
            a = r{'abc'}; % Need to use temp
            assert(length(a) == 3);
            assert(a(1) == single(-1.1));
            assert(a(2) == single(123123.2));
            assert(a(3) == single(100.0));
            a = r{'ABC'}; % Need to use temp
            assert(length(a) == 2);
            assert(a(1) == single(42.24));
            assert(a(2) == single(-1.61));
            a = r{'aBc'}; % Need to use temp
            assert(length(a) == 2);
            assert(a(1) == single(-3.14));
            assert(a(2) == single(3.14));

            sdi1 = configureDictionary('char', 'cell');
            sdi2 = configureDictionary('char', 'cell');

            si1 = [ 1.1E10, 1.2E10, 1.3E10 ];
            si2 = [ 1.4E10, 1.5E10 ];
            si3 = [ 1.6E10, 1.7E10 ];

            sdi1{'Hello!!'} = si1;
            sdi1{'Goodbye'} =  si2;
            sdi2{''} = si3;

            [r, p3] = call(p, 'opStringDoubleSD', sdi1, sdi2);

            assert(p3.numEntries == 1);
            a = p3{''}; % Need to use temp
            assert(length(a) == 2);
            assert(a(1) == 1.6E10);
            assert(a(2) == 1.7E10);
            assert(r.numEntries== 3);
            a = r{'Hello!!'}; % Need to use temp
            assert(length(a) == 3);
            assert(a(1) == 1.1E10);
            assert(a(2) == 1.2E10);
            assert(a(3) == 1.3E10);
            a = r{'Goodbye'}; % Need to use temp
            assert(length(a) == 2);
            assert(a(1) == 1.4E10);
            assert(a(2) == 1.5E10);
            a = r{''}; % Need to use temp
            assert(length(a) == 2);
            assert(a(1) == 1.6E10);
            assert(a(2) == 1.7E10);

            sdi1 = configureDictionary('char', 'cell');
            sdi2 = configureDictionary('char', 'cell');

            si1 = { 'abc', 'de', 'fghi' };
            si2 = { 'xyz', 'or' };
            si3 = { 'and', 'xor' };

            sdi1{'abc'} = si1;
            sdi1{'def'} = si2;
            sdi2{'ghi'} = si3;

            [r, p3] = call(p, 'opStringStringSD', sdi1, sdi2);

            assert(p3.numEntries == 1);
            a = p3{'ghi'}; % Need to use temp
            assert(length(a) == 2);
            assert(strcmp(a(1), 'and'));
            assert(strcmp(a(2), 'xor'));
            assert(r.numEntries== 3);
            a = r{'abc'}; % Need to use temp
            assert(length(a) == 3);
            assert(strcmp(a(1), 'abc'));
            assert(strcmp(a(2), 'de'));
            assert(strcmp(a(3), 'fghi'));
            a = r{'def'}; % Need to use temp
            assert(length(a) == 2);
            assert(strcmp(a(1), 'xyz'));
            assert(strcmp(a(2), 'or'));
            a = r{'ghi'}; % Need to use temp
            assert(length(a) == 2);
            assert(strcmp(a(1), 'and'));
            assert(strcmp(a(2), 'xor'));

            sdi1 = configureDictionary('Test.MyEnum', 'cell');
            sdi2 = configureDictionary('Test.MyEnum', 'cell');

            si1 = [ MyEnum.enum1, MyEnum.enum1, MyEnum.enum2 ];
            si2 = [ MyEnum.enum1, MyEnum.enum2 ];
            si3 = [ MyEnum.enum3, MyEnum.enum3 ];

            sdi1{MyEnum.enum3} = si1;
            sdi1{MyEnum.enum2} = si2;
            sdi2{MyEnum.enum1} = si3;

            [r, p3] = call(p, 'opMyEnumMyEnumSD', sdi1, sdi2);

            assert(p3.numEntries == 1);
            a = p3{MyEnum.enum1}; % Need to use temp
            assert(length(a) == 2);
            assert(a(1) == MyEnum.enum3);
            assert(a(2) == MyEnum.enum3);
            assert(r.numEntries== 3);
            a = r{MyEnum.enum3}; % Need to use temp
            assert(length(a) == 3);
            assert(a(1) == MyEnum.enum1);
            assert(a(2) == MyEnum.enum1);
            assert(a(3) == MyEnum.enum2);
            a = r{MyEnum.enum2}; % Need to use temp
            assert(length(a) == 2);
            assert(a(1) == MyEnum.enum1);
            assert(a(2) == MyEnum.enum2);
            a = r{MyEnum.enum1}; % Need to use temp
            assert(length(a) == 2);
            assert(a(1) == MyEnum.enum3);
            assert(a(2) == MyEnum.enum3);

            lengths = [0, 1, 2, 126, 127, 128, 129, 253, 254, 255, 256, 257, 1000];

            for l = lengths
                s = zeros(1, l);
                if l > 0
                    s(1:l) = (1:l);
                end
                r = call(p, 'opIntS', s);
                assert(length(r) == l);
                for j = 1:l
                    assert(r(j) == -j);
                end
            end

            ctx = configureDictionary('char', 'char');
            ctx('one') = 'ONE';
            ctx('two') = 'TWO';
            ctx('three') = 'THREE';

            assert(p.ice_getContext().numEntries == 0);
            r = call(p, 'opContext');
            assert(~isequal(r, ctx));

            r = call(p, 'opContext', ctx);
            assert(p.ice_getContext().numEntries == 0);
            assert(isequal(r, ctx));

            p2 = p.ice_context(ctx);
            assert(isequal(p2.ice_getContext(), ctx));
            r = p2.opContext();
            assert(isequal(r, ctx));
            r = p2.opContext(ctx);
            assert(isequal(r, ctx));

            %
            % Test implicit context propagation
            %
            impls = {'Shared', 'PerThread'};
            for i = 1:2
                properties = helper.communicator().getProperties().clone();
                properties.setProperty('Ice.ImplicitContext', impls{i});

                ic = helper.initialize(properties);

                ctx = configureDictionary('char', 'char');
                ctx('one') = 'ONE';
                ctx('two') = 'TWO';
                ctx('three') = 'THREE';

                p3 = MyClassPrx(ic, ['test:', helper.getTestEndpoint()]);

                ic.getImplicitContext().setContext(ctx);
                assert(isequal(ic.getImplicitContext().getContext(), ctx));
                assert(isequal(p3.opContext(), ctx));

                assert(~ic.getImplicitContext().containsKey('zero'));
                r = ic.getImplicitContext().put('zero', 'ZERO');
                assert(strcmp(r, ''));
                assert(ic.getImplicitContext().containsKey('zero'));
                assert(strcmp(ic.getImplicitContext().get('zero'), 'ZERO'));

                ctx = ic.getImplicitContext().getContext();
                assert(isequal(p3.opContext(), ctx));

                prxContext = configureDictionary('char', 'char');
                prxContext('one') = 'UN';
                prxContext('four') = 'QUATRE';

                combined = dictionary(ctx.keys(), ctx.values());
                keys = prxContext.keys();
                for j = 1:prxContext.numEntries
                    combined(keys{j}) = prxContext(keys{j});
                end
                assert(strcmp(combined('one'), 'UN'));

                p3 = p3.ice_context(prxContext);

                ic.getImplicitContext().setContext([]);
                assert(isequal(p3.opContext(), prxContext));

                ic.getImplicitContext().setContext(ctx);
                assert(isequal(p3.opContext(), combined));

                assert(strcmp(ic.getImplicitContext().remove('one'), 'ONE'));

                ic.destroy();
            end

            d = 1278312346.0 / 13.0;
            ds = zeros(1, 5);
            for i = 1:5
                ds(i) = d;
            end
            call(p, 'opDoubleMarshaling', d, ds);

            call(p, 'opIdempotent');

            assert(call(p, 'opByte1', hex2dec('FF')) == hex2dec('FF'));
            assert(call(p, 'opShort1', hex2dec('7FFF')) == hex2dec('7FFF'));
            assert(call(p, 'opInt1', hex2dec('7FFFFFFF')) == hex2dec('7FFFFFFF'));
            assert(call(p, 'opLong1', int64(9223372036854775807)) == int64(9223372036854775807)); % 0x7FFFFFFFFFFFFFFF
            assert(call(p, 'opFloat1', 1.0) == single(1.0));
            assert(call(p, 'opDouble1', 1.0) == 1.0);
            assert(strcmp(call(p, 'opString1', 'opString1'), 'opString1'));
            assert(length(call(p, 'opStringS1', {})) == 0);
            empty = call(p, 'opByteBoolD1', []);
            assert(empty.numEntries == 0);
            assert(length(call(p, 'opStringS2', [])) == 0);
            empty = call(p, 'opByteBoolD2', []);
            assert(empty.numEntries == 0);

            d = MyDerivedClassPrx.uncheckedCast(p);
            s = MyStruct1();
            s.tesT = 'Test.MyStruct1.s';
            s.myClass = [];
            s.myStruct1 = 'Test.MyStruct1.myStruct1';
            s = call(d, 'opMyStruct1', s);
            assert(strcmp(s.tesT, 'Test.MyStruct1.s'));
            assert(isempty(s.myClass));
            assert(strcmp(s.myStruct1, 'Test.MyStruct1.myStruct1'));

            c = MyClass1();
            c.tesT = 'Test.MyClass1.testT';
            c.myClass = [];
            c.myClass1 = 'Test.MyClass1.myClass1';
            c = call(d, 'opMyClass1', c);
            assert(strcmp(c.tesT, 'Test.MyClass1.testT'));
            assert(isempty(c.myClass));
            assert(strcmp(c.myClass1, 'Test.MyClass1.myClass1'));
        end
    end
end

function varargout = call(p, op, varargin)
    name = [op, 'Async'];
    future = p.(name)(varargin{:});
    assert(strcmp(future.Operation, op));
    assert(~future.Read);
    assert(future.wait());
    assert(strcmp(future.State, 'finished'));
    [varargout{1:nargout}] = future.fetchOutputs();
    assert(future.Read);
end
