% Copyright (c) ZeroC, Inc.

classdef AllTests
    methods(Static)
        function r = allTests(helper, customSliceLoader)
            import Test.*;

            communicator = helper.communicator();

            fprintf('testing stringToProxy... ');
            ref = ['Test:', helper.getTestEndpoint(),' -t 10000'];
            base = communicator.stringToProxy(ref);
            assert(~isempty(base));
            fprintf('ok\n');

            fprintf('testing checked cast... ');
            proxy = TestIntfPrx.checkedCast(base);
            assert(~isempty(proxy));
            assert(proxy == base);
            fprintf('ok\n');

            fprintf('base as Object... ');

            sb = proxy.SBaseAsObject();
            assert(~isempty(sb));
            assert(strcmp(sb.ice_id(), '::Test::SBase'));
            assert(strcmp(sb.sb, 'SBase.sb'));

            fprintf('ok\n');

            fprintf('base as Object (AMI)... ');

            sb = proxy.SBaseAsObjectAsync.fetchOutputs();
            assert(~isempty(sb));
            assert(strcmp(sb.ice_id(), '::Test::SBase'));
            assert(strcmp(sb.sb, 'SBase.sb'));

            fprintf('ok\n');

            fprintf('base as Base... ');

            sb = proxy.SBaseAsSBase();
            assert(~isempty(sb));
            assert(strcmp(sb.ice_id(), '::Test::SBase'));
            assert(strcmp(sb.sb, 'SBase.sb'));

            fprintf('ok\n');

            fprintf('base as Base (AMI)... ');

            sb = proxy.SBaseAsSBaseAsync.fetchOutputs();
            assert(~isempty(sb));
            assert(strcmp(sb.ice_id(), '::Test::SBase'));
            assert(strcmp(sb.sb, 'SBase.sb'));

            fprintf('ok\n');

            fprintf('base with known derived as base... ');

            sbskd = proxy.SBSKnownDerivedAsSBase();
            assert(~isempty(sbskd));
            assert(strcmp(sbskd.sb, 'SBSKnownDerived.sb'));
            assert(strcmp(sbskd.sbskd, 'SBSKnownDerived.sbskd'));

            fprintf('ok\n');

            fprintf('base with known derived as base (AMI)... ');

            sbskd = proxy.SBSKnownDerivedAsSBaseAsync().fetchOutputs();
            assert(~isempty(sbskd));
            assert(strcmp(sbskd.sb, 'SBSKnownDerived.sb'));
            assert(strcmp(sbskd.sbskd, 'SBSKnownDerived.sbskd'));

            fprintf('ok\n');

            fprintf('base with known derived as known derived... ');

            sbskd = proxy.SBSKnownDerivedAsSBSKnownDerived();
            assert(~isempty(sbskd));
            assert(strcmp(sbskd.sb, 'SBSKnownDerived.sb'));
            assert(strcmp(sbskd.sbskd, 'SBSKnownDerived.sbskd'));

            fprintf('ok\n');

            fprintf('base with known derived as known derived (AMI)... ');

            sbskd = proxy.SBSKnownDerivedAsSBSKnownDerivedAsync().fetchOutputs();
            assert(~isempty(sbskd));
            assert(strcmp(sbskd.sb, 'SBSKnownDerived.sb'));
            assert(strcmp(sbskd.sbskd, 'SBSKnownDerived.sbskd'));

            fprintf('ok\n');

            fprintf('base with unknown derived as base... ');

            sb = proxy.SBSUnknownDerivedAsSBase();
            assert(~isempty(sb));
            assert(strcmp(sb.sb, 'SBSUnknownDerived.sb'));
            if proxy.ice_getEncodingVersion() == Ice.EncodingVersion(1, 0)
                try
                    sb = proxy.SBSUnknownDerivedAsSBaseCompact();
                    assert(strcmp(sb.sb, 'SBSUnknownDerived.sb'));
                catch ex
                    if ~isa(ex, 'Ice.OperationNotExistException')
                        rethrow(ex);
                    end
                end
            else
                try
                    %
                    % This test fails when using the compact format because the instance cannot
                    % be sliced to a known type.
                    %
                    proxy.SBSUnknownDerivedAsSBaseCompact();
                    assert(false);
                catch ex
                    if isa(ex, 'Ice.MarshalException')
                        % Expected.
                    elseif isa(ex, 'Ice.OperationNotExistException')
                        % Ignore
                    else
                        rethrow(ex);
                    end
                end
            end

            fprintf('ok\n');

            fprintf('base with unknown derived as base (AMI)... ');

            sbskd = proxy.SBSUnknownDerivedAsSBaseAsync().fetchOutputs();
            assert(~isempty(sb));
            assert(strcmp(sb.sb, 'SBSUnknownDerived.sb'));
            if proxy.ice_getEncodingVersion() == Ice.EncodingVersion(1, 0)
                try
                    sb = proxy.SBSUnknownDerivedAsSBaseCompactAsync().fetchOutputs();
                    assert(strcmp(sb.sb, 'SBSUnknownDerived.sb'));
                catch ex
                    if ~isa(ex, 'Ice.OperationNotExistException')
                        rethrow(ex);
                    end
                end
            else
                try
                    %
                    % This test fails when using the compact format because the instance cannot
                    % be sliced to a known type.
                    %
                    proxy.SBSUnknownDerivedAsSBaseCompactAsync().fetchOutputs();
                    assert(false);
                catch ex
                    if isa(ex, 'Ice.MarshalException')
                        % Expected.
                    elseif isa(ex, 'Ice.OperationNotExistException')
                        % Ignore
                    else
                        rethrow(ex);
                    end
                end
            end

            fprintf('ok\n');

            fprintf('unknown with Object as Object... ');

            try
                o = proxy.SUnknownAsObject();
                assert(proxy.ice_getEncodingVersion() ~= Ice.EncodingVersion(1, 0));
                assert(isa(o, 'Ice.UnknownSlicedValue'));
                assert(strcmp(o.ice_id(), '::Test::SUnknown'));
                assert(~isempty(o.ice_getSlicedData()));
                proxy.checkSUnknown(o);
            catch ex
                if isa(ex, 'Ice.MarshalException')
                    assert(proxy.ice_getEncodingVersion() == Ice.EncodingVersion(1, 0));
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('unknown with Object as Object (AMI)... ');

            try
                o = proxy.SUnknownAsObjectAsync().fetchOutputs();
                assert(proxy.ice_getEncodingVersion() ~= Ice.EncodingVersion(1, 0));
                assert(isa(o, 'Ice.UnknownSlicedValue'));
                assert(strcmp(o.ice_id(), '::Test::SUnknown'));
                assert(~isempty(o.ice_getSlicedData()));
                proxy.checkSUnknown(o);
            catch ex
                if isa(ex, 'Ice.MarshalException')
                    assert(proxy.ice_getEncodingVersion() == Ice.EncodingVersion(1, 0));
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('one-element cycle... ');

            b = proxy.oneElementCycle();
            assert(~isempty(b));
            assert(strcmp(b.ice_id(), '::Test::B'));
            assert(strcmp(b.sb, 'B1.sb'));
            assert(b.pb == b);

            fprintf('ok\n');

            fprintf('one-element cycle (AMI)... ');

            b = proxy.oneElementCycleAsync().fetchOutputs();
            assert(~isempty(b));
            assert(strcmp(b.ice_id(), '::Test::B'));
            assert(strcmp(b.sb, 'B1.sb'));
            assert(b.pb == b);

            fprintf('ok\n');

            fprintf('two-element cycle... ');

            b1 = proxy.twoElementCycle();
            assert(~isempty(b1));
            assert(strcmp(b1.ice_id(), '::Test::B'));
            assert(strcmp(b1.sb, 'B1.sb'));
            b2 = b1.pb;
            assert(~isempty(b2));
            assert(strcmp(b2.ice_id(), '::Test::B'));
            assert(strcmp(b2.sb, 'B2.sb'));
            assert(b2.pb == b1);

            fprintf('ok\n');

            fprintf('two-element cycle (AMI)... ');

            b1 = proxy.twoElementCycleAsync().fetchOutputs();
            assert(~isempty(b1));
            assert(strcmp(b1.ice_id(), '::Test::B'));
            assert(strcmp(b1.sb, 'B1.sb'));
            b2 = b1.pb;
            assert(~isempty(b2));
            assert(strcmp(b2.ice_id(), '::Test::B'));
            assert(strcmp(b2.sb, 'B2.sb'));
            assert(b2.pb == b1);

            fprintf('ok\n');

            fprintf('known derived pointer slicing as base... ');

            b1 = proxy.D1AsB();
            assert(~isempty(b1));
            assert(strcmp(b1.ice_id(), '::Test::D1'));
            assert(strcmp(b1.sb, 'D1.sb'));
            assert(~isempty(b1.pb));
            assert(b1.pb ~= b1);
            d1 = b1;
            assert(~isempty(d1));
            assert(strcmp(d1.sd1, 'D1.sd1'));
            assert(~isempty(d1.pd1));
            assert(d1.pd1 ~= b1);
            assert(b1.pb == d1.pd1);

            fprintf('ok\n');

            fprintf('known derived pointer slicing as base (AMI)... ');

            b1 = proxy.D1AsBAsync().fetchOutputs();
            assert(~isempty(b1));
            assert(strcmp(b1.ice_id(), '::Test::D1'));
            assert(strcmp(b1.sb, 'D1.sb'));
            assert(~isempty(b1.pb));
            assert(b1.pb ~= b1);
            d1 = b1;
            assert(~isempty(d1));
            assert(strcmp(d1.sd1, 'D1.sd1'));
            assert(~isempty(d1.pd1));
            assert(d1.pd1 ~= b1);
            assert(b1.pb == d1.pd1);

            fprintf('ok\n');

            fprintf('known derived pointer slicing as derived... ');

            d1 = proxy.D1AsD1();
            assert(~isempty(d1));
            assert(strcmp(d1.ice_id(), '::Test::D1'));
            assert(strcmp(d1.sb, 'D1.sb'));
            assert(~isempty(d1.pb));
            assert(d1.pb ~= d1);
            b2 = d1.pb;
            assert(~isempty(b2));
            assert(strcmp(b2.ice_id(), '::Test::B'));
            assert(strcmp(b2.sb, 'D2.sb'));
            assert(b2.pb == d1);

            fprintf('ok\n');

            fprintf('known derived pointer slicing as derived (AMI)... ');

            d1 = proxy.D1AsD1Async().fetchOutputs();
            assert(~isempty(d1));
            assert(strcmp(d1.ice_id(), '::Test::D1'));
            assert(strcmp(d1.sb, 'D1.sb'));
            assert(~isempty(d1.pb));
            assert(d1.pb ~= d1);
            b2 = d1.pb;
            assert(~isempty(b2));
            assert(strcmp(b2.ice_id(), '::Test::B'));
            assert(strcmp(b2.sb, 'D2.sb'));
            assert(b2.pb == d1);

            fprintf('ok\n');

            fprintf('unknown derived pointer slicing as base... ');

            b2 = proxy.D2AsB();
            assert(~isempty(b2));
            assert(strcmp(b2.ice_id(), '::Test::B'));
            assert(strcmp(b2.sb, 'D2.sb'));
            assert(~isempty(b2.pb));
            assert(b2.pb ~= b2);
            b1 = b2.pb;
            assert(~isempty(b1));
            assert(strcmp(b1.ice_id(), '::Test::D1'));
            assert(strcmp(b1.sb, 'D1.sb'));
            assert(b1.pb == b2);
            d1 = b1;
            assert(~isempty(d1));
            assert(strcmp(d1.sd1, 'D1.sd1'));
            assert(d1.pd1 == b2);

            fprintf('ok\n');

            fprintf('unknown derived pointer slicing as base (AMI)... ');

            b2 = proxy.D2AsBAsync.fetchOutputs();
            assert(~isempty(b2));
            assert(strcmp(b2.ice_id(), '::Test::B'));
            assert(strcmp(b2.sb, 'D2.sb'));
            assert(~isempty(b2.pb));
            assert(b2.pb ~= b2);
            b1 = b2.pb;
            assert(~isempty(b1));
            assert(strcmp(b1.ice_id(), '::Test::D1'));
            assert(strcmp(b1.sb, 'D1.sb'));
            assert(b1.pb == b2);
            d1 = b1;
            assert(~isempty(d1));
            assert(strcmp(d1.sd1, 'D1.sd1'));
            assert(d1.pd1 == b2);

            fprintf('ok\n');

            fprintf('param ptr slicing with known first... ');

            [p1, p2] = proxy.paramTest1();
            assert(~isempty(p1));
            assert(strcmp(p1.ice_id(), '::Test::D1'));
            assert(strcmp(p1.sb, 'D1.sb'));
            assert(p1.pb == p2);
            d1 = p1;
            assert(~isempty(d1));
            assert(strcmp(d1.sd1, 'D1.sd1'));
            assert(d1.pd1 == p2);
            assert(~isempty(p2));
            assert(strcmp(p2.ice_id(), '::Test::B')); % No factory, must be sliced
            assert(strcmp(p2.sb, 'D2.sb'));
            assert(p2.pb == p1);

            fprintf('ok\n');

            fprintf('param ptr slicing with known first (AMI)... ');

            [p1, p2] = proxy.paramTest1Async().fetchOutputs();
            assert(~isempty(p1));
            assert(strcmp(p1.ice_id(), '::Test::D1'));
            assert(strcmp(p1.sb, 'D1.sb'));
            assert(p1.pb == p2);
            d1 = p1;
            assert(~isempty(d1));
            assert(strcmp(d1.sd1, 'D1.sd1'));
            assert(d1.pd1 == p2);
            assert(~isempty(p2));
            assert(strcmp(p2.ice_id(), '::Test::B')); % No factory, must be sliced
            assert(strcmp(p2.sb, 'D2.sb'));
            assert(p2.pb == p1);

            fprintf('ok\n');

            fprintf('param ptr slicing with unknown first... ');

            [p2, p1] = proxy.paramTest2();
            assert(~isempty(p1));
            assert(strcmp(p1.ice_id(), '::Test::D1'));
            assert(strcmp(p1.sb, 'D1.sb'));
            assert(p1.pb == p2);
            d1 = p1;
            assert(~isempty(d1));
            assert(strcmp(d1.sd1, 'D1.sd1'));
            assert(d1.pd1 == p2);
            assert(~isempty(p2));
            assert(strcmp(p2.ice_id(), '::Test::B')); % No factory, must be sliced
            assert(strcmp(p2.sb, 'D2.sb'));
            assert(p2.pb == p1);

            fprintf('ok\n');

            fprintf('param ptr slicing with unknown first (AMI)... ');

            [p2, p1] = proxy.paramTest2Async().fetchOutputs();
            assert(~isempty(p1));
            assert(strcmp(p1.ice_id(), '::Test::D1'));
            assert(strcmp(p1.sb, 'D1.sb'));
            assert(p1.pb == p2);
            d1 = p1;
            assert(~isempty(d1));
            assert(strcmp(d1.sd1, 'D1.sd1'));
            assert(d1.pd1 == p2);
            assert(~isempty(p2));
            assert(strcmp(p2.ice_id(), '::Test::B')); % No factory, must be sliced
            assert(strcmp(p2.sb, 'D2.sb'));
            assert(p2.pb == p1);

            fprintf('ok\n');

            fprintf('return value identity with known first... ');

            [r, p1, p2] = proxy.returnTest1();
            assert(r == p1);

            fprintf('ok\n');

            fprintf('return value identity with known first (AMI)... ');

            [r, p1, p2] = proxy.returnTest1Async().fetchOutputs();
            assert(r == p1);

            fprintf('ok\n');

            fprintf('return value identity with unknown first... ');

            [r, p2, p1] = proxy.returnTest2();
            assert(r == p2);

            fprintf('ok\n');

            fprintf('return value identity with unknown first (AMI)... ');

            [r, p2, p1] = proxy.returnTest2Async().fetchOutputs();
            assert(r == p2);

            fprintf('ok\n');

            fprintf('return value identity for input params known first... ');

            d1 = D1();
            d1.sb = 'D1.sb';
            d1.sd1 = 'D1.sd1';
            d3 = D3();
            d3.pb = d1;
            d3.sb = 'D3.sb';
            d3.sd3 = 'D3.sd3';
            d3.pd3 = d1;
            d1.pb = d3;
            d1.pd1 = d3;

            b1 = proxy.returnTest3(d1, d3);

            assert(~isempty(b1));
            assert(strcmp(b1.sb, 'D1.sb'));
            assert(strcmp(b1.ice_id(), '::Test::D1'));
            p1 = b1;
            assert(~isempty(p1));
            assert(strcmp(p1.sd1, 'D1.sd1'));
            assert(p1.pd1 == b1.pb);

            b2 = b1.pb;
            assert(~isempty(b2));
            assert(strcmp(b2.sb, 'D3.sb'));
            assert(b2.pb == b1);
            if proxy.ice_getEncodingVersion() == Ice.EncodingVersion(1, 0)
                assert(~isa(b2, 'Test.D3'));
            else
                assert(isa(b2, 'Test.D3'));
                assert(strcmp(b2.ice_id(), '::Test::D3'));  % preserved
            end

            assert(b1 ~= d1);
            assert(b1 ~= d3);
            assert(b2 ~= d1);
            assert(b2 ~= d3);

            fprintf('ok\n');

            fprintf('return value identity for input params known first (AMI)... ');

            d1 = D1();
            d1.sb = 'D1.sb';
            d1.sd1 = 'D1.sd1';
            d3 = D3();
            d3.pb = d1;
            d3.sb = 'D3.sb';
            d3.sd3 = 'D3.sd3';
            d3.pd3 = d1;
            d1.pb = d3;
            d1.pd1 = d3;

            b1 = proxy.returnTest3Async(d1, d3).fetchOutputs();

            assert(~isempty(b1));
            assert(strcmp(b1.sb, 'D1.sb'));
            assert(strcmp(b1.ice_id(), '::Test::D1'));
            p1 = b1;
            assert(~isempty(p1));
            assert(strcmp(p1.sd1, 'D1.sd1'));
            assert(p1.pd1 == b1.pb);

            b2 = b1.pb;
            assert(~isempty(b2));
            assert(strcmp(b2.sb, 'D3.sb'));
            assert(b2.pb == b1);
            if proxy.ice_getEncodingVersion() == Ice.EncodingVersion(1, 0)
                assert(~isa(b2, 'Test.D3'));
            else
                assert(isa(b2, 'Test.D3'));
                assert(strcmp(b2.ice_id(), '::Test::D3'));  % preserved
            end

            assert(b1 ~= d1);
            assert(b1 ~= d3);
            assert(b2 ~= d1);
            assert(b2 ~= d3);

            fprintf('ok\n');

            fprintf('return value identity for input params unknown first... ');

            d1 = D1();
            d1.sb = 'D1.sb';
            d1.sd1 = 'D1.sd1';
            d3 = D3();
            d3.pb = d1;
            d3.sb = 'D3.sb';
            d3.sd3 = 'D3.sd3';
            d3.pd3 = d1;
            d1.pb = d3;
            d1.pd1 = d3;

            b1 = proxy.returnTest3(d3, d1);

            assert(~isempty(b1));
            assert(strcmp(b1.sb, 'D3.sb'));
            if proxy.ice_getEncodingVersion() == Ice.EncodingVersion(1, 0)
                assert(~isa(b1, 'Test.D3'));
            else
                assert(isa(b1, 'Test.D3'));
                assert(strcmp(b1.ice_id(), '::Test::D3')); % preserved
            end

            b2 = b1.pb;
            assert(~isempty(b2));
            assert(strcmp(b2.sb, 'D1.sb'));
            assert(strcmp(b2.ice_id(), '::Test::D1'));
            assert(b2.pb == b1);
            p3 = b2;
            assert(~isempty(p3));
            assert(strcmp(p3.sd1, 'D1.sd1'));
            assert(p3.pd1 == b1);

            assert(b1 ~= d1);
            assert(b1 ~= d3);
            assert(b2 ~= d1);
            assert(b2 ~= d3);

            fprintf('ok\n');

            fprintf('return value identity for input params unknown first (AMI)... ');

            d1 = D1();
            d1.sb = 'D1.sb';
            d1.sd1 = 'D1.sd1';
            d3 = D3();
            d3.pb = d1;
            d3.sb = 'D3.sb';
            d3.sd3 = 'D3.sd3';
            d3.pd3 = d1;
            d1.pb = d3;
            d1.pd1 = d3;

            b1 = proxy.returnTest3Async(d3, d1).fetchOutputs();

            assert(~isempty(b1));
            assert(strcmp(b1.sb, 'D3.sb'));
            if proxy.ice_getEncodingVersion() == Ice.EncodingVersion(1, 0)
                assert(~isa(b1, 'Test.D3'));
            else
                assert(isa(b1, 'Test.D3'));
                assert(strcmp(b1.ice_id(), '::Test::D3')); % preserved
            end

            b2 = b1.pb;
            assert(~isempty(b2));
            assert(strcmp(b2.sb, 'D1.sb'));
            assert(strcmp(b2.ice_id(), '::Test::D1'));
            assert(b2.pb == b1);
            p3 = b2;
            assert(~isempty(p3));
            assert(strcmp(p3.sd1, 'D1.sd1'));
            assert(p3.pd1 == b1);

            assert(b1 ~= d1);
            assert(b1 ~= d3);
            assert(b2 ~= d1);
            assert(b2 ~= d3);

            fprintf('ok\n');

            fprintf('remainder unmarshaling (3 instances)... ');

            [r, p1, p2] = proxy.paramTest3();

            assert(~isempty(p1));
            assert(strcmp(p1.sb, 'D2.sb (p1 1)'));
            assert(isempty(p1.pb));
            assert(strcmp(p1.ice_id(), '::Test::B'));

            assert(~isempty(p2));
            assert(strcmp(p2.sb, 'D2.sb (p2 1)'));
            assert(isempty(p2.pb));
            assert(strcmp(p2.ice_id(), '::Test::B'));

            assert(~isempty(r));
            assert(strcmp(r.sb, 'D1.sb (p2 2)'));
            assert(isempty(r.pb));
            assert(strcmp(r.ice_id(), '::Test::D1'));

            fprintf('ok\n');

            fprintf('remainder unmarshaling (3 instances) (AMI)... ');

            [r, p1, p2] = proxy.paramTest3Async().fetchOutputs();

            assert(~isempty(p1));
            assert(strcmp(p1.sb, 'D2.sb (p1 1)'));
            assert(isempty(p1.pb));
            assert(strcmp(p1.ice_id(), '::Test::B'));

            assert(~isempty(p2));
            assert(strcmp(p2.sb, 'D2.sb (p2 1)'));
            assert(isempty(p2.pb));
            assert(strcmp(p2.ice_id(), '::Test::B'));

            assert(~isempty(r));
            assert(strcmp(r.sb, 'D1.sb (p2 2)'));
            assert(isempty(r.pb));
            assert(strcmp(r.ice_id(), '::Test::D1'));

            fprintf('ok\n');

            fprintf('remainder unmarshaling (4 instances)... ');

            [r, p] = proxy.paramTest4();

            assert(~isempty(p));
            assert(strcmp(p.sb, 'D4.sb (1)'));
            assert(isempty(p.pb));
            assert(strcmp(p.ice_id(), '::Test::B'));

            assert(~isempty(r));
            assert(strcmp(r.sb, 'B.sb (2)'));
            assert(isempty(r.pb));
            assert(strcmp(r.ice_id(), '::Test::B'));

            fprintf('ok\n');

            fprintf('remainder unmarshaling (4 instances) (AMI)... ');

            [r, p] = proxy.paramTest4();

            assert(~isempty(p));
            assert(strcmp(p.sb, 'D4.sb (1)'));
            assert(isempty(p.pb));
            assert(strcmp(p.ice_id(), '::Test::B'));

            assert(~isempty(r));
            assert(strcmp(r.sb, 'B.sb (2)'));
            assert(isempty(r.pb));
            assert(strcmp(r.ice_id(), '::Test::B'));

            fprintf('ok\n');

            fprintf('param ptr slicing, instance marshaled in unknown derived as base... ');

            b1 = B();
            b1.sb = 'B.sb(1)';
            b1.pb = b1;

            d3 = D3();
            d3.sb = 'D3.sb';
            d3.pb = d3;
            d3.sd3 = 'D3.sd3';
            d3.pd3 = b1;

            b2 = B();
            b2.sb = 'B.sb(2)';
            b2.pb = b1;

            r = proxy.returnTest3(d3, b2);

            assert(~isempty(r));
            if proxy.ice_getEncodingVersion() == Ice.EncodingVersion(1, 0)
                assert(~isa(r, 'Test.D3'));
            else
                assert(isa(r, 'Test.D3'));
                assert(strcmp(r.ice_id(), '::Test::D3'));
                assert(strcmp(r.sb, 'D3.sb'));
                assert(r.pb == r);
            end

            fprintf('ok\n');

            fprintf('param ptr slicing, instance marshaled in unknown derived as base (AMI)... ');

            b1 = B();
            b1.sb = 'B.sb(1)';
            b1.pb = b1;

            d3 = D3();
            d3.sb = 'D3.sb';
            d3.pb = d3;
            d3.sd3 = 'D3.sd3';
            d3.pd3 = b1;

            b2 = B();
            b2.sb = 'B.sb(2)';
            b2.pb = b1;

            r = proxy.returnTest3Async(d3, b2).fetchOutputs();

            assert(~isempty(r));
            if proxy.ice_getEncodingVersion() == Ice.EncodingVersion(1, 0)
                assert(~isa(r, 'Test.D3'));
            else
                assert(isa(r, 'Test.D3'));
                assert(strcmp(r.ice_id(), '::Test::D3'));
                assert(strcmp(r.sb, 'D3.sb'));
                assert(r.pb == r);
            end

            fprintf('ok\n');

            fprintf('param ptr slicing, instance marshaled in unknown derived as derived... ');

            d11 = D1();
            d11.sb = 'D1.sb(1)';
            d11.pb = d11;
            d11.sd1 = 'D1.sd1(1)';

            d3 = D3();
            d3.sb = 'D3.sb';
            d3.pb = d3;
            d3.sd3 = 'D3.sd3';
            d3.pd3 = d11;

            d12 = D1();
            d12.sb = 'D1.sb(2)';
            d12.pb = d12;
            d12.sd1 = 'D1.sd1(2)';
            d12.pd1 = d11;

            r = proxy.returnTest3(d3, d12);
            assert(~isempty(r));
            if proxy.ice_getEncodingVersion() == Ice.EncodingVersion(1, 0)
                assert(~isa(r, 'Test.D3'));
            else
                assert(isa(r, 'Test.D3'));
                assert(strcmp(r.ice_id(), '::Test::D3'));
                assert(strcmp(r.sb, 'D3.sb'));
                assert(r.pb == r);
            end
            fprintf('ok\n');

            fprintf('param ptr slicing, instance marshaled in unknown derived as derived (AMI)... ');

            d11 = D1();
            d11.sb = 'D1.sb(1)';
            d11.pb = d11;
            d11.sd1 = 'D1.sd1(1)';

            d3 = D3();
            d3.sb = 'D3.sb';
            d3.pb = d3;
            d3.sd3 = 'D3.sd3';
            d3.pd3 = d11;

            d12 = D1();
            d12.sb = 'D1.sb(2)';
            d12.pb = d12;
            d12.sd1 = 'D1.sd1(2)';
            d12.pd1 = d11;

            r = proxy.returnTest3Async(d3, d12).fetchOutputs();
            assert(~isempty(r));
            if proxy.ice_getEncodingVersion() == Ice.EncodingVersion(1, 0)
                assert(~isa(r, 'Test.D3'));
            else
                assert(isa(r, 'Test.D3'));
                assert(strcmp(r.ice_id(), '::Test::D3'));
                assert(strcmp(r.sb, 'D3.sb'));
                assert(r.pb == r);
            end
            fprintf('ok\n');

            fprintf('sequence slicing... ');

            ss1b = B();
            ss1b.sb = 'B.sb';
            ss1b.pb = ss1b;

            ss1d1 = D1();
            ss1d1.sb = 'D1.sb';
            ss1d1.sd1 = 'D1.sd1';
            ss1d1.pb = ss1b;

            ss1d3 = D3();
            ss1d3.sb = 'D3.sb';
            ss1d3.sd3 = 'D3.sd3';
            ss1d3.pb = ss1b;

            ss2b = B();
            ss2b.sb = 'B.sb';
            ss2b.pb = ss1b;

            ss2d1 = D1();
            ss2d1.sb = 'D1.sb';
            ss2d1.sd1 = 'D1.sd1';
            ss2d1.pb = ss2b;

            ss2d3 = D3();
            ss2d3.sb = 'D3.sb';
            ss2d3.sd3 = 'D3.sd3';
            ss2d3.pb = ss2b;

            ss1d1.pd1 = ss2b;
            ss1d3.pd3 = ss2d1;

            ss2d1.pd1 = ss1d3;
            ss2d3.pd3 = ss1d1;

            ss1 = SS1();
            ss1.s = {};
            ss1.s{1} = ss1b;
            ss1.s{2} = ss1d1;
            ss1.s{3} = ss1d3;

            ss2 = SS2();
            ss2.s = {};
            ss2.s{1} = ss2b;
            ss2.s{2} = ss2d1;
            ss2.s{3} = ss2d3;

            ss = proxy.sequenceTest(ss1, ss2);

            assert(~isempty(ss.c1));
            ss1b = ss.c1.s{1};
            ss1d1 = ss.c1.s{2};
            assert(~isempty(ss.c2));
            ss1d3 = ss.c1.s{3};

            assert(~isempty(ss.c2));
            ss2b = ss.c2.s{1};
            ss2d1 = ss.c2.s{2};
            ss2d3 = ss.c2.s{3};

            assert(ss1b.pb == ss1b);
            assert(ss1d1.pb == ss1b);
            assert(ss1d3.pb == ss1b);

            assert(ss2b.pb == ss1b);
            assert(ss2d1.pb == ss2b);
            assert(ss2d3.pb == ss2b);

            assert(strcmp(ss1b.ice_id(), '::Test::B'));
            assert(strcmp(ss1d1.ice_id(), '::Test::D1'));
            if proxy.ice_getEncodingVersion() == Ice.EncodingVersion(1, 0)
                assert(~isa(ss1d3, 'Test.D3'));
            else
                assert(strcmp(ss1d3.ice_id(), '::Test::D3'));
            end

            assert(strcmp(ss2b.ice_id(), '::Test::B'));
            assert(strcmp(ss2d1.ice_id(), '::Test::D1'));
            if proxy.ice_getEncodingVersion() == Ice.EncodingVersion(1, 0)
                assert(~isa(ss2d3, 'Test.D3'));
            else
                assert(strcmp(ss2d3.ice_id(), '::Test::D3'));
            end

            fprintf('ok\n');

            fprintf('sequence slicing (AMI)... ');

            ss1b = B();
            ss1b.sb = 'B.sb';
            ss1b.pb = ss1b;

            ss1d1 = D1();
            ss1d1.sb = 'D1.sb';
            ss1d1.sd1 = 'D1.sd1';
            ss1d1.pb = ss1b;

            ss1d3 = D3();
            ss1d3.sb = 'D3.sb';
            ss1d3.sd3 = 'D3.sd3';
            ss1d3.pb = ss1b;

            ss2b = B();
            ss2b.sb = 'B.sb';
            ss2b.pb = ss1b;

            ss2d1 = D1();
            ss2d1.sb = 'D1.sb';
            ss2d1.sd1 = 'D1.sd1';
            ss2d1.pb = ss2b;

            ss2d3 = D3();
            ss2d3.sb = 'D3.sb';
            ss2d3.sd3 = 'D3.sd3';
            ss2d3.pb = ss2b;

            ss1d1.pd1 = ss2b;
            ss1d3.pd3 = ss2d1;

            ss2d1.pd1 = ss1d3;
            ss2d3.pd3 = ss1d1;

            ss1 = SS1();
            ss1.s = {};
            ss1.s{1} = ss1b;
            ss1.s{2} = ss1d1;
            ss1.s{3} = ss1d3;

            ss2 = SS2();
            ss2.s = {};
            ss2.s{1} = ss2b;
            ss2.s{2} = ss2d1;
            ss2.s{3} = ss2d3;

            ss = proxy.sequenceTestAsync(ss1, ss2).fetchOutputs();

            assert(~isempty(ss.c1));
            ss1b = ss.c1.s{1};
            ss1d1 = ss.c1.s{2};
            assert(~isempty(ss.c2));
            ss1d3 = ss.c1.s{3};

            assert(~isempty(ss.c2));
            ss2b = ss.c2.s{1};
            ss2d1 = ss.c2.s{2};
            ss2d3 = ss.c2.s{3};

            assert(ss1b.pb == ss1b);
            assert(ss1d1.pb == ss1b);
            assert(ss1d3.pb == ss1b);

            assert(ss2b.pb == ss1b);
            assert(ss2d1.pb == ss2b);
            assert(ss2d3.pb == ss2b);

            assert(strcmp(ss1b.ice_id(), '::Test::B'));
            assert(strcmp(ss1d1.ice_id(), '::Test::D1'));
            if proxy.ice_getEncodingVersion() == Ice.EncodingVersion(1, 0)
                assert(~isa(ss1d3, 'Test.D3'));
            else
                assert(strcmp(ss1d3.ice_id(), '::Test::D3'));
            end

            assert(strcmp(ss2b.ice_id(), '::Test::B'));
            assert(strcmp(ss2d1.ice_id(), '::Test::D1'));
            if proxy.ice_getEncodingVersion() == Ice.EncodingVersion(1, 0)
                assert(~isa(ss2d3, 'Test.D3'));
            else
                assert(strcmp(ss2d3.ice_id(), '::Test::D3'));
            end

            fprintf('ok\n');

            fprintf('dictionary slicing... ');

            bin = configureDictionary('int32', 'cell');
            for i = 0:9
                s = ['D1.', num2str(i)];
                d1 = D1();
                d1.sb = s;
                d1.pb = d1;
                d1.sd1 = s;
                bin{i} = d1;
            end

            [r, bout] = proxy.dictionaryTest(bin);

            assert(bout.numEntries == 10);
            for i = 0:9
                b = bout{i * 10};
                assert(~isempty(b));
                s = ['D1.', num2str(i)];
                assert(strcmp(b.sb, s));
                assert(~isempty(b.pb));
                assert(b.pb ~= b);
                assert(strcmp(b.pb.sb, s));
                assert(b.pb.pb == b.pb);
            end

            assert(r.numEntries == 10);
            for i = 0:9
                b = r{i * 20};
                assert(~isempty(b));
                s = ['D1.', num2str(i * 20)];
                assert(strcmp(b.sb, s));
                if i == 0
                    assert(isempty(b.pb));
                else
                    assert(b.pb == r{(i - 1) * 20});
                end
                d1 = b;
                assert(~isempty(d1));
                assert(strcmp(d1.sd1, s));
                assert(d1.pd1 == d1);
            end

            fprintf('ok\n');

            fprintf('dictionary slicing (AMI)... ');

            bin = configureDictionary('int32', 'cell');
            for i = 0:9
                s = ['D1.', num2str(i)];
                d1 = D1();
                d1.sb = s;
                d1.pb = d1;
                d1.sd1 = s;
                bin{i} = d1;
            end

            [r, bout] = proxy.dictionaryTestAsync(bin).fetchOutputs();

            assert(bout.numEntries == 10);
            for i = 0:9
                b = bout{i * 10};
                assert(~isempty(b));
                s = ['D1.', num2str(i)];
                assert(strcmp(b.sb, s));
                assert(~isempty(b.pb));
                assert(b.pb ~= b);
                assert(strcmp(b.pb.sb, s));
                assert(b.pb.pb == b.pb);
            end

            assert(r.numEntries == 10);
            for i = 0:9
                b = r{i * 20};
                assert(~isempty(b));
                s = ['D1.', num2str(i * 20)];
                assert(strcmp(b.sb, s));
                if i == 0
                    assert(isempty(b.pb));
                else
                    assert(b.pb == r{(i - 1) * 20});
                end
                d1 = b;
                assert(~isempty(d1));
                assert(strcmp(d1.sd1, s));
                assert(d1.pd1 == d1);
            end

            fprintf('ok\n');

            fprintf('base exception thrown as base exception... ');
            try
                proxy.throwBaseAsBase();
                assert(false);
            catch ex
                if isa(ex, 'Test.BaseException')
                    assert(strcmp(ex.ice_id(), '::Test::BaseException'));
                    assert(strcmp(ex.sbe, 'sbe'));
                    assert(~isempty(ex.pb));
                    assert(strcmp(ex.pb.sb, 'sb'));
                    assert(ex.pb.pb == ex.pb);
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('base exception thrown as base exception (AMI)... ');
            try
                proxy.throwBaseAsBaseAsync().fetchOutputs();
                assert(false);
            catch ex
                if isa(ex, 'Test.BaseException')
                    assert(strcmp(ex.ice_id(), '::Test::BaseException'));
                    assert(strcmp(ex.sbe, 'sbe'));
                    assert(~isempty(ex.pb));
                    assert(strcmp(ex.pb.sb, 'sb'));
                    assert(ex.pb.pb == ex.pb);
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('derived exception thrown as base exception... ');
            try
                proxy.throwDerivedAsBase();
                assert(false);
            catch ex
                if isa(ex, 'Test.DerivedException')
                    assert(strcmp(ex.ice_id(), '::Test::DerivedException'));
                    assert(strcmp(ex.sbe, 'sbe'));
                    assert(~isempty(ex.pb));
                    assert(strcmp(ex.pb.sb, 'sb1'));
                    assert(ex.pb.pb == ex.pb);
                    assert(strcmp(ex.sde, 'sde1'));
                    assert(~isempty(ex.pd1));
                    assert(strcmp(ex.pd1.sb, 'sb2'));
                    assert(ex.pd1.pb == ex.pd1);
                    assert(strcmp(ex.pd1.sd1, 'sd2'));
                    assert(ex.pd1.pd1 == ex.pd1);
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('derived exception thrown as base exception (AMI)... ');
            try
                proxy.throwDerivedAsBaseAsync().fetchOutputs();
                assert(false);
            catch ex
                if isa(ex, 'Test.DerivedException')
                    assert(strcmp(ex.ice_id(), '::Test::DerivedException'));
                    assert(strcmp(ex.sbe, 'sbe'));
                    assert(~isempty(ex.pb));
                    assert(strcmp(ex.pb.sb, 'sb1'));
                    assert(ex.pb.pb == ex.pb);
                    assert(strcmp(ex.sde, 'sde1'));
                    assert(~isempty(ex.pd1));
                    assert(strcmp(ex.pd1.sb, 'sb2'));
                    assert(ex.pd1.pb == ex.pd1);
                    assert(strcmp(ex.pd1.sd1, 'sd2'));
                    assert(ex.pd1.pd1 == ex.pd1);
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('derived exception thrown as derived exception... ');
            try
                proxy.throwDerivedAsDerived();
                assert(false);
            catch ex
                if isa(ex, 'Test.DerivedException')
                    assert(strcmp(ex.ice_id(), '::Test::DerivedException'));
                    assert(strcmp(ex.sbe, 'sbe'));
                    assert(~isempty(ex.pb));
                    assert(strcmp(ex.pb.sb, 'sb1'));
                    assert(ex.pb.pb == ex.pb);
                    assert(strcmp(ex.sde, 'sde1'));
                    assert(~isempty(ex.pd1));
                    assert(strcmp(ex.pd1.sb, 'sb2'));
                    assert(ex.pd1.pb == ex.pd1);
                    assert(strcmp(ex.pd1.sd1, 'sd2'));
                    assert(ex.pd1.pd1 == ex.pd1);
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('derived exception thrown as derived exception (AMI)... ');
            try
                proxy.throwDerivedAsDerivedAsync().fetchOutputs();
                assert(false);
            catch ex
                if isa(ex, 'Test.DerivedException')
                    assert(strcmp(ex.ice_id(), '::Test::DerivedException'));
                    assert(strcmp(ex.sbe, 'sbe'));
                    assert(~isempty(ex.pb));
                    assert(strcmp(ex.pb.sb, 'sb1'));
                    assert(ex.pb.pb == ex.pb);
                    assert(strcmp(ex.sde, 'sde1'));
                    assert(~isempty(ex.pd1));
                    assert(strcmp(ex.pd1.sb, 'sb2'));
                    assert(ex.pd1.pb == ex.pd1);
                    assert(strcmp(ex.pd1.sd1, 'sd2'));
                    assert(ex.pd1.pd1 == ex.pd1);
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('unknown derived exception thrown as base exception... ');
            try
                proxy.throwUnknownDerivedAsBase();
                assert(false);
            catch ex
                if isa(ex, 'Test.BaseException')
                    assert(strcmp(ex.ice_id(), '::Test::BaseException'));
                    assert(strcmp(ex.sbe, 'sbe'));
                    assert(~isempty(ex.pb));
                    assert(strcmp(ex.pb.sb, 'sb d2'));
                    assert(ex.pb.pb == ex.pb);
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('unknown derived exception thrown as base exception (AMI)... ');
            try
                proxy.throwUnknownDerivedAsBaseAsync().fetchOutputs();
                assert(false);
            catch ex
                if isa(ex, 'Test.BaseException')
                    assert(strcmp(ex.ice_id(), '::Test::BaseException'));
                    assert(strcmp(ex.sbe, 'sbe'));
                    assert(~isempty(ex.pb));
                    assert(strcmp(ex.pb.sb, 'sb d2'));
                    assert(ex.pb.pb == ex.pb);
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('forward-declared class... ');

            f = proxy.useForward();
            assert(~isempty(f));

            fprintf('ok\n');

            fprintf('forward-declared class (AMI)... ');

            f = proxy.useForwardAsync().fetchOutputs();
            assert(~isempty(f));

            fprintf('ok\n');

            fprintf('preserved classes... ');

            try
                %
                % Server knows the most-derived class PDerived.
                %
                pd = PDerived();
                pd.pi = 3;
                pd.ps = 'preserved';
                pd.pb = pd;

                r = proxy.exchangePBase(pd);
                p2 = r;
                assert(p2.pi == 3);
                assert(strcmp(p2.ps, 'preserved'));
                assert(p2.pb == p2);
            catch ex
                if isa(ex, 'Ice.OperationNotExistException')
                    % Ignore
                else
                    rethrow(ex);
                end
            end

            try
                %
                % Send an object that will have multiple preserved slices in the server.
                % The object will be sliced to Preserved for the 1.0 encoding.
                %
                pcd = PCDerived3();
                pcd.pi = 3;
                %
                % Sending more than 254 objects exercises the encoding for object ids.
                %
                pcd.pbs = {};
                for i = 1:300
                    p2 = PCDerived2();
                    p2.pi = i;
                    p2.pbs = {};
                    p2.pbs{1} = []; % Nil reference. This slice should not have an indirection table.
                    p2.pcd2 = i;
                    pcd.pbs{i} = p2;
                end
                pcd.pcd2 = pcd.pi;
                pcd.pcd3 = pcd.pbs{10};

                r = proxy.exchangePBase(pcd);
                if proxy.ice_getEncodingVersion() == Ice.EncodingVersion(1, 0)
                    assert(~isa(r, 'Test.PCDerived3'));
                    assert(isa(r, 'Test.Preserved'));
                    assert(r.pi == 3);
                else
                    p3 = r;
                    assert(p3.pi == 3);
                    for i = 1:300
                        p2 = p3.pbs{i};
                        assert(p2.pi == i);
                        assert(length(p2.pbs) == 1);
                        assert(isempty(p2.pbs{1}));
                        assert(p2.pcd2 == i);
                    end
                    assert(p3.pcd2 == p3.pi);
                    assert(p3.pcd3 == p3.pbs{10});
                end
            catch ex
                if isa(ex, 'Ice.OperationNotExistException')
                    % Ignore
                else
                    rethrow(ex);
                end
            end

            try
                %
                % Obtain an object with preserved slices and send it back to the server.
                % The preserved slices should be excluded for the 1.0 encoding, otherwise
                % they should be included.
                %
                p = proxy.PBSUnknownAsPreserved();
                assert(~isempty(p));
                proxy.checkPBSUnknown(p);
                slicedData = p.ice_getSlicedData();
                assert(~isempty(slicedData));
                if proxy.ice_getEncodingVersion() ~= Ice.EncodingVersion(1, 0)
                    assert(length(slicedData.slices) == 1);
                    assert(strcmp(slicedData.slices{1}.typeId, '::Test::PSUnknown'));
                    proxy.ice_encodingVersion(Ice.EncodingVersion(1, 0)).checkPBSUnknown(p);
                else
                    assert(length(slicedData.slices) == 0);
                end
            catch ex
                if isa(ex, 'Ice.OperationNotExistException')
                    % Ignore
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('preserved classes (AMI)... ');

            try
                %
                % Server knows the most-derived class PDerived.
                %
                pd = PDerived();
                pd.pi = 3;
                pd.ps = 'preserved';
                pd.pb = pd;

                r = proxy.exchangePBaseAsync(pd).fetchOutputs();
                p2 = r;
                assert(p2.pi == 3);
                assert(strcmp(p2.ps, 'preserved'));
                assert(p2.pb == p2);
            catch ex
                if isa(ex, 'Ice.OperationNotExistException')
                    % Ignore
                else
                    rethrow(ex);
                end
            end

            try
                %
                % Send an object that will have multiple preserved slices in the server.
                % The object will be sliced to Preserved for the 1.0 encoding.
                %
                pcd = PCDerived3();
                pcd.pi = 3;
                %
                % Sending more than 254 objects exercises the encoding for object ids.
                %
                pcd.pbs = {};
                for i = 1:300
                    p2 = PCDerived2();
                    p2.pi = i;
                    p2.pbs = {};
                    p2.pbs{1} = []; % Nil reference. This slice should not have an indirection table.
                    p2.pcd2 = i;
                    pcd.pbs{i} = p2;
                end
                pcd.pcd2 = pcd.pi;
                pcd.pcd3 = pcd.pbs{10};

                r = proxy.exchangePBaseAsync(pcd).fetchOutputs();
                if proxy.ice_getEncodingVersion() == Ice.EncodingVersion(1, 0)
                    assert(~isa(r, 'Test.PCDerived3'));
                    assert(isa(r, 'Test.Preserved'));
                    assert(r.pi == 3);
                else
                    p3 = r;
                    assert(p3.pi == 3);
                    for i = 1:300
                        p2 = p3.pbs{i};
                        assert(p2.pi == i);
                        assert(length(p2.pbs) == 1);
                        assert(isempty(p2.pbs{1}));
                        assert(p2.pcd2 == i);
                    end
                    assert(p3.pcd2 == p3.pi);
                    assert(p3.pcd3 == p3.pbs{10});
                end
            catch ex
                if isa(ex, 'Ice.OperationNotExistException')
                    % Ignore
                else
                    rethrow(ex);
                end
            end

            try
                %
                % Obtain an object with preserved slices and send it back to the server.
                % The preserved slices should be excluded for the 1.0 encoding, otherwise
                % they should be included.
                %
                p = proxy.PBSUnknownAsPreservedAsync().fetchOutputs();
                assert(~isempty(p));
                proxy.checkPBSUnknown(p);
                slicedData = p.ice_getSlicedData();
                assert(~isempty(slicedData));
                if proxy.ice_getEncodingVersion() ~= Ice.EncodingVersion(1, 0)
                    assert(length(slicedData.slices) == 1);
                    assert(strcmp(slicedData.slices{1}.typeId, '::Test::PSUnknown'));
                    proxy.ice_encodingVersion(Ice.EncodingVersion(1, 0)).checkPBSUnknown(p);
                else
                    assert(length(slicedData.slices) == 0);
                end
            catch ex
                if isa(ex, 'Ice.OperationNotExistException')
                    % Ignore
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('garbage collection for preserved classes... ');
            try
                %
                % Relay a graph through the server.
                %
                c = PNode();
                c.next = PNode();
                c.next.next = PNode();
                c.next.next.next = c;

                assert(customSliceLoader.nodeCounter == 0);
                proxy.exchangePNode(c);

                assert(customSliceLoader.nodeCounter == 3);
                customSliceLoader.nodeCounter = 0;

                %
                % Obtain a preserved object from the server where the most-derived
                % type is unknown. The preserved slice refers to a graph of PNode
                % objects.
                %
                assert(customSliceLoader.nodeCounter == 0);
                p = proxy.PBSUnknownAsPreservedWithGraph();
                proxy.checkPBSUnknownWithGraph(p);
                assert(customSliceLoader.nodeCounter == 3);
                customSliceLoader.nodeCounter = 0;

                %
                % Obtain a preserved object from the server where the most-derived
                % type is unknown. A data member in the preserved slice refers to the
                % outer object, so the chain of references looks like this:
                %
                % outer.slicedData.outer
                %
                customSliceLoader.preservedCounter = 0;
                p = proxy.PBSUnknown2AsPreservedWithGraph();
                proxy.checkPBSUnknown2WithGraph(p);
                assert(customSliceLoader.preservedCounter == 1);
                customSliceLoader.preservedCounter = 0;

                %
                % Throw a preserved exception where the most-derived type is unknown.
                % The preserved exception slice contains a class data member. This
                % object is also preserved, and its most-derived type is also unknown.
                % The preserved slice of the object contains a class data member that
                % refers to itself.
                %
                % The chain of references looks like this:
                %
                % ex.slicedData.obj.slicedData.obj
                %
                assert(customSliceLoader.preservedCounter == 0);

                try
                    proxy.throwPreservedException();
                catch ex
                    if isa(ex, 'Test.PreservedException')
                        assert(customSliceLoader.preservedCounter == 1);
                    else
                        rethrow(ex);
                    end
                end

                customSliceLoader.preservedCounter = 0;
            catch ex
                if isa(ex, 'Ice.OperationNotExistException')
                    % Ignore
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            r = proxy;
        end
    end
end
