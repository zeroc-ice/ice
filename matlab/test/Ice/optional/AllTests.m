%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef AllTests
    methods(Static)
        function r = allTests(app)
            import test.Ice.optional.Test.*;

            communicator = app.communicator();

            ref = ['initial:', app.getTestEndpoint(0, '')];
            base = communicator.stringToProxy(ref);
            initial = InitialPrx.checkedCast(base);

            fprintf('testing optional data members... ');

            oo1 = OneOptional();
            assert(oo1.a == Ice.Unset);
            oo1.a = 15;

            oo2 = OneOptional(16);
            assert(oo2.a == 16);

            mo1 = MultiOptional();
            assert(mo1.a == Ice.Unset);
            assert(mo1.b == Ice.Unset);
            assert(mo1.c == Ice.Unset);
            assert(mo1.d == Ice.Unset);
            assert(mo1.e == Ice.Unset);
            assert(mo1.f == Ice.Unset);
            assert(mo1.g == Ice.Unset);
            assert(mo1.h == Ice.Unset);
            assert(mo1.i == Ice.Unset);
            assert(mo1.j == Ice.Unset);
            assert(mo1.k == Ice.Unset);
            assert(mo1.bs == Ice.Unset);
            assert(mo1.ss == Ice.Unset);
            assert(mo1.iid == Ice.Unset);
            assert(mo1.sid == Ice.Unset);
            assert(mo1.fs == Ice.Unset);
            assert(mo1.vs == Ice.Unset);

            assert(mo1.shs == Ice.Unset);
            assert(mo1.es == Ice.Unset);
            assert(mo1.fss == Ice.Unset);
            assert(mo1.vss == Ice.Unset);
            assert(mo1.oos == Ice.Unset);
            assert(mo1.oops == Ice.Unset);

            assert(mo1.ied == Ice.Unset);
            assert(mo1.ifsd == Ice.Unset);
            assert(mo1.ivsd == Ice.Unset);
            assert(mo1.iood == Ice.Unset);
            assert(mo1.ioopd == Ice.Unset);

            assert(mo1.bos == Ice.Unset);

            ss = SmallStruct();
            fs = FixedStruct(78);
            vs = VarStruct('hello');
            iid = IntIntDict.new();
            iid(4) = 3;
            sid = StringIntDict.new();
            sid('test') = 10;
            ied = IntEnumDict.new();
            ied(4) = MyEnum.MyEnumMember;
            oos = OneOptionalSeq.new();
            oos(1) = oo1;
            ifsd = IntFixedStructDict.new();
            ifsd(4) = fs;
            ivsd = IntVarStructDict.new();
            ivsd(5) = vs;
            iood = IntOneOptionalDict.new();
            iood(5) = OneOptional(15);
            ioopd = IntOneOptionalPrxDict.new();
            ioopd(5) = communicator.stringToProxy('test');
            mo1 = MultiOptional(15, true, 19, 78, 99, 5.5, 1.0, 'test', MyEnum.MyEnumMember, ...
                                     communicator.stringToProxy('test'), ...
                                     [], [5], {'test', 'test2'}, iid, sid, fs, vs, [1], ...
                                     {MyEnum.MyEnumMember, MyEnum.MyEnumMember}, ...
                                     { fs }, { vs }, oos, { communicator.stringToProxy('test') }, ...
                                     ied, ifsd, ivsd, iood, ioopd, [false, true, false], []);

            assert(mo1.a == 15);
            assert(mo1.b == true);
            assert(mo1.c == 19);
            assert(mo1.d == 78);
            assert(mo1.e == 99);
            assert(mo1.f == 5.5);
            assert(mo1.g == 1.0);
            assert(strcmp(mo1.h, 'test'));
            assert(mo1.i == MyEnum.MyEnumMember);
            assert(mo1.j == communicator.stringToProxy('test'));
            assert(isempty(mo1.k));
            assert(mo1.bs == [5])
            assert(isequal(mo1.ss, {'test', 'test2'}));
            assert(mo1.iid(4) == 3);
            assert(mo1.sid('test') == 10);
            assert(isequal(mo1.fs, FixedStruct(78)));
            assert(isequal(mo1.vs, VarStruct('hello')));

            assert(mo1.shs(1) == 1);
            assert(mo1.es{1} == MyEnum.MyEnumMember && mo1.es{2} == MyEnum.MyEnumMember);
            assert(isequal(mo1.fss{1}, FixedStruct(78)));
            assert(isequal(mo1.vss{1}, VarStruct('hello')));
            assert(isequal(mo1.oos(1), oo1));
            assert(mo1.oops{1} == communicator.stringToProxy('test'));

            assert(mo1.ied(4) == MyEnum.MyEnumMember);
            assert(isequal(mo1.ifsd(4), FixedStruct(78)));
            assert(isequal(mo1.ivsd(5), VarStruct('hello')));
            assert(mo1.iood(5).a == 15);
            assert(mo1.ioopd(5) == communicator.stringToProxy('test'));

            assert(isequal(mo1.bos, [false, true, false]));

            %
            % Test generated struct and classes compare with Ice.Unset
            %
            assert(ss ~= Ice.Unset);
            assert(fs ~= Ice.Unset);
            assert(vs ~= Ice.Unset);
            assert(mo1 ~= Ice.Unset);

            fprintf('ok\n');

            fprintf('testing marshaling... ');

            oo4 = initial.pingPong(OneOptional());
            assert(oo4.a == Ice.Unset);

            oo5 = initial.pingPong(oo1);
            assert(oo1.a == oo5.a);

            mo4 = initial.pingPong(MultiOptional());
            assert(mo4.a == Ice.Unset);
            assert(mo4.b == Ice.Unset);
            assert(mo4.c == Ice.Unset);
            assert(mo4.d == Ice.Unset);
            assert(mo4.e == Ice.Unset);
            assert(mo4.f == Ice.Unset);
            assert(mo4.g == Ice.Unset);
            assert(mo4.h == Ice.Unset);
            assert(mo4.i == Ice.Unset);
            assert(mo4.j == Ice.Unset);
            assert(mo4.k == Ice.Unset);
            assert(mo4.bs == Ice.Unset);
            assert(mo4.ss == Ice.Unset);
            assert(mo4.iid == Ice.Unset);
            assert(mo4.sid == Ice.Unset);
            assert(mo4.fs == Ice.Unset);
            assert(mo4.vs == Ice.Unset);

            assert(mo4.shs == Ice.Unset);
            assert(mo4.es == Ice.Unset);
            assert(mo4.fss == Ice.Unset);
            assert(mo4.vss == Ice.Unset);
            assert(mo4.oos == Ice.Unset);
            assert(mo4.oops == Ice.Unset);

            assert(mo4.ied == Ice.Unset);
            assert(mo4.ifsd == Ice.Unset);
            assert(mo4.ivsd == Ice.Unset);
            assert(mo4.iood == Ice.Unset);
            assert(mo4.ioopd == Ice.Unset);

            assert(mo4.bos == Ice.Unset);

            mo5 = initial.pingPong(mo1);
            assert(mo5.a == mo1.a);
            assert(mo5.b == mo1.b);
            assert(mo5.c == mo1.c);
            assert(mo5.d == mo1.d);
            assert(mo5.e == mo1.e);
            assert(mo5.f == mo1.f);
            assert(mo5.g == mo1.g);
            assert(strcmp(mo5.h, mo1.h));
            assert(mo5.i == mo1.i);
            assert(mo5.j == mo1.j);
            assert(isempty(mo5.k));
            assert(mo5.bs(1) == 5);
            assert(isequal(mo5.ss, mo1.ss));
            assert(mo5.iid(4) == 3);
            assert(mo5.sid('test') == 10);
            assert(mo5.fs == mo1.fs);
            assert(mo5.vs == mo1.vs);
            assert(isequal(mo5.shs, mo1.shs));
            assert(mo5.es{1} == MyEnum.MyEnumMember && mo1.es{2} == MyEnum.MyEnumMember);
            assert(mo5.fss{1} == FixedStruct(78));
            assert(mo5.vss{1} == VarStruct('hello'));
            assert(mo5.oos(1).a == 15);
            assert(mo5.oops{1} == communicator.stringToProxy('test'));

            assert(mo5.ied(4) == MyEnum.MyEnumMember);
            assert(mo5.ifsd(4) == FixedStruct(78));
            assert(mo5.ivsd(5) == VarStruct('hello'));
            assert(mo5.iood(5).a == 15);
            assert(mo5.ioopd(5) == communicator.stringToProxy('test'));

            assert(isequal(mo5.bos, mo1.bos));

            % Clear the first half of the optional members
            mo6 = MultiOptional();
            mo6.b = mo5.b;
            mo6.d = mo5.d;
            mo6.f = mo5.f;
            mo6.h = mo5.h;
            mo6.j = mo5.j;
            mo6.bs = mo5.bs;
            mo6.iid = mo5.iid;
            mo6.fs = mo5.fs;
            mo6.shs = mo5.shs;
            mo6.fss = mo5.fss;
            mo6.oos = mo5.oos;
            mo6.ifsd = mo5.ifsd;
            mo6.iood = mo5.iood;
            mo6.bos = mo5.bos;

            mo7 = initial.pingPong(mo6);
            assert(mo7.a == Ice.Unset);
            assert(mo7.b == mo1.b);
            assert(mo7.c == Ice.Unset);
            assert(mo7.d == mo1.d);
            assert(mo7.e == Ice.Unset);
            assert(mo7.f == mo1.f);
            assert(mo7.g == Ice.Unset);
            assert(strcmp(mo7.h, mo1.h));
            assert(mo7.i == Ice.Unset);
            assert(mo7.j == mo1.j);
            assert(mo7.k == Ice.Unset);
            assert(mo7.bs(1) == 5);
            assert(mo7.ss == Ice.Unset);
            assert(mo7.iid(4) == 3);
            assert(mo7.sid == Ice.Unset);
            assert(mo7.fs == mo1.fs);
            assert(mo7.vs == Ice.Unset);

            assert(isequal(mo7.shs, mo1.shs));
            assert(mo7.es == Ice.Unset);
            assert(mo7.fss{1} == FixedStruct(78));
            assert(mo7.vss == Ice.Unset);
            assert(mo7.oos(1).a == 15);
            assert(mo7.oops == Ice.Unset);

            assert(mo7.ied == Ice.Unset);
            assert(mo7.ifsd(4) == FixedStruct(78));
            assert(mo7.ivsd == Ice.Unset);
            assert(mo7.iood(5).a == 15);
            assert(mo7.ioopd == Ice.Unset);

            assert(isequal(mo7.bos, [false, true, false]));

            % Clear the second half of the optional members
            mo8 = MultiOptional();
            mo8.a = mo5.a;
            mo8.c = mo5.c;
            mo8.e = mo5.e;
            mo8.g = mo5.g;
            mo8.i = mo5.i;
            mo8.k = mo8;
            mo8.ss = mo5.ss;
            mo8.sid = mo5.sid;
            mo8.vs = mo5.vs;

            mo8.es = mo5.es;
            mo8.vss = mo5.vss;
            mo8.oops = mo5.oops;

            mo8.ied = mo5.ied;
            mo8.ivsd = mo5.ivsd;
            mo8.ioopd = mo5.ioopd;

            mo9 = initial.pingPong(mo8);
            assert(mo9.a == mo1.a);
            assert(mo9.b == Ice.Unset);
            assert(mo9.c == mo1.c);
            assert(mo9.d == Ice.Unset);
            assert(mo9.e == mo1.e);
            assert(mo9.f == Ice.Unset);
            assert(mo9.g == mo1.g);
            assert(mo9.h == Ice.Unset);
            assert(mo9.i == mo1.i);
            assert(mo9.j == Ice.Unset);
            assert(mo9.k == mo9);
            assert(mo9.bs == Ice.Unset);
            assert(isequal(mo9.ss, mo1.ss));
            assert(mo9.iid == Ice.Unset);
            assert(mo9.sid('test') == 10);
            assert(mo9.fs == Ice.Unset);
            assert(mo9.vs == mo1.vs);

            assert(mo9.shs == Ice.Unset);
            assert(mo9.es{1} == MyEnum.MyEnumMember && mo1.es{2} == MyEnum.MyEnumMember);
            assert(mo9.fss == Ice.Unset);
            assert(mo9.vss{1} == VarStruct('hello'));
            assert(mo9.oos == Ice.Unset);
            assert(mo9.oops{1} == communicator.stringToProxy('test'));

            assert(mo9.ied(4) == MyEnum.MyEnumMember);
            assert(mo9.ifsd == Ice.Unset);
            assert(mo9.ivsd(5) == VarStruct('hello'));
            assert(mo9.iood == Ice.Unset);
            assert(mo9.ioopd(5) == communicator.stringToProxy('test'));

            assert(mo9.bos == Ice.Unset);

            %
            % Use the 1.0 encoding with operations whose only class parameters are optional.
            %
            initial.sendOptionalClass(true, OneOptional(53));
            initial.ice_encodingVersion(Ice.EncodingVersion(1, 0)).sendOptionalClass(true, OneOptional(53));

            r = initial.returnOptionalClass(true);
            assert(r ~= Ice.Unset)
            r = initial.ice_encodingVersion(Ice.EncodingVersion(1, 0)).returnOptionalClass(true);
            assert(r == Ice.Unset);

            recursive1 = RecursiveSeq.new();
            recursive2 = RecursiveSeq.new();
            r1 = Recursive();
            r2 = Recursive();
            r1.value = recursive2;
            recursive1(1) = r1;
            recursive2(1) = r2;

            outer = Recursive();
            outer.value = recursive1;
            initial.pingPong(outer);

            g = G();
            g.gg1Opt = G1('gg1Opt');
            g.gg2 = G2(10);
            g.gg2Opt = G2(20);
            g.gg1 = G1('gg1');
            r = initial.opG(g);
            assert(strcmp(r.gg1Opt.a, 'gg1Opt'));
            assert(r.gg2.a == 10);
            assert(r.gg2Opt.a == 20);
            assert(strcmp(r.gg1.a, 'gg1'));

            initial2 = Initial2Prx.uncheckedCast(base);
            initial2.opVoid(15, 'test');

            fprintf('ok\n');

            fprintf('testing marshaling of large containers with fixed size elements... ');

            mc = MultiOptional();

            mc.bs = uint8(zeros(1, 1000));
            mc.shs = int16(zeros(1, 300));

            mc.fss = cell(1, 300);
            for i = 1:300
                mc.fss{i} = FixedStruct();
            end

            mc.ifsd = IntFixedStructDict.new();
            for i = 1:300
                mc.ifsd(i) = FixedStruct();
            end

            mc = initial.pingPong(mc);
            assert(length(mc.bs) == 1000);
            assert(length(mc.shs) == 300);
            assert(length(mc.fss) == 300);
            assert(length(mc.ifsd) == 300);

            fprintf('ok\n');

            fprintf('testing tag marshaling... ');

            b = B();
            b2 = initial.pingPong(b);
            assert(b2.ma == Ice.Unset);
            assert(b2.mb == Ice.Unset);
            assert(b2.mc == Ice.Unset);

            b.ma = 10;
            b.mb = 11;
            b.mc = 12;
            b.md = 13;

            b2 = initial.pingPong(b);
            assert(b2.ma == 10);
            assert(b2.mb == 11);
            assert(b2.mc == 12);
            assert(b2.md == 13);

            fprintf('ok\n');

            fprintf('testing marshalling of objects with optional objects... ');

            f = F();

            f.af = A();
            f.ae = f.af;

            rf = initial.pingPong(f);
            assert(rf.ae == rf.af);

            fprintf('ok\n');

            fprintf('testing optional with default values... ');

            wd = initial.pingPong(WD());
            assert(wd.a == 5);
            assert(strcmp(wd.s, 'test'));
            wd.a = Ice.Unset;
            wd.s = Ice.Unset;
            wd = initial.pingPong(wd);
            assert(wd.a == Ice.Unset);
            assert(wd.s == Ice.Unset);

            fprintf('ok\n');

            if communicator.getProperties().getPropertyAsInt('Ice.Default.SlicedFormat') > 0
                fprintf('testing marshaling with unknown class slices... ');

                c = C();
                c.ss = 'test';
                c.ms = 'testms';
                c = initial.pingPong(c);
                assert(c.ma == Ice.Unset);
                assert(c.mb == Ice.Unset);
                assert(c.mc == Ice.Unset);
                assert(c.md == Ice.Unset);
                assert(strcmp(c.ss, 'test'));
                assert(strcmp(c.ms, 'testms'));

                fprintf('ok\n');

                fprintf('testing optionals with unknown classes... ');

                initial2 = Initial2Prx.uncheckedCast(base);
                d = D();
                d.ds = 'test';
                d.seq = {'test1', 'test2', 'test3', 'test4'};
                d.ao = A(18, Ice.Unset, Ice.Unset, Ice.Unset);
                d.requiredB = 14;
                d.requiredA = 14;
                initial2.opClassAndUnknownOptional(A(), d);

                fprintf('ok\n');
            end

            fprintf('testing optional parameters... ');

            [p2, p3] = initial.opByte(Ice.Unset);
            assert(p2 == Ice.Unset && p3 == Ice.Unset);
            [p2, p3] = initial.opByte(56);
            assert(p2 == 56 && p3 == 56);
            f = initial.opByteAsync(56);
            [p2, p3] = f.fetchOutputs();
            assert(p2 == 56 && p3 == 56);

            [p2, p3] = initial.opBool(Ice.Unset);
            assert(p2 == Ice.Unset && p3 == Ice.Unset);
            [p2, p3] = initial.opBool(true);
            assert(p2 == true && p3 == true);
            f = initial.opBoolAsync(true);
            [p2, p3] = f.fetchOutputs();
            assert(p2 == true && p3 == true);

            [p2, p3] = initial.opShort(Ice.Unset);
            assert(p2 == Ice.Unset && p3 == Ice.Unset);
            [p2, p3] = initial.opShort(56);
            assert(p2 == 56 && p3 == 56);
            f = initial.opShortAsync(56);
            [p2, p3] = f.fetchOutputs();
            assert(p2 == 56 && p3 == 56);

            [p2, p3] = initial.opInt(Ice.Unset);
            assert(p2 == Ice.Unset && p3 == Ice.Unset);
            [p2, p3] = initial.opInt(56);
            assert(p2 == 56 && p3 == 56);
            f = initial.opIntAsync(56);
            [p2, p3] = f.fetchOutputs();
            assert(p2 == 56 && p3 == 56);

            [p2, p3] = initial.opLong(Ice.Unset);
            assert(p2 == Ice.Unset && p3 == Ice.Unset);
            [p2, p3] = initial.opLong(56);
            assert(p2 == 56 && p3 == 56);
            f = initial.opLongAsync(56);
            [p2, p3] = f.fetchOutputs();
            assert(p2 == 56 && p3 == 56);

            [p2, p3] = initial.opFloat(Ice.Unset);
            assert(p2 == Ice.Unset && p3 == Ice.Unset);
            [p2, p3] = initial.opFloat(1.0);
            assert(p2 == 1.0 && p3 == 1.0);
            f = initial.opFloatAsync(1.0);
            [p2, p3] = f.fetchOutputs();
            assert(p2 == 1.0 && p3 == 1.0);

            [p2, p3] = initial.opDouble(Ice.Unset);
            assert(p2 == Ice.Unset && p3 == Ice.Unset);
            [p2, p3] = initial.opDouble(1.0);
            assert(p2 == 1.0 && p3 == 1.0);
            f = initial.opDoubleAsync(1.0);
            [p2, p3] = f.fetchOutputs();
            assert(p2 == 1.0 && p3 == 1.0);

            [p2, p3] = initial.opString(Ice.Unset);
            assert(p2 == Ice.Unset && p3 == Ice.Unset);
            [p2, p3] = initial.opString('test');
            assert(strcmp(p2, 'test') && strcmp(p3, 'test'));
            f = initial.opStringAsync('test');
            [p2, p3] = f.fetchOutputs();
            assert(strcmp(p2, 'test') && strcmp(p3, 'test'));

            [p2, p3] = initial.opMyEnum(Ice.Unset);
            assert(p2 == Ice.Unset && p3 == Ice.Unset);
            [p2, p3] = initial.opMyEnum(MyEnum.MyEnumMember);
            assert(p2 == MyEnum.MyEnumMember && p3 == MyEnum.MyEnumMember);
            f = initial.opMyEnumAsync(MyEnum.MyEnumMember);
            [p2, p3] = f.fetchOutputs();
            assert(p2 == MyEnum.MyEnumMember && p3 == MyEnum.MyEnumMember);

            [p2, p3] = initial.opSmallStruct(Ice.Unset);
            assert(p2 == Ice.Unset && p3 == Ice.Unset);
            p1 = SmallStruct(56);
            [p2, p3] = initial.opSmallStruct(p1);
            assert(p2 == p1 && p3 == p1);
            [p2, p3] = initial.opSmallStruct([]); % Test null struct
            assert(p2.m == 0 && p3.m == 0);
            f = initial.opSmallStructAsync(p1);
            [p2, p3] = f.fetchOutputs();
            assert(p2 == p1 && p3 == p1);

            [p2, p3] = initial.opFixedStruct(Ice.Unset);
            assert(p2 == Ice.Unset && p3 == Ice.Unset);
            p1 = FixedStruct(56);
            [p2, p3] = initial.opFixedStruct(p1);
            assert(p2 == p1 && p3 == p1);
            f = initial.opFixedStructAsync(p1);
            [p2, p3] = f.fetchOutputs();
            assert(p2 == p1 && p3 == p1);

            [p2, p3] = initial.opVarStruct(Ice.Unset);
            assert(p2 == Ice.Unset && p3 == Ice.Unset);
            p1 = VarStruct('test');
            [p2, p3] = initial.opVarStruct(p1);
            assert(p2 == p1 && p3 == p1);
            f = initial.opVarStructAsync(p1);
            [p2, p3] = f.fetchOutputs();
            assert(p2 == p1 && p3 == p1);

            [p2, p3] = initial.opOneOptional(Ice.Unset);
            assert(p2 == Ice.Unset && p3 == Ice.Unset);
            if initial.supportsNullOptional()
                [p2, p3] = initial.opOneOptional([]);
                assert(isempty(p2) && isempty(p3));
            end
            p1 = OneOptional(58);
            [p2, p3] = initial.opOneOptional(p1);
            assert(p2.a == p1.a && p3.a == p1.a);
            f = initial.opOneOptionalAsync(p1);
            [p2, p3] = f.fetchOutputs();
            assert(p2.a == p1.a && p3.a == p1.a);

            [p2, p3] = initial.opOneOptionalProxy(Ice.Unset);
            assert(p2 == Ice.Unset && p3 == Ice.Unset);
            p1 = communicator.stringToProxy('test');
            [p2, p3] = initial.opOneOptionalProxy(p1);
            assert(p2 == p1 && p3 == p1);
            f = initial.opOneOptionalProxyAsync(p1);
            [p2, p3] = f.fetchOutputs();
            assert(p2 == p1 && p3 == p1);

            [p2, p3] = initial.opByteSeq(Ice.Unset);
            assert(p2 == Ice.Unset && p3 == Ice.Unset);
            p1 = [];
            p1(1:100) = 56;
            [p2, p3] = initial.opByteSeq(p1);
            assert(length(p2) == length(p1) && length(p3) == length(p1));
            assert(p2(1) == 56);
            assert(p3(1) == 56);
            f = initial.opByteSeqAsync(p1);
            [p2, p3] = f.fetchOutputs();
            assert(length(p2) == length(p1) && length(p3) == length(p1));
            assert(p2(1) == 56);
            assert(p3(1) == 56);

            [p2, p3] = initial.opBoolSeq(Ice.Unset);
            assert(p2 == Ice.Unset && p3 == Ice.Unset);
            p1 = [];
            p1(1:100) = true;
            [p2, p3] = initial.opBoolSeq(p1);
            assert(isequal(p2, p1) && isequal(p3, p1));
            f = initial.opBoolSeqAsync(p1);
            [p2, p3] = f.fetchOutputs();
            assert(isequal(p2, p1) && isequal(p3, p1));

            [p2, p3] = initial.opShortSeq(Ice.Unset);
            assert(p2 == Ice.Unset && p3 == Ice.Unset);
            p1 = [];
            p1(1:100) = 56;
            [p2, p3] = initial.opShortSeq(p1);
            assert(isequal(p2, p1) && isequal(p3, p1));
            f = initial.opShortSeqAsync(p1);
            [p2, p3] = f.fetchOutputs();
            assert(isequal(p2, p1) && isequal(p3, p1));

            [p2, p3] = initial.opIntSeq(Ice.Unset);
            assert(p2 == Ice.Unset && p3 == Ice.Unset);
            p1 = [];
            p1(1:100) = 56;
            [p2, p3] = initial.opIntSeq(p1);
            assert(isequal(p2, p1) && isequal(p3, p1));
            f = initial.opIntSeqAsync(p1);
            [p2, p3] = f.fetchOutputs();
            assert(isequal(p2, p1) && isequal(p3, p1));

            [p2, p3] = initial.opLongSeq(Ice.Unset);
            assert(p2 == Ice.Unset && p3 == Ice.Unset);
            p1 = [];
            p1(1:100) = 56;
            [p2, p3] = initial.opLongSeq(p1);
            assert(isequal(p2, p1) && isequal(p3, p1));
            f = initial.opLongSeqAsync(p1);
            [p2, p3] = f.fetchOutputs();
            assert(isequal(p2, p1) && isequal(p3, p1));

            [p2, p3] = initial.opFloatSeq(Ice.Unset);
            assert(p2 == Ice.Unset && p3 == Ice.Unset);
            p1 = [];
            p1(1:100) = 1.0;
            [p2, p3] = initial.opFloatSeq(p1);
            assert(isequal(p2, p1) && isequal(p3, p1));
            f = initial.opFloatSeqAsync(p1);
            [p2, p3] = f.fetchOutputs();
            assert(isequal(p2, p1) && isequal(p3, p1));

            [p2, p3] = initial.opDoubleSeq(Ice.Unset);
            assert(p2 == Ice.Unset && p3 == Ice.Unset);
            p1 = [];
            p1(1:100) = 1.0;
            [p2, p3] = initial.opDoubleSeq(p1);
            assert(isequal(p2, p1) && isequal(p3, p1));
            f = initial.opDoubleSeqAsync(p1);
            [p2, p3] = f.fetchOutputs();
            assert(isequal(p2, p1) && isequal(p3, p1));

            [p2, p3] = initial.opStringSeq(Ice.Unset);
            assert(p2 == Ice.Unset && p3 == Ice.Unset);
            p1 = cell(1, 100);
            for i = 1:length(p1)
                p1{i} = 'test';
            end
            [p2, p3] = initial.opStringSeq(p1);
            assert(isequal(p2, p1) && isequal(p3, p1));
            f = initial.opStringSeqAsync(p1);
            [p2, p3] = f.fetchOutputs();
            assert(isequal(p2, p1) && isequal(p3, p1));

            [p2, p3] = initial.opSmallStructSeq(Ice.Unset);
            assert(p2 == Ice.Unset && p3 == Ice.Unset);
            p1 = cell(1, 100);
            for i = 1:length(p1)
                p1{i} = SmallStruct(1);
            end
            [p2, p3] = initial.opSmallStructSeq(p1);
            assert(isequal(p2, p1) && isequal(p3, p1));
            f = initial.opSmallStructSeqAsync(p1);
            [p2, p3] = f.fetchOutputs();
            assert(isequal(p2, p1) && isequal(p3, p1));

            [p2, p3] = initial.opSmallStructList(Ice.Unset);
            assert(p2 == Ice.Unset && p3 == Ice.Unset);
            p1 = cell(1, 100);
            for i = 1:length(p1)
                p1{i} = SmallStruct(1);
            end
            [p2, p3] = initial.opSmallStructList(p1);
            assert(isequal(p2, p1) && isequal(p3, p1));
            f = initial.opSmallStructListAsync(p1);
            [p2, p3] = f.fetchOutputs();
            assert(isequal(p2, p1) && isequal(p3, p1));

            [p2, p3] = initial.opFixedStructSeq(Ice.Unset);
            assert(p2 == Ice.Unset && p3 == Ice.Unset);
            p1 = cell(1, 100);
            for i = 1:length(p1)
                p1{i} = FixedStruct(1);
            end
            [p2, p3] = initial.opFixedStructSeq(p1);
            assert(isequal(p2, p1) && isequal(p3, p1));
            f = initial.opFixedStructSeqAsync(p1);
            [p2, p3] = f.fetchOutputs();
            assert(isequal(p2, p1) && isequal(p3, p1));

            [p2, p3] = initial.opFixedStructList(Ice.Unset);
            assert(p2 == Ice.Unset && p3 == Ice.Unset);
            p1 = cell(1, 100);
            for i = 1:length(p1)
                p1{i} = FixedStruct(1);
            end
            [p2, p3] = initial.opFixedStructList(p1);
            assert(isequal(p2, p1) && isequal(p3, p1));
            f = initial.opFixedStructListAsync(p1);
            [p2, p3] = f.fetchOutputs();
            assert(isequal(p2, p1) && isequal(p3, p1));

            [p2, p3] = initial.opVarStructSeq(Ice.Unset);
            assert(p2 == Ice.Unset && p3 == Ice.Unset);
            p1 = cell(1, 100);
            for i = 1:length(p1)
                p1{i} = VarStruct('test');
            end
            [p2, p3] = initial.opVarStructSeq(p1);
            assert(isequal(p2, p1) && isequal(p3, p1));
            f = initial.opVarStructSeqAsync(p1);
            [p2, p3] = f.fetchOutputs();
            assert(isequal(p2, p1) && isequal(p3, p1));

            [p2, p3] = initial.opIntIntDict(Ice.Unset);
            assert(p2 == Ice.Unset && p3 == Ice.Unset);
            p1 = IntIntDict.new();
            p1(1) = 2;
            p1(2) = 3;
            [p2, p3] = initial.opIntIntDict(p1);
            assert(isequal(p2, p1) && isequal(p3, p1));
            f = initial.opIntIntDictAsync(p1);
            [p2, p3] = f.fetchOutputs();
            assert(isequal(p2, p1) && isequal(p3, p1));

            [p2, p3] = initial.opStringIntDict(Ice.Unset);
            assert(p2 == Ice.Unset && p3 == Ice.Unset);
            p1 = StringIntDict.new();
            p1('1') = 2;
            p1('2') = 3;
            [p2, p3] = initial.opStringIntDict(p1);
            assert(isequal(p2, p1) && isequal(p3, p1));
            f = initial.opStringIntDictAsync(p1);
            [p2, p3] = f.fetchOutputs();
            assert(isequal(p2, p1) && isequal(p3, p1));

            [p2, p3] = initial.opIntOneOptionalDict(Ice.Unset);
            assert(p2 == Ice.Unset && p3 == Ice.Unset);
            p1 = IntOneOptionalDict.new();
            p1(1) = OneOptional(58);
            p1(2) = OneOptional(59);
            [p2, p3] = initial.opIntOneOptionalDict(p1);
            assert(p2(1).a == 58 && p3(1).a == 58);
            f = initial.opIntOneOptionalDictAsync(p1);
            [p2, p3] = f.fetchOutputs();
            assert(p2(1).a == 58 && p3(1).a == 58);

            fprintf('ok\n');

            fprintf('testing exception optionals... ');

            try
                initial.opOptionalException(Ice.Unset, Ice.Unset, Ice.Unset);
            catch ex
                assert(isa(ex, 'test.Ice.optional.Test.OptionalException'));
                assert(ex.a == Ice.Unset);
                assert(ex.b == Ice.Unset);
                assert(ex.o == Ice.Unset);
            end

            try
                initial.opOptionalException(30, 'test', OneOptional(53));
            catch ex
                assert(isa(ex, 'test.Ice.optional.Test.OptionalException'));
                assert(ex.a == 30);
                assert(strcmp(ex.b, 'test'));
                assert(ex.o.a == 53);
            end

            try
                %
                % Use the 1.0 encoding with an exception whose only class members are optional.
                %
                initial.ice_encodingVersion(Ice.EncodingVersion(1, 0)).opOptionalException(30, 'test', OneOptional(53));
            catch ex
                assert(isa(ex, 'test.Ice.optional.Test.OptionalException'));
                assert(ex.a == Ice.Unset);
                assert(ex.b == Ice.Unset);
                assert(ex.o == Ice.Unset);
            end

            try
                initial.opDerivedException(Ice.Unset, Ice.Unset, Ice.Unset);
            catch ex
                assert(isa(ex, 'test.Ice.optional.Test.DerivedException'));
                assert(ex.a == Ice.Unset);
                assert(ex.b == Ice.Unset);
                assert(ex.o == Ice.Unset);
                assert(ex.ss == Ice.Unset);
                assert(ex.o2 == Ice.Unset);
            end

            try
                initial.opDerivedException(30, 'test2', OneOptional(53));
            catch ex
                assert(isa(ex, 'test.Ice.optional.Test.DerivedException'));
                assert(ex.a == 30);
                assert(strcmp(ex.b, 'test2'));
                assert(ex.o.a == 53);
                assert(strcmp(ex.ss, 'test2'));
                assert(ex.o2 == ex.o);
            end

            try
                initial.opRequiredException(Ice.Unset, Ice.Unset, Ice.Unset);
            catch ex
                assert(isa(ex, 'test.Ice.optional.Test.RequiredException'));
                assert(ex.a == Ice.Unset);
                assert(ex.b == Ice.Unset);
                assert(ex.o == Ice.Unset);
                assert(strcmp(ex.ss, 'test'));
                assert(isempty(ex.o2));
            end

            try
                initial.opRequiredException(30, 'test2', OneOptional(53));
            catch ex
                assert(isa(ex, 'test.Ice.optional.Test.RequiredException'));
                assert(ex.a == 30);
                assert(strcmp(ex.b, 'test2'));
                assert(ex.o.a == 53);
                assert(strcmp(ex.ss, 'test2'));
                assert(ex.o2 == ex.o);
            end

            fprintf('ok\n');

            r = initial;
        end
    end
end
