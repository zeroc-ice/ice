%{
**********************************************************************

Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef Twoways
    methods(Static)
        function twoways(app, p)
            import Test.*;

            communicator = app.communicator();

            literals = p.opStringLiterals();

            assert(strcmp(s0.value, sprintf('\\')) && ...
                   strcmp(s0.value, sw0.value) && ...
                   strcmp(s0.value, literals{1}) && ...
                   strcmp(s0.value, literals{12}));

            assert(strcmp(s1.value, sprintf('A')) && ...
                   strcmp(s1.value, sw1.value) && ...
                   strcmp(s1.value, literals{2}) && ...
                   strcmp(s1.value, literals{13}));

            assert(strcmp(s2.value, sprintf('Ice')) && ...
                   strcmp(s2.value, sw2.value) && ...
                   strcmp(s2.value, literals{3}) && ...
                   strcmp(s2.value, literals{14}));

            assert(strcmp(s3.value, sprintf('A21')) && ...
                   strcmp(s3.value, sw3.value) && ...
                   strcmp(s3.value, literals{4}) && ...
                   strcmp(s3.value, literals{15}));

            assert(strcmp(s4.value, sprintf('\\u0041 \\U00000041')) && ...
                   strcmp(s4.value, sw4.value) && ...
                   strcmp(s4.value, literals{5}) && ...
                   strcmp(s4.value, literals{16}));

            assert(strcmp(s5.value, sprintf('\x00FF')) && ...
                   strcmp(s5.value, sw5.value) && ...
                   strcmp(s5.value, literals{6}) && ...
                   strcmp(s5.value, literals{17}));

            assert(strcmp(s6.value, sprintf('\x03FF')) && ...
                   strcmp(s6.value, sw6.value) && ...
                   strcmp(s6.value, literals{7}) && ...
                   strcmp(s6.value, literals{18}));

            assert(strcmp(s7.value, sprintf('\x05F0')) && ...
                   strcmp(s7.value, sw7.value) && ...
                   strcmp(s7.value, literals{8}) && ...
                   strcmp(s7.value, literals{19}));

            assert(strcmp(s8.value, sprintf('\xD800\xDC00')) && ...
                   strcmp(s8.value, sw8.value) && ...
                   strcmp(s8.value, literals{9}) && ...
                   strcmp(s8.value, literals{20}));

            assert(strcmp(s9.value, sprintf('\xD83C\xDF4C')) && ...
                   strcmp(s9.value, sw9.value) && ...
                   strcmp(s9.value, literals{10}) && ...
                   strcmp(s9.value, literals{21}));

            assert(strcmp(s10.value, sprintf('\x0DA7')) && ...
                   strcmp(s10.value, sw10.value) && ...
                   strcmp(s10.value, literals{11}) && ...
                   strcmp(s10.value, literals{22}));

            assert(strcmp(s11.value, sprintf('\x20ac\x20ac\x20ac')));

            assert(strcmp(s12.value, sprintf('\\101')));

            assert(strcmp(s13.value, sprintf('\xd83c\xdf4c\060\061\062\063')));

            assert(strcmp(s14.value, sprintf('\xd83c\xdf4c\141bcd')));

            assert(strcmp(s15.value, sprintf('\xd83c\xdf4c\101BCD')));

            assert(strcmp(s16.value, sprintf('\177abcd')));

            assert(strcmp(s17.value, sprintf('\177ABCD')));

            assert(strcmp(s18.value, sprintf('\001\061\062\0638')));

            assert(strcmp(s19.value, sprintf('abc\001\061\062\0638\babcd')));

            assert(strcmp(ss0.value, sprintf('''"\x003f\\\x0007\b\f\n\r\t\x000b\6')) && ...
                   strcmp(ss0.value, ss1.value) && ...
                   strcmp(ss0.value, ss2.value) && ...
                   strcmp(ss0.value, literals{23}) && ...
                   strcmp(ss0.value, literals{24}) && ...
                   strcmp(ss0.value, literals{25}));

            assert(strcmp(ss3.value, sprintf('\\\\U\\u\\')) && ...
                   strcmp(ss3.value, literals{26}));

            assert(strcmp(ss4.value, sprintf('\\A\\')) && ...
                   strcmp(ss4.value, literals{27}));

            assert(strcmp(ss5.value, sprintf('\\u0041\\')) && ...
                   strcmp(ss5.value, literals{28}));

            assert(strcmp(su0.value, su1.value) && ...
                   strcmp(su0.value, su2.value) && ...
                   strcmp(su0.value, literals{29}) && ...
                   strcmp(su0.value, literals{30}) && ...
                   strcmp(su0.value, literals{31}));

            p.ice_ping();

            assert(p.ice_isA(MyClassPrx.ice_staticId()));

            assert(strcmp(p.ice_id(), MyDerivedClassPrx.ice_staticId()));

            ids = p.ice_ids();
            assert(length(ids) == 3);
            assert(strcmp(ids{1}, '::Ice::Object'));
            assert(strcmp(ids{2}, '::Test::MyClass'));
            assert(strcmp(ids{3}, '::Test::MyDerivedClass'));

            p.opVoid();

            [r, p3] = p.opByte(hex2dec('ff'), hex2dec('0f'));
            assert(p3 == hex2dec('f0'));
            assert(r == hex2dec('ff'));

            [r, p3] = p.opBool(true, false);
            assert(p3);
            assert(~r);

            [r, p4, p5, p6] = p.opShortIntLong(10, 11, 12);
            assert(p4 == 10);
            assert(p5 == 11);
            assert(p6 == 12);
            assert(r == 12);

            [r, p4, p5, p6] = p.opShortIntLong(intmin('int16'), intmin('int32'), intmin('int64'));
            assert(p4 == intmin('int16'));
            assert(p5 == intmin('int32'));
            assert(p6 == intmin('int64'));
            assert(r == intmin('int64'));

            [r, p4, p5, p6] = p.opShortIntLong(intmax('int16'), intmax('int32'), intmax('int64'));
            assert(p4 == intmax('int16'));
            assert(p5 == intmax('int32'));
            assert(p6 == intmax('int64'));
            assert(r == intmax('int64'));

            [r, p3, p4] = p.opFloatDouble(3.14, 1.1E10);
            assert(p3 == single(3.14));
            assert(p4 == 1.1E10);
            assert(r == 1.1E10);

            [r, p3, p4] = p.opFloatDouble(realmin('single'), realmin('double'));
            assert(p3 == realmin('single'));
            assert(p4 == realmin('double'));
            assert(r == realmin('double'));

            [r, p3, p4] = p.opFloatDouble(realmax('single'), realmax('double'));
            assert(p3 == realmax('single'));
            assert(p4 == realmax('double'));
            assert(r == realmax('double'));

            [r, p3] = p.opString('hello', 'world');
            assert(strcmp(p3, 'world hello'));
            assert(strcmp(r, 'hello world'));

            [r, p2] = p.opMyEnum(MyEnum.enum2);
            assert(p2 == MyEnum.enum2);
            assert(r == MyEnum.enum3);

            %
            % Test marshalling of null enum (first enum value is
            % marshalled in this case).
            %
            [r, p2] = p.opMyEnum([]);
            assert(p2 == MyEnum.enum1);
            assert(r == MyEnum.enum3);

            [r, p2, p3] = p.opMyClass(p);
            assert(Ice.proxyIdentityAndFacetCompare(p2, p) == 0);
            assert(Ice.proxyIdentityAndFacetCompare(p3, p) ~= 0);
            assert(Ice.proxyIdentityAndFacetCompare(r, p) == 0);
            assert(isequal(p2.ice_getIdentity(), Ice.stringToIdentity('test')));
            assert(isequal(p3.ice_getIdentity(), Ice.stringToIdentity('noSuchIdentity')));
            assert(isequal(r.ice_getIdentity(), Ice.stringToIdentity('test')));
            r.opVoid();
            p2.opVoid();
            try
                p3.opVoid();
                assert(false);
            catch ex
                assert(isa(ex, 'Ice.ObjectNotExistException'));
            end

            [r, p2, p3] = p.opMyClass([]);
            assert(isempty(p2));
            assert(~isempty(p3));
            assert(Ice.proxyIdentityAndFacetCompare(r, p) == 0);
            r.opVoid();

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

            [r, p3] = p.opStruct(si1, si2);
            assert(isempty(r.p));
            assert(r.e == MyEnum.enum2);
            assert(strcmp(r.s.s, 'def'));
            assert(p3.p == p);
            assert(p3.e == MyEnum.enum3);
            assert(strcmp(p3.s.s, 'a new string'));
            p3.p.opVoid();

            %
            % Test marshalling of null structs and structs with default member values.
            %
            si1 = Structure();
            si2 = [];

            [r, p3] = p.opStruct(si1, si2);
            assert(isempty(r.p));
            assert(r.e == MyEnum.enum1);
            assert(isempty(r.s.s));
            assert(isempty(p3.p));
            assert(p3.e == MyEnum.enum1);
            assert(strcmp(p3.s.s, 'a new string'));

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

            [r, p3] = p.opByteS(bsi1, bsi2);
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

            [r, p3] = p.opBoolS(bsi1, bsi2);
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

            [r, p4, p5, p6] = p.opShortIntLongS(ssi, isi, lsi);
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

            [r, p3, p4] = p.opFloatDoubleS(fsi, dsi);
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

            [r, p3] = p.opStringS(ssi1, ssi2);
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

            [r, p3] = p.opByteSS(bsi1, bsi2);
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

            [r, p3] = p.opBoolSS(bsi1, bsi2);
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

            [r, p4, p5, p6] = p.opShortIntLongSS(ssi, isi, lsi);
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

            [r, p3, p4] = p.opFloatDoubleSS(fsi, dsi);
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

            [r, p3] = p.opStringSS(ssi1, ssi2);
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

            [r, p3] = p.opStringSSS(sssi1, sssi2);
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

            di1 = containers.Map('KeyType', 'int32', 'ValueType', 'logical');
            di1(10) = true;
            di1(100) = false;
            di2 = containers.Map('KeyType', 'int32', 'ValueType', 'logical');
            di2(10) = true;
            di2(11) = false;
            di2(101) = true;

            [r, p3] = p.opByteBoolD(di1, di2);

            assert(isequal(p3, di1));
            assert(r.Count == 4);
            assert(r(10));
            assert(~r(11));
            assert(~r(100));
            assert(r(101));

            di1 = containers.Map('KeyType', 'int32', 'ValueType', 'int32');
            di1(110) = -1;
            di1(1100) = 123123;
            di2 = containers.Map('KeyType', 'int32', 'ValueType', 'int32');
            di2(110) = -1;
            di2(111) = -100;
            di2(1101) = 0;

            [r, p3] = p.opShortIntD(di1, di2);

            assert(isequal(p3, di1));
            assert(r.Count == 4);
            assert(r(110) == -1);
            assert(r(111) == -100);
            assert(r(1100) == 123123);
            assert(r(1101) == 0);

            di1 = containers.Map('KeyType', 'int64', 'ValueType', 'single');
            di1(999999110) = -1.1;
            di1(999999111) = 123123.2;
            di2 = containers.Map('KeyType', 'int64', 'ValueType', 'single');
            di2(999999110) = -1.1;
            di2(999999120) = -100.4;
            di2(999999130) = 0.5;

            [r, p3] = p.opLongFloatD(di1, di2);

            assert(isequal(p3, di1));
            assert(r.Count == 4);
            assert(r(999999110) == single(-1.1));
            assert(r(999999120) == single(-100.4));
            assert(r(999999111) == single(123123.2));
            assert(r(999999130) == single(0.5));

            di1 = containers.Map('KeyType', 'char', 'ValueType', 'char');
            di1('foo') = 'abc -1.1';
            di1('bar') = 'abc 123123.2';
            di2 = containers.Map('KeyType', 'char', 'ValueType', 'char');
            di2('foo') = 'abc -1.1';
            di2('FOO') = 'abc -100.4';
            di2('BAR') = 'abc 0.5';

            [r, p3] = p.opStringStringD(di1, di2);

            assert(isequal(p3, di1));
            assert(r.Count == 4);
            assert(strcmp(r('foo'), 'abc -1.1'));
            assert(strcmp(r('FOO'), 'abc -100.4'));
            assert(strcmp(r('bar'), 'abc 123123.2'));
            assert(strcmp(r('BAR'), 'abc 0.5'));

            di1 = containers.Map('KeyType', 'char', 'ValueType', 'any');
            di1('abc') = MyEnum.enum1;
            di1('') = MyEnum.enum2;
            di2 = containers.Map('KeyType', 'char', 'ValueType', 'any');
            di2('abc') = MyEnum.enum1;
            di2('qwerty') = MyEnum.enum3;
            di2('Hello!!') = MyEnum.enum2;

            [r, p3] = p.opStringMyEnumD(di1, di2);

            assert(isequal(p3, di1));
            assert(r.Count == 4);
            assert(r('abc') == MyEnum.enum1);
            assert(r('qwerty') == MyEnum.enum3);
            assert(r('') == MyEnum.enum2);
            assert(r('Hello!!') == MyEnum.enum2);

            di1 = containers.Map('KeyType', 'int32', 'ValueType', 'char');
            di1(int32(MyEnum.enum1)) = 'abc';
            di2 = containers.Map('KeyType', 'int32', 'ValueType', 'char');
            di2(int32(MyEnum.enum2)) = 'Hello!!';
            di2(int32(MyEnum.enum3)) = 'qwerty';

            [r, p3] = p.opMyEnumStringD(di1, di2);

            assert(isequal(p3, di1));
            assert(r.Count == 3);
            assert(strcmp(r(int32(MyEnum.enum1)), 'abc'));
            assert(strcmp(r(int32(MyEnum.enum2)), 'Hello!!'));
            assert(strcmp(r(int32(MyEnum.enum3)), 'qwerty'));

            s11 = MyStruct(1, 1);
            s12 = MyStruct(1, 2);
            di1 = struct.empty();
            di1(1).key = s11;
            di1(1).value = MyEnum.enum1;
            di1(2).key = s12;
            di1(2).value = MyEnum.enum2;

            s22 = MyStruct(2, 2);
            s23 = MyStruct(2, 3);
            di2 = struct.empty();
            di2(1).key = s11;
            di2(1).value = MyEnum.enum1;
            di2(2).key = s22;
            di2(2).value = MyEnum.enum3;
            di2(3).key = s23;
            di2(3).value = MyEnum.enum2;

            [r, p3] = p.opMyStructMyEnumD(di1, di2);

            assert(isequal(p3, di1));
            assert(length(r) == 4);
            for i = 1:length(r)
                if isequal(r(i).key, s11)
                    assert(r(i).value == MyEnum.enum1);
                elseif isequal(r(i).key, s12)
                    assert(r(i).value == MyEnum.enum2);
                elseif isequal(r(i).key, s22)
                    assert(r(i).value == MyEnum.enum3);
                elseif isequal(r(i).key, s23)
                    assert(r(i).value == MyEnum.enum2);
                else
                    assert(false);
                end
            end

            di1 = containers.Map('KeyType', 'int32', 'ValueType', 'logical');
            di1(10) = true;
            di1(100) = false;
            di2 = containers.Map('KeyType', 'int32', 'ValueType', 'logical');
            di2(10) = true;
            di2(11) = false;
            di2(101) = true;
            di3 = containers.Map('KeyType', 'int32', 'ValueType', 'logical');
            di3(100) = false;
            di3(101) = false;

            dsi1 = {di1, di2};
            dsi2 = {di3};

            [r, p3] = p.opByteBoolDS(dsi1, dsi2);

            assert(length(r) == 2);
            assert(r{1}.Count == 3);
            assert(r{1}(10));
            assert(~r{1}(11));
            assert(r{1}(101));
            assert(r{2}.Count == 2);
            assert(r{2}(10));
            assert(~r{2}(100));

            assert(length(p3) == 3);
            assert(p3{1}.Count == 2);
            assert(~p3{1}(100));
            assert(~p3{1}(101));
            assert(p3{2}.Count == 2);
            assert(p3{2}(10));
            assert(~p3{2}(100));
            assert(p3{3}.Count == 3);
            assert(p3{3}(10));
            assert(~p3{3}(11));
            assert(p3{3}(101));

            di1 = containers.Map('KeyType', 'int32', 'ValueType', 'int32');
            di1(110) = -1;
            di1(1100) = 123123;
            di2 = containers.Map('KeyType', 'int32', 'ValueType', 'int32');
            di2(110) = -1;
            di2(111) = -100;
            di2(1101) = 0;
            di3 = containers.Map('KeyType', 'int32', 'ValueType', 'int32');
            di3(100) = -1001;

            dsi1 = {di1, di2};
            dsi2 = {di3};

            [r, p3] = p.opShortIntDS(dsi1, dsi2);

            assert(length(r) == 2);
            assert(r{1}.Count == 3);
            assert(r{1}(110) == -1);
            assert(r{1}(111) == -100);
            assert(r{1}(1101) == 0);
            assert(r{2}.Count == 2);
            assert(r{2}(110) == -1);
            assert(r{2}(1100) == 123123);

            assert(length(p3) == 3);
            assert(p3{1}.Count == 1);
            assert(p3{1}(100) == -1001);
            assert(p3{2}.Count == 2);
            assert(p3{2}(110) == -1);
            assert(p3{2}(1100) == 123123);
            assert(p3{3}.Count == 3);
            assert(p3{3}(110) == -1);
            assert(p3{3}(111) == -100);
            assert(p3{3}(1101) == 0);

            di1 = containers.Map('KeyType', 'int64', 'ValueType', 'single');
            di1(999999110) = -1.1;
            di1(999999111) = 123123.2;
            di2 = containers.Map('KeyType', 'int64', 'ValueType', 'single');
            di2(999999110) = -1.1;
            di2(999999120) = -100.4;
            di2(999999130) = 0.5;
            di3 = containers.Map('KeyType', 'int64', 'ValueType', 'single');
            di3(999999140) = 3.14;

            dsi1 = {di1, di2};
            dsi2 = {di3};

            [r, p3] = p.opLongFloatDS(dsi1, dsi2);

            assert(length(r) == 2);
            assert(r{1}.Count == 3);
            assert(r{1}(999999110) == single(-1.1));
            assert(r{1}(999999120) == single(-100.4));
            assert(r{1}(999999130) == single(0.5));
            assert(r{2}.Count == 2);
            assert(r{2}(999999110) == single(-1.1));
            assert(r{2}(999999111) == single(123123.2));

            assert(length(p3) == 3);
            assert(p3{1}.Count == 1);
            assert(p3{1}(999999140) == single(3.14));
            assert(p3{2}.Count == 2);
            assert(p3{2}(999999110) == single(-1.1));
            assert(p3{2}(999999111) == single(123123.2));
            assert(p3{3}.Count == 3);
            assert(p3{3}(999999110) == single(-1.1));
            assert(p3{3}(999999120) == single(-100.4));
            assert(p3{3}(999999130) == single(0.5));

            di1 = containers.Map('KeyType', 'char', 'ValueType', 'char');
            di1('foo') = 'abc -1.1';
            di1('bar') = 'abc 123123.2';
            di2 = containers.Map('KeyType', 'char', 'ValueType', 'char');
            di2('foo') = 'abc -1.1';
            di2('FOO') = 'abc -100.4';
            di2('BAR') = 'abc 0.5';
            di3 = containers.Map('KeyType', 'char', 'ValueType', 'char');
            di3('f00') = 'ABC -3.14';

            dsi1 = {di1, di2};
            dsi2 = {di3};

            [r, p3] = p.opStringStringDS(dsi1, dsi2);

            assert(length(r) == 2);
            assert(r{1}.Count == 3);
            assert(strcmp(r{1}('foo'), 'abc -1.1'));
            assert(strcmp(r{1}('FOO'), 'abc -100.4'));
            assert(strcmp(r{1}('BAR'), 'abc 0.5'));
            assert(r{2}.Count == 2);
            assert(strcmp(r{2}('foo'), 'abc -1.1'));
            assert(strcmp(r{2}('bar'), 'abc 123123.2'));

            assert(length(p3) == 3);
            assert(p3{1}.Count == 1);
            assert(strcmp(p3{1}('f00'), 'ABC -3.14'));
            assert(p3{2}.Count == 2);
            assert(strcmp(p3{2}('foo'), 'abc -1.1'));
            assert(strcmp(p3{2}('bar'), 'abc 123123.2'));
            assert(p3{3}.Count == 3);
            assert(strcmp(p3{3}('foo'), 'abc -1.1'));
            assert(strcmp(p3{3}('FOO'), 'abc -100.4'));
            assert(strcmp(p3{3}('BAR'), 'abc 0.5'));

            di1 = containers.Map('KeyType', 'char', 'ValueType', 'any');
            di1('abc') = MyEnum.enum1;
            di1('') = MyEnum.enum2;
            di2 = containers.Map('KeyType', 'char', 'ValueType', 'any');
            di2('abc') = MyEnum.enum1;
            di2('qwerty') = MyEnum.enum3;
            di2('Hello!!') = MyEnum.enum2;
            di3 = containers.Map('KeyType', 'char', 'ValueType', 'any');
            di3('Goodbye') = MyEnum.enum1;

            dsi1 = {di1, di2};
            dsi2 = {di3};

            [r, p3] = p.opStringMyEnumDS(dsi1, dsi2);

            assert(length(r) == 2);
            assert(r{1}.Count == 3);
            assert(r{1}('abc') == MyEnum.enum1);
            assert(r{1}('qwerty') == MyEnum.enum3);
            assert(r{1}('Hello!!') == MyEnum.enum2);
            assert(r{2}.Count == 2);
            assert(r{2}('abc') == MyEnum.enum1);
            assert(r{2}('') == MyEnum.enum2);

            assert(length(p3) == 3);
            assert(p3{1}.Count == 1);
            assert(p3{1}('Goodbye') == MyEnum.enum1);
            assert(p3{2}.Count == 2);
            assert(p3{2}('abc') == MyEnum.enum1);
            assert(p3{2}('') == MyEnum.enum2);
            assert(p3{3}.Count == 3);
            assert(p3{3}('abc') == MyEnum.enum1);
            assert(p3{3}('qwerty') == MyEnum.enum3);
            assert(p3{3}('Hello!!') == MyEnum.enum2);

            di1 = containers.Map('KeyType', 'int32', 'ValueType', 'char');
            di1(int32(MyEnum.enum1)) = 'abc';
            di2 = containers.Map('KeyType', 'int32', 'ValueType', 'char');
            di2(int32(MyEnum.enum2)) = 'Hello!!';
            di2(int32(MyEnum.enum3)) = 'qwerty';
            di3 = containers.Map('KeyType', 'int32', 'ValueType', 'char');
            di3(int32(MyEnum.enum1)) = 'Goodbye';

            dsi1 = {di1, di2};
            dsi2 = {di3};

            [r, p3] = p.opMyEnumStringDS(dsi1, dsi2);

            assert(length(r) == 2);
            assert(r{1}.Count == 2);
            assert(strcmp(r{1}(int32(MyEnum.enum2)), 'Hello!!'));
            assert(strcmp(r{1}(int32(MyEnum.enum3)), 'qwerty'));
            assert(r{2}.Count == 1);
            assert(strcmp(r{2}(int32(MyEnum.enum1)), 'abc'));

            assert(length(p3) == 3);
            assert(p3{1}.Count == 1);
            assert(strcmp(p3{1}(int32(MyEnum.enum1)), 'Goodbye'));
            assert(p3{2}.Count == 1);
            assert(strcmp(p3{2}(int32(MyEnum.enum1)), 'abc'));
            assert(p3{3}.Count == 2);
            assert(strcmp(p3{3}(int32(MyEnum.enum2)), 'Hello!!'));
            assert(strcmp(p3{3}(int32(MyEnum.enum3)), 'qwerty'));

            s11 = MyStruct(1, 1);
            s12 = MyStruct(1, 2);
            di1 = struct.empty();
            di1(1).key = s11;
            di1(1).value = MyEnum.enum1;
            di1(2).key = s12;
            di1(2).value = MyEnum.enum2;

            s22 = MyStruct(2, 2);
            s23 = MyStruct(2, 3);
            di2 = struct.empty();
            di2(1).key = s11;
            di2(1).value = MyEnum.enum1;
            di2(2).key = s22;
            di2(2).value = MyEnum.enum3;
            di2(3).key = s23;
            di2(3).value = MyEnum.enum2;

            di3 = struct.empty();
            di3(1).key = s23;
            di3(1).value = MyEnum.enum2;

            dsi1 = {di1, di2};
            dsi2 = {di3};

            [r, p3] = p.opMyStructMyEnumDS(dsi1, dsi2);

            function checkStructDict(d)
                import Test.*;
                for i = 1:length(d)
                    if isequal(d(i).key, s11)
                        assert(d(i).value == MyEnum.enum1);
                    elseif isequal(d(i).key, s12)
                        assert(d(i).value == MyEnum.enum2);
                    elseif isequal(d(i).key, s22)
                        assert(d(i).value == MyEnum.enum3);
                    elseif isequal(d(i).key, s23)
                        assert(d(i).value == MyEnum.enum2);
                    else
                        assert(false);
                    end
                end
            end
            assert(length(r) == 2);
            assert(length(r{1}) == 3);
            checkStructDict(r{1});
            checkStructDict(r{2});

            assert(length(p3) == 3);
            assert(length(p3{1}) == 1);
            checkStructDict(p3{1});
            assert(length(p3{2}) == 2);
            checkStructDict(p3{2});
            assert(length(p3{3}) == 3);
            checkStructDict(p3{3});

            sdi1 = containers.Map('KeyType', 'int32', 'ValueType', 'any');
            sdi2 = containers.Map('KeyType', 'int32', 'ValueType', 'any');

            si1 = [hex2dec('01'), hex2dec('11')];
            si2 = [hex2dec('12')];
            si3 = [hex2dec('f2'), hex2dec('f3')];

            sdi1(hex2dec('01')) = si1;
            sdi1(hex2dec('22')) = si2;
            sdi2(hex2dec('f1')) = si3;

            [r, p3] = p.opByteByteSD(sdi1, sdi2);

            assert(p3.Count == 1);
            a = p3(hex2dec('f1')); % Need to use a temp
            assert(length(a) == 2);
            assert(a(1) == hex2dec('f2'));
            assert(a(2) == hex2dec('f3'));
            assert(r.Count == 3);
            a = r(hex2dec('01')); % Need to use a temp
            assert(length(a) == 2);
            assert(a(1) == hex2dec('01'));
            assert(a(2) == hex2dec('11'));
            a = r(hex2dec('22')); % Need to use a temp
            assert(length(a) == 1);
            assert(a(1) == hex2dec('12'));
            a = r(hex2dec('f1')); % Need to use a temp
            assert(length(a) == 2);
            assert(a(1) == hex2dec('f2'));
            assert(a(2) == hex2dec('f3'));

            sdi1 = containers.Map('KeyType', 'int32', 'ValueType', 'any');
            sdi2 = containers.Map('KeyType', 'int32', 'ValueType', 'any');

            si1 = [true, false];
            si2 = [false, true, true];

            sdi1(false) = si1;
            sdi1(true) = si2;
            sdi2(false) = si1;

            [r, p3] = p.opBoolBoolSD(sdi1, sdi2);

            assert(p3.Count == 1);
            a = p3(false); % Need to use temp
            assert(length(a) == 2);
            assert(a(1));
            assert(~a(2));
            assert(r.Count == 2);
            a = r(false); % Need to use temp
            assert(length(a) == 2);
            assert(a(1));
            assert(~a(2));
            a = r(true); % Need to use temp
            assert(length(a) == 3);
            assert(~a(1));
            assert(a(2));
            assert(a(3));

            sdi1 = containers.Map('KeyType', 'int32', 'ValueType', 'any');
            sdi2 = containers.Map('KeyType', 'int32', 'ValueType', 'any');

            si1 = [1, 2, 3];
            si2 = [4, 5];
            si3 = [6, 7];

            sdi1(1) = si1;
            sdi1(2) = si2;
            sdi2(4) = si3;

            [r, p3] = p.opShortShortSD(sdi1, sdi2);

            assert(p3.Count == 1);
            a = p3(4); % Need to use temp
            assert(length(a) == 2);
            assert(a(1) == 6);
            assert(a(2) == 7);
            assert(r.Count == 3);
            a = r(1); % Need to use temp
            assert(length(a) == 3);
            assert(a(1) == 1);
            assert(a(2) == 2);
            assert(a(3) == 3);
            a = r(2); % Need to use temp
            assert(length(a) == 2);
            assert(a(1) == 4);
            assert(a(2) == 5);
            a = r(4); % Need to use temp
            assert(length(a) == 2);
            assert(a(1) == 6);
            assert(a(2) == 7);

            sdi1 = containers.Map('KeyType', 'int32', 'ValueType', 'any');
            sdi2 = containers.Map('KeyType', 'int32', 'ValueType', 'any');

            si1 = [100, 200, 300];
            si2 = [400, 500];
            si3 = [600, 700];

            sdi1(100) = si1;
            sdi1(200) = si2;
            sdi2(400) = si3;

            [r, p3] = p.opIntIntSD(sdi1, sdi2);

            assert(p3.Count == 1);
            a = p3(400); % Need to use temp
            assert(length(a) == 2);
            assert(a(1) == 600);
            assert(a(2) == 700);
            assert(r.Count == 3);
            a = r(100); % Need to use temp
            assert(length(a) == 3);
            assert(a(1) == 100);
            assert(a(2) == 200);
            assert(a(3) == 300);
            a = r(200); % Need to use temp
            assert(length(a) == 2);
            assert(a(1) == 400);
            assert(a(2) == 500);
            a = r(400); % Need to use temp
            assert(length(a) == 2);
            assert(a(1) == 600);
            assert(a(2) == 700);

            sdi1 = containers.Map('KeyType', 'int64', 'ValueType', 'any');
            sdi2 = containers.Map('KeyType', 'int64', 'ValueType', 'any');

            si1 = [999999110, 999999111, 999999110];
            si2 = [999999120, 999999130];
            si3 = [999999110, 999999120];

            sdi1(999999990) = si1;
            sdi1(999999991) = si2;
            sdi2(999999992) = si3;

            [r, p3] = p.opLongLongSD(sdi1, sdi2);

            assert(p3.Count == 1);
            a = p3(999999992); % Need to use temp
            assert(length(a) == 2);
            assert(a(1) == 999999110);
            assert(a(2) == 999999120);
            assert(r.Count == 3);
            a = r(999999990); % Need to use temp
            assert(length(a) == 3);
            assert(a(1) == 999999110);
            assert(a(2) == 999999111);
            assert(a(3) == 999999110);
            a = r(999999991); % Need to use temp
            assert(length(a) == 2);
            assert(a(1) == 999999120);
            assert(a(2) == 999999130);
            a = r(999999992); % Need to use temp
            assert(length(a) == 2);
            assert(a(1) == 999999110);
            assert(a(2) == 999999120);

            sdi1 = containers.Map('KeyType', 'char', 'ValueType', 'any');
            sdi2 = containers.Map('KeyType', 'char', 'ValueType', 'any');

            si1 = [-1.1, 123123.2, 100.0];
            si2 = [42.24, -1.61];
            si3 = [-3.14, 3.14];

            sdi1('abc') = si1;
            sdi1('ABC') = si2;
            sdi2('aBc') = si3;

            [r, p3] = p.opStringFloatSD(sdi1, sdi2);

            assert(p3.Count == 1);
            a = p3('aBc'); % Need to use temp
            assert(length(a) == 2);
            assert(a(1) == single(-3.14));
            assert(a(2) == single(3.14));
            assert(r.Count == 3);
            a = r('abc'); % Need to use temp
            assert(length(a) == 3);
            assert(a(1) == single(-1.1));
            assert(a(2) == single(123123.2));
            assert(a(3) == single(100.0));
            a = r('ABC'); % Need to use temp
            assert(length(a) == 2);
            assert(a(1) == single(42.24));
            assert(a(2) == single(-1.61));
            a = r('aBc'); % Need to use temp
            assert(length(a) == 2);
            assert(a(1) == single(-3.14));
            assert(a(2) == single(3.14));

            sdi1 = containers.Map('KeyType', 'char', 'ValueType', 'any');
            sdi2 = containers.Map('KeyType', 'char', 'ValueType', 'any');

            si1 = [ 1.1E10, 1.2E10, 1.3E10 ];
            si2 = [ 1.4E10, 1.5E10 ];
            si3 = [ 1.6E10, 1.7E10 ];

            sdi1('Hello!!') = si1;
            sdi1('Goodbye') =  si2;
            sdi2('') = si3;

            [r, p3] = p.opStringDoubleSD(sdi1, sdi2);

            assert(p3.Count == 1);
            a = p3(''); % Need to use temp
            assert(length(a) == 2);
            assert(a(1) == 1.6E10);
            assert(a(2) == 1.7E10);
            assert(r.Count== 3);
            a = r('Hello!!'); % Need to use temp
            assert(length(a) == 3);
            assert(a(1) == 1.1E10);
            assert(a(2) == 1.2E10);
            assert(a(3) == 1.3E10);
            a = r('Goodbye'); % Need to use temp
            assert(length(a) == 2);
            assert(a(1) == 1.4E10);
            assert(a(2) == 1.5E10);
            a = r(''); % Need to use temp
            assert(length(a) == 2);
            assert(a(1) == 1.6E10);
            assert(a(2) == 1.7E10);

            sdi1 = containers.Map('KeyType', 'char', 'ValueType', 'any');
            sdi2 = containers.Map('KeyType', 'char', 'ValueType', 'any');

            si1 = { 'abc', 'de', 'fghi' };
            si2 = { 'xyz', 'or' };
            si3 = { 'and', 'xor' };

            sdi1('abc') = si1;
            sdi1('def') = si2;
            sdi2('ghi') = si3;

            [r, p3] = p.opStringStringSD(sdi1, sdi2);

            assert(p3.Count == 1);
            a = p3('ghi'); % Need to use temp
            assert(length(a) == 2);
            assert(strcmp(a(1), 'and'));
            assert(strcmp(a(2), 'xor'));
            assert(r.Count== 3);
            a = r('abc'); % Need to use temp
            assert(length(a) == 3);
            assert(strcmp(a(1), 'abc'));
            assert(strcmp(a(2), 'de'));
            assert(strcmp(a(3), 'fghi'));
            a = r('def'); % Need to use temp
            assert(length(a) == 2);
            assert(strcmp(a(1), 'xyz'));
            assert(strcmp(a(2), 'or'));
            a = r('ghi'); % Need to use temp
            assert(length(a) == 2);
            assert(strcmp(a(1), 'and'));
            assert(strcmp(a(2), 'xor'));

            sdi1 = containers.Map('KeyType', 'int32', 'ValueType', 'any');
            sdi2 = containers.Map('KeyType', 'int32', 'ValueType', 'any');

            si1 = [ MyEnum.enum1, MyEnum.enum1, MyEnum.enum2 ];
            si2 = [ MyEnum.enum1, MyEnum.enum2 ];
            si3 = [ MyEnum.enum3, MyEnum.enum3 ];

            sdi1(int32(MyEnum.enum3)) = si1;
            sdi1(int32(MyEnum.enum2)) = si2;
            sdi2(int32(MyEnum.enum1)) = si3;

            [r, p3] = p.opMyEnumMyEnumSD(sdi1, sdi2);

            assert(p3.Count == 1);
            a = p3(int32(MyEnum.enum1)); % Need to use temp
            assert(length(a) == 2);
            assert(a(1) == MyEnum.enum3);
            assert(a(2) == MyEnum.enum3);
            assert(r.Count== 3);
            a = r(int32(MyEnum.enum3)); % Need to use temp
            assert(length(a) == 3);
            assert(a(1) == MyEnum.enum1);
            assert(a(2) == MyEnum.enum1);
            assert(a(3) == MyEnum.enum2);
            a = r(int32(MyEnum.enum2)); % Need to use temp
            assert(length(a) == 2);
            assert(a(1) == MyEnum.enum1);
            assert(a(2) == MyEnum.enum2);
            a = r(int32(MyEnum.enum1)); % Need to use temp
            assert(length(a) == 2);
            assert(a(1) == MyEnum.enum3);
            assert(a(2) == MyEnum.enum3);

            lengths = [0, 1, 2, 126, 127, 128, 129, 253, 254, 255, 256, 257, 1000];

            for l = lengths
                s = zeros(1, l);
                if l > 0
                    s(1:l) = (1:l);
                end
                r = p.opIntS(s);
                assert(length(r) == l);
                for j = 1:l
                    assert(r(j) == -j);
                end
            end

            ctx = containers.Map('KeyType', 'char', 'ValueType', 'char');
            ctx('one') = 'ONE';
            ctx('two') = 'TWO';
            ctx('three') = 'THREE';

            assert(p.ice_getContext().Count == 0);
            r = p.opContext();
            assert(~isequal(r, ctx));

            r = p.opContext(ctx);
            assert(p.ice_getContext().Count == 0);
            assert(isequal(r, ctx));

            p2 = MyClassPrx.checkedCast(p.ice_context(ctx));
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
                initData = app.cloneInitData();
                initData.properties_.setProperty('Ice.ImplicitContext', impls{i});

                ic = Ice.initialize(initData);

                ctx = containers.Map('KeyType', 'char', 'ValueType', 'char');
                ctx('one') = 'ONE';
                ctx('two') = 'TWO';
                ctx('three') = 'THREE';

                p3 = MyClassPrx.uncheckedCast(ic.stringToProxy(['test:', app.getTestEndpoint(0)]));

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

                prxContext = containers.Map('KeyType', 'char', 'ValueType', 'char');
                prxContext('one') = 'UN';
                prxContext('four') = 'QUATRE';

                combined = containers.Map(ctx.keys(), ctx.values());
                keys = prxContext.keys();
                for j = 1:prxContext.Count
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
            p.opDoubleMarshaling(d, ds);

            p.opIdempotent();

            p.opNonmutating();

            assert(p.opByte1(hex2dec('FF')) == hex2dec('FF'));
            assert(p.opShort1(hex2dec('7FFF')) == hex2dec('7FFF'));
            assert(p.opInt1(hex2dec('7FFFFFFF')) == hex2dec('7FFFFFFF'));
            %assert(p.opLong1(0x7FFFFFFFFFFFFFFF) == 0x7FFFFFFFFFFFFFFF);
            assert(p.opFloat1(1.0) == single(1.0));
            assert(p.opDouble1(1.0) == 1.0);
            assert(strcmp(p.opString1('opString1'), 'opString1'));
            assert(length(p.opStringS1({})) == 0);
            assert(p.opByteBoolD1([]).Count == 0);
            assert(length(p.opStringS2([])) == 0);
            assert(p.opByteBoolD2([]).Count == 0);

            d = MyDerivedClassPrx.uncheckedCast(p);
            s = MyStruct1();
            s.tesT = 'Test.MyStruct1.s';
            s.myClass = [];
            s.myStruct1 = 'Test.MyStruct1.myStruct1';
            s = d.opMyStruct1(s);
            assert(strcmp(s.tesT, 'Test.MyStruct1.s'));
            assert(isempty(s.myClass));
            assert(strcmp(s.myStruct1, 'Test.MyStruct1.myStruct1'));

            c = MyClass1();
            c.tesT = 'Test.MyClass1.testT';
            c.myClass = [];
            c.myClass1 = 'Test.MyClass1.myClass1';
            c = d.opMyClass1(c);
            assert(strcmp(c.tesT, 'Test.MyClass1.testT'));
            assert(isempty(c.myClass));
            assert(strcmp(c.myClass1, 'Test.MyClass1.myClass1'));
        end
    end
end
