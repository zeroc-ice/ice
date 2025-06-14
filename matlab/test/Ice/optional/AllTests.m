% Copyright (c) ZeroC, Inc.

classdef AllTests
    methods(Static)
        function r = allTests(helper)
            import Test.*;

            communicator = helper.communicator();

            ref = ['initial:', helper.getTestEndpoint()];
            initial = InitialPrx(communicator, ref);

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
            assert(isempty(mo1.j));
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
            assert(mo1.mips == Ice.Unset);

            assert(mo1.ied == Ice.Unset);
            assert(mo1.ifsd == Ice.Unset);
            assert(mo1.ivsd == Ice.Unset);
            assert(mo1.imipd == Ice.Unset);

            assert(mo1.bos == Ice.Unset);

            ss = SmallStruct();
            fs = FixedStruct(78);
            vs = VarStruct('hello');
            iid = configureDictionary('int32', 'int32');
            iid(4) = 3;
            sid = configureDictionary('char', 'int32');
            sid('test') = 10;
            ied = configureDictionary('int32', 'Test.MyEnum');
            ied(4) = MyEnum.MyEnumMember;
            ifsd = configureDictionary('int32', 'Test.FixedStruct');
            ifsd(4) = fs;
            ivsd = configureDictionary('int32', 'Test.VarStruct');
            ivsd(5) = vs;
            imipd = configureDictionary('int32', 'cell');
            imipd{5} = MyInterfacePrx(communicator, 'test');
            mo1 = MultiOptional(15, true, 19, 78, 99, 5.5, 1.0, 'test', MyEnum.MyEnumMember, ...
                                     MyInterfacePrx(communicator, 'test'), ...
                                     [5], {'test', 'test2'}, iid, sid, fs, vs, [1], ...
                                     [MyEnum.MyEnumMember, MyEnum.MyEnumMember], ...
                                     [ fs ], [ vs ], { MyInterfacePrx(communicator, 'test') }, ...
                                     ied, ifsd, ivsd, imipd, [false, true, false], []);

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
            assert(mo1.bs == [5])
            assert(isequal(mo1.ss, {'test', 'test2'}));
            assert(mo1.iid(4) == 3);
            assert(mo1.sid('test') == 10);
            assert(isequal(mo1.fs, FixedStruct(78)));
            assert(isequal(mo1.vs, VarStruct('hello')));

            assert(mo1.shs(1) == 1);
            assert(mo1.es(1) == MyEnum.MyEnumMember && mo1.es(2) == MyEnum.MyEnumMember);
            assert(isequal(mo1.fss(1), FixedStruct(78)));
            assert(isequal(mo1.vss(1), VarStruct('hello')));
            assert(mo1.mips{1} == communicator.stringToProxy('test'));

            assert(mo1.ied(4) == MyEnum.MyEnumMember);
            assert(isequal(mo1.ifsd(4), FixedStruct(78)));
            assert(isequal(mo1.ivsd(5), VarStruct('hello')));
            assert(mo1.imipd{5} == communicator.stringToProxy('test'));

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
            assert(isempty(mo4.j)); % we don't use Unset for optional proxies
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
            assert(mo4.mips == Ice.Unset);

            assert(mo4.ied == Ice.Unset);
            assert(mo4.ifsd == Ice.Unset);
            assert(mo4.ivsd == Ice.Unset);
            assert(mo4.imipd == Ice.Unset);

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
            assert(mo5.bs(1) == 5);
            assert(isequal(mo5.ss, mo1.ss));
            assert(mo5.iid(4) == 3);
            assert(mo5.sid('test') == 10);
            assert(mo5.fs == mo1.fs);
            assert(mo5.vs == mo1.vs);
            assert(isequal(mo5.shs, mo1.shs));
            assert(mo5.es(1) == MyEnum.MyEnumMember && mo1.es(2) == MyEnum.MyEnumMember);
            assert(mo5.fss(1) == FixedStruct(78));
            assert(mo5.vss(1) == VarStruct('hello'));
            assert(mo5.mips{1} == communicator.stringToProxy('test'));

            assert(mo5.ied(4) == MyEnum.MyEnumMember);
            assert(mo5.ifsd(4) == FixedStruct(78));
            assert(mo5.ivsd(5) == VarStruct('hello'));
            assert(mo5.imipd{5} == communicator.stringToProxy('test'));

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
            mo6.ifsd = mo5.ifsd;
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
            assert(mo7.bs(1) == 5);
            assert(mo7.ss == Ice.Unset);
            assert(mo7.iid(4) == 3);
            assert(mo7.sid == Ice.Unset);
            assert(mo7.fs == mo1.fs);
            assert(mo7.vs == Ice.Unset);

            assert(isequal(mo7.shs, mo1.shs));
            assert(mo7.es == Ice.Unset);
            assert(mo7.fss(1) == FixedStruct(78));
            assert(mo7.vss == Ice.Unset);
            assert(mo7.mips == Ice.Unset);

            assert(mo7.ied == Ice.Unset);
            assert(mo7.ifsd(4) == FixedStruct(78));
            assert(mo7.ivsd == Ice.Unset);
            assert(mo7.imipd == Ice.Unset);

            assert(isequal(mo7.bos, [false, true, false]));

            % Clear the second half of the optional members
            mo8 = MultiOptional();
            mo8.a = mo5.a;
            mo8.c = mo5.c;
            mo8.e = mo5.e;
            mo8.g = mo5.g;
            mo8.i = mo5.i;
            mo8.ss = mo5.ss;
            mo8.sid = mo5.sid;
            mo8.vs = mo5.vs;

            mo8.es = mo5.es;
            mo8.vss = mo5.vss;
            mo8.mips = mo5.mips;

            mo8.ied = mo5.ied;
            mo8.ivsd = mo5.ivsd;
            mo8.imipd = mo5.imipd;

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
            assert(isempty(mo9.j)); % optional proxy
            assert(mo9.bs == Ice.Unset);
            assert(isequal(mo9.ss, mo1.ss));
            assert(mo9.iid == Ice.Unset);
            assert(mo9.sid('test') == 10);
            assert(isempty(mo9.j)); % optional proxy
            assert(mo9.vs == mo1.vs);

            assert(mo9.shs == Ice.Unset);
            assert(mo9.es(1) == MyEnum.MyEnumMember && mo1.es(2) == MyEnum.MyEnumMember);
            assert(mo9.fss == Ice.Unset);
            assert(mo9.vss(1) == VarStruct('hello'));
            assert(mo9.mips{1} == communicator.stringToProxy('test'));

            assert(mo9.ied(4) == MyEnum.MyEnumMember);
            assert(mo9.ifsd == Ice.Unset);
            assert(mo9.ivsd(5) == VarStruct('hello'));
            assert(mo9.imipd{5} == communicator.stringToProxy('test'));

            assert(mo9.bos == Ice.Unset);

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

            initial2 = Initial2Prx.uncheckedCast(initial);
            initial2.opVoid(15, 'test');

            fprintf('ok\n');

            fprintf('testing marshaling of large containers with fixed size elements... ');

            mc = MultiOptional();

            mc.bs = uint8(zeros(1, 1000));
            mc.shs = int16(zeros(1, 300));

            mc.fss = FixedStruct();
            for i = 1:300
                mc.fss(i) = FixedStruct();
            end

            mc.ifsd = configureDictionary('int32', 'Test.FixedStruct');
            for i = 1:300
                mc.ifsd(i) = FixedStruct();
            end

            mc = initial.pingPong(mc);
            assert(length(mc.bs) == 1000);
            assert(length(mc.shs) == 300);
            assert(length(mc.fss) == 300);
            assert(mc.ifsd.numEntries == 300);

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

            fprintf('testing marshaling of objects with optional members... ');

            f = F();

            f.fsf = FixedStruct();
            f.fse = f.fsf;

            rf = initial.pingPong(f);
            assert(rf.fse == rf.fsf);

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

                fprintf('testing operations with unknown optionals... ');

                initial2 = Initial2Prx.uncheckedCast(initial);
                ovs = VarStruct('test');
                initial2.opClassAndUnknownOptional(A(), ovs);

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

            p1 = OneOptional();
            [p2, p3] = initial.opOneOptional(p1);
            assert(p2.a == Ice.Unset && p3.a == Ice.Unset);
            p1 = OneOptional(58);
            [p2, p3] = initial.opOneOptional(p1);
            assert(p2.a == p1.a && p3.a == p1.a);
            f = initial.opOneOptionalAsync(p1);
            [p2, p3] = f.fetchOutputs();
            assert(p2.a == p1.a && p3.a == p1.a);

            [p2, p3] = initial.opMyInterfaceProxy([]);
            assert(isempty(p2) && isempty(p3));
            p1 = MyInterfacePrx(communicator, 'test');
            [p2, p3] = initial.opMyInterfaceProxy(p1);
            assert(p2 == p1 && p3 == p1);
            f = initial.opMyInterfaceProxyAsync(p1);
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
            clear p1;
            p1(1, 100) = SmallStruct();
            for i = 1:length(p1)
                p1(i) = SmallStruct(1);
            end
            [p2, p3] = initial.opSmallStructSeq(p1);
            assert(isequal(p2, p1) && isequal(p3, p1));
            f = initial.opSmallStructSeqAsync(p1);
            [p2, p3] = f.fetchOutputs();
            assert(isequal(p2, p1) && isequal(p3, p1));

            [p2, p3] = initial.opSmallStructList(Ice.Unset);
            assert(p2 == Ice.Unset && p3 == Ice.Unset);
            clear p1;
            p1(1, 100) = SmallStruct();
            for i = 1:length(p1)
                p1(i) = SmallStruct(1);
            end
            [p2, p3] = initial.opSmallStructList(p1);
            assert(isequal(p2, p1) && isequal(p3, p1));
            f = initial.opSmallStructListAsync(p1);
            [p2, p3] = f.fetchOutputs();
            assert(isequal(p2, p1) && isequal(p3, p1));

            [p2, p3] = initial.opFixedStructSeq(Ice.Unset);
            assert(p2 == Ice.Unset && p3 == Ice.Unset);
            clear p1;
            p1(1, 100) = FixedStruct();
            for i = 1:length(p1)
                p1(i) = FixedStruct(1);
            end
            [p2, p3] = initial.opFixedStructSeq(p1);
            assert(isequal(p2, p1) && isequal(p3, p1));
            f = initial.opFixedStructSeqAsync(p1);
            [p2, p3] = f.fetchOutputs();
            assert(isequal(p2, p1) && isequal(p3, p1));

            [p2, p3] = initial.opFixedStructList(Ice.Unset);
            assert(p2 == Ice.Unset && p3 == Ice.Unset);
            clear p1;
            p1(1, 100) = FixedStruct();
            for i = 1:length(p1)
                p1(i) = FixedStruct(1);
            end
            [p2, p3] = initial.opFixedStructList(p1);
            assert(isequal(p2, p1) && isequal(p3, p1));
            f = initial.opFixedStructListAsync(p1);
            [p2, p3] = f.fetchOutputs();
            assert(isequal(p2, p1) && isequal(p3, p1));

            [p2, p3] = initial.opVarStructSeq(Ice.Unset);
            assert(p2 == Ice.Unset && p3 == Ice.Unset);
            clear p1;
            p1(1, 100) = VarStruct();
            for i = 1:length(p1)
                p1(i) = VarStruct('test');
            end
            [p2, p3] = initial.opVarStructSeq(p1);
            assert(isequal(p2, p1) && isequal(p3, p1));
            f = initial.opVarStructSeqAsync(p1);
            [p2, p3] = f.fetchOutputs();
            assert(isequal(p2, p1) && isequal(p3, p1));

            [p2, p3] = initial.opIntIntDict(Ice.Unset);
            assert(p2 == Ice.Unset && p3 == Ice.Unset);
            p1 = configureDictionary('int32', 'int32');
            p1(1) = 2;
            p1(2) = 3;
            [p2, p3] = initial.opIntIntDict(p1);
            assert(isequal(p2, p1) && isequal(p3, p1));
            f = initial.opIntIntDictAsync(p1);
            [p2, p3] = f.fetchOutputs();
            assert(isequal(p2, p1) && isequal(p3, p1));

            [p2, p3] = initial.opStringIntDict(Ice.Unset);
            assert(p2 == Ice.Unset && p3 == Ice.Unset);
            p1 = configureDictionary('char', 'int32');
            p1('1') = 2;
            p1('2') = 3;
            [p2, p3] = initial.opStringIntDict(p1);
            assert(isequal(p2, p1) && isequal(p3, p1));
            f = initial.opStringIntDictAsync(p1);
            [p2, p3] = f.fetchOutputs();
            assert(isequal(p2, p1) && isequal(p3, p1));

            fprintf('ok\n');

            fprintf('testing exception optionals... ');

            try
                initial.opOptionalException(Ice.Unset, Ice.Unset);
            catch ex
                assert(isa(ex, 'Test.OptionalException'));
                assert(ex.a == Ice.Unset);
                assert(ex.b == Ice.Unset);
            end

            try
                initial.opOptionalException(30, 'test');
            catch ex
                assert(isa(ex, 'Test.OptionalException'));
                assert(ex.a == 30);
                assert(strcmp(ex.b, 'test'));
            end

            try
                %
                % Use the 1.0 encoding with an exception whose only data members are optional.
                %
                initial.ice_encodingVersion(Ice.EncodingVersion(1, 0)).opOptionalException(30, 'test');
            catch ex
                assert(isa(ex, 'Test.OptionalException'));
                assert(ex.a == Ice.Unset);
                assert(ex.b == Ice.Unset);
            end

            try
                initial.opDerivedException(Ice.Unset, Ice.Unset);
            catch ex
                assert(isa(ex, 'Test.DerivedException'));
                assert(ex.a == Ice.Unset);
                assert(ex.b == Ice.Unset);
                assert(ex.ss == Ice.Unset);
                assert(strcmp(ex.d1, 'd1'));
                assert(strcmp(ex.d2, 'd2'));
            end

            try
                initial.opDerivedException(30, 'test2');
            catch ex
                assert(isa(ex, 'Test.DerivedException'));
                assert(ex.a == 30);
                assert(strcmp(ex.b, 'test2'));
                assert(strcmp(ex.ss, 'test2'));
                assert(strcmp(ex.d1, 'd1'));
                assert(strcmp(ex.d2, 'd2'));
            end

            try
                initial.opRequiredException(Ice.Unset, Ice.Unset);
            catch ex
                assert(isa(ex, 'Test.RequiredException'));
                assert(ex.a == Ice.Unset);
                assert(ex.b == Ice.Unset);
                assert(strcmp(ex.ss, 'test'));
            end

            try
                initial.opRequiredException(30, 'test2');
            catch ex
                assert(isa(ex, 'Test.RequiredException'));
                assert(ex.a == 30);
                assert(strcmp(ex.b, 'test2'));
                assert(strcmp(ex.ss, 'test2'));
            end

            fprintf('ok\n');

            r = initial;
        end
    end
end
