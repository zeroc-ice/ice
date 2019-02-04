%
% Copyright (c) ZeroC, Inc. All rights reserved.
%

classdef AllTests
    methods(Static)
        function r = allTests(helper)
            import Test.*;

            communicator = helper.communicator();
            ref = ['initial:', helper.getTestEndpoint()];
            base = communicator.stringToProxy(ref);
            initial = InitialPrx.checkedCast(base);

            fprintf('getting B1... ');
            b1 = initial.getB1();
            assert(~isempty(b1));
            fprintf('ok\n');

            fprintf('getting B2... ');
            b2 = initial.getB2();
            assert(~isempty(b2));
            fprintf('ok\n');

            fprintf('getting C... ');
            c = initial.getC();
            assert(~isempty(c));
            fprintf('ok\n');

            fprintf('getting D... ');
            d = initial.getD();
            assert(~isempty(d));
            fprintf('ok\n');

            fprintf('checking consistency... ');
            assert(b1 ~= b2);
            assert(b1.theB == b1);
            assert(isempty(b1.theC));
            assert(isa(b1.theA, 'Test.B'));
            assert(b1.theA.theA == b1.theA);
            assert(b1.theA.theB == b1);
            assert(isa(b1.theA.theC, 'Test.C'));
            assert(b1.theA.theC.theB == b1.theA);
            assert(b1.preMarshalInvoked);
            assert(b1.postUnmarshalInvoked);
            assert(b1.theA.preMarshalInvoked);
            assert(b1.theA.postUnmarshalInvoked);
            assert(b1.theA.theC.preMarshalInvoked);
            assert(b1.theA.theC.postUnmarshalInvoked);

            % More tests possible for b2 and d, but I think this is already sufficient.
            assert(b2.theA == b2);
            assert(isempty(d.theC));
            fprintf('ok\n');

            fprintf('getting B1, B2, C, and D all at once... ');
            [b1, b2, c, d] = initial.getAll();
            assert(~isempty(b1));
            assert(~isempty(b2));
            assert(~isempty(c));
            assert(~isempty(d));
            fprintf('ok\n');

            fprintf('checking consistency... ');
            assert(b1 ~= b2);
            assert(b1.theA == b2);
            assert(b1.theB == b1);
            assert(isempty(b1.theC));
            assert(b2.theA == b2);
            assert(b2.theB == b1);
            assert(b2.theC == c);
            assert(c.theB == b2);
            assert(d.theA == b1);
            assert(d.theB == b2);
            assert(isempty(d.theC));
            assert(d.preMarshalInvoked);
            assert(d.postUnmarshalInvoked);
            assert(d.theA.preMarshalInvoked);
            assert(d.theA.postUnmarshalInvoked);
            assert(d.theB.preMarshalInvoked);
            assert(d.theB.postUnmarshalInvoked);
            assert(d.theB.theC.preMarshalInvoked);
            assert(d.theB.theC.postUnmarshalInvoked);
            fprintf('ok\n');

            fprintf('testing protected members... ');
            e = initial.getE();
            assert(e.checkValues());
            em = ?Test.E;
            assert(strcmp(em.PropertyList(1).GetAccess, 'protected'));
            assert(strcmp(em.PropertyList(2).GetAccess, 'protected'));
            f = initial.getF();
            assert(f.checkValues());
            assert(f.e2.checkValues());
            fm = ?Test.F;
            assert(strcmp(fm.PropertyList(1).GetAccess, 'public'));
            assert(strcmp(fm.PropertyList(2).GetAccess, 'protected'));
            fprintf('ok\n');

            fprintf('getting I, J and H... ');
            i = initial.getI();
            assert(~isempty(i) && strcmp(i.ice_id(), IPrx.ice_staticId()));
            j = initial.getJ();
            assert(~isempty(j) && strcmp(j.ice_id(), JPrx.ice_staticId()));
            h = initial.getH();
            assert(~isempty(h) && isa(h, 'Test.H'));
            fprintf('ok\n');

            fprintf('getting K... ');
            k = initial.getK();
            assert(~isempty(k));
            assert(isa(k.value, 'Test.L'));
            assert(strcmp(k.value.data, 'l'));
            fprintf('ok\n');

            fprintf('testing Value as parameter... ');
            [v2, v3] = initial.opValue(L('l'));
            assert(strcmp(v2.data, 'l'));
            assert(strcmp(v3.data, 'l'));
            [v2, v3] = initial.opValueSeq({L('l')});
            assert(strcmp(v2{1}.data, 'l'));
            assert(strcmp(v3{1}.data, 'l'));
            d = containers.Map('KeyType', 'char', 'ValueType', 'any');
            d('l') = L('l');
            [v2, v3] = initial.opValueMap(d);
            assert(strcmp(v2('l').data, 'l'));
            assert(strcmp(v3('l').data, 'l'));
            fprintf('ok\n');

            fprintf('getting D1... ');
            d1 = D1(A1('a1'), A1('a2'), A1('a3'), A1('a4'));
            d1 = initial.getD1(d1);
            assert(strcmp(d1.a1.name, 'a1'));
            assert(strcmp(d1.a2.name, 'a2'));
            assert(strcmp(d1.a3.name, 'a3'));
            assert(strcmp(d1.a4.name, 'a4'));
            fprintf('ok\n');

            fprintf('throw EDerived... ');
            try
                initial.throwEDerived();
                assert(false);
            catch ederived
                assert(isa(ederived, 'Test.EDerived'));
                assert(strcmp(ederived.a1.name, 'a1'));
                assert(strcmp(ederived.a2.name, 'a2'));
                assert(strcmp(ederived.a3.name, 'a3'));
                assert(strcmp(ederived.a4.name, 'a4'));
            end
            fprintf('ok\n');

            fprintf('setting G... ');
            try
                initial.setG(G(S('hello'), 'g'));
            catch ex
                if ~isa(ex, 'Ice.OperationNotExistException')
                    rethrow(ex);
                end
            end
            fprintf('ok\n');

            fprintf('setting I... ');
            initial.setI(i);
            initial.setI(j);
            initial.setI(h);
            fprintf('ok\n');

            fprintf('testing sequences... ');
            try
                inS = {};
                [r, outS] = initial.opBaseSeq(inS);
                assert(length(r) == 0 && length(outS) == 0);

                %
                % The MATLAB version of this test is a little more thorough.
                %
                inS = cell(1, 20);
                for i = 1:length(inS)
                    inS{i} = Base(S(num2str(i)), num2str(i));
                end
                [r, outS] = initial.opBaseSeq(inS);
                assert(length(r) == length(inS) && length(outS) == length(inS));
                assert(isa(r, 'cell') && isa(outS, 'cell'));
                for i = 1:length(inS)
                    assert(r{i} == outS{i});
                    s = num2str(i);
                    assert(strcmp(r{i}.str, s));
                    assert(strcmp(r{i}.theS.str, s));
                end
            catch ex
                if ~isa(ex, 'Ice.OperationNotExistException')
                    rethrow(ex);
                end
            end
            fprintf('ok\n');

            fprintf('testing recursive type... ');
            top = Recursive();
            p = top;
            depth = 0;
            try
                for depth = 0:20000
                    p.v = Recursive();
                    p = p.v;
                    if (depth < 10 && mod(depth, 10) == 0) || ...
                       (depth < 1000 && mod(depth, 100) == 0) || ...
                       (depth < 10000 && mod(depth, 1000) == 0) || ...
                       mod(depth, 10000) == 0
                        initial.setRecursive(top);
                    end
                end
                assert(~initial.supportsClassGraphDepthMax());
            catch ex
                if isa(ex, 'Ice.UnknownLocalException')
                    % Expected marshal exception from the server (max class graph depth reached)
                elseif isa(ex, 'Ice.UnknownException')
                    % Expected stack overflow from the server (Java only)
                else
                    rethrow(ex);
                end
            end
            initial.setRecursive(Recursive());
            fprintf('ok\n');

            fprintf('testing compact ID... ');
            try
                assert(~isempty(initial.getCompact()));
            catch ex
                if ~isa(ex, 'Ice.OperationNotExistException')
                    rethrow(ex);
                end
            end
            fprintf('ok\n');

            fprintf('testing UnexpectedObjectException... ');
            ref = ['uoet:', helper.getTestEndpoint()];
            base = communicator.stringToProxy(ref);
            assert(~isempty(base));
            uoet = UnexpectedObjectExceptionTestPrx.uncheckedCast(base);
            assert(~isempty(uoet));
            try
                uoet.op();
                assert(false);
            catch ex
                if isa(ex, 'Ice.UnexpectedObjectException')
                    assert(strcmp(ex.type_, 'Test.AlsoEmpty'));
                    assert(strcmp(ex.expectedType, 'Test.Empty'));
                else
                    rethrow(ex);
                end
            end
            fprintf('ok\n');

            fprintf('testing class containing complex dictionary... ');
            m = M();
            m.v(1).key = StructKey(1, '1');
            m.v(1).value = L('one');

            m.v(2).key = StructKey(2, '2');
            m.v(2).value = L('two');

            [m1, m2] = initial.opM(m);

            assert(length(m1.v) == 2);
            assert(length(m2.v) == 2);

            assert(strcmp(m1.v(1).value.data, 'one'));
            assert(strcmp(m1.v(2).value.data, 'two'));

            assert(strcmp(m2.v(1).value.data, 'one'));
            assert(strcmp(m2.v(2).value.data, 'two'));

            fprintf('ok\n');
            r = initial;
        end
    end
end
