% Copyright (c) ZeroC, Inc.

classdef AllTests
    methods(Static)
        function r = allTests(helper)
            import Test.*;

            communicator = helper.communicator();

            fprintf('testing stringToProxy... ');

            ref = ['initial:', helper.getTestEndpoint()];
            base = communicator.stringToProxy(ref);
            assert(~isempty(base));
            fprintf('ok\n');

            fprintf('testing checked cast... ');

            initial = InitialPrx.checkedCast(base);
            assert(~isempty(initial));
            assert(initial == base);
            fprintf('ok\n');

            fprintf('getting proxies for interface hierarchy... ');

            ia = initial.iaop();
            ib1 = initial.ib1op();
            ib2 = initial.ib2op();
            ic = initial.icop();
            assert(ia ~= ib1);
            assert(ia ~= ib2);
            assert(ia ~= ic);
            assert(ib1 ~= ic);
            assert(ib2 ~= ic);
            fprintf('ok\n');

            fprintf('invoking proxy operations on interface hierarchy... ');

            iao = [];
            ib1o = [];
            ib2o = [];
            ico = [];

            iao = ia.iaop(ia);
            assert(iao == ia);
            iao = ia.iaop(ib1);
            assert(iao == ib1);
            iao = ia.iaop(ib2);
            assert(iao == ib2);
            iao = ia.iaop(ic);
            assert(iao == ic);
            iao = ib1.iaop(ia);
            assert(iao == ia);
            iao = ib1.iaop(ib1);
            assert(iao == ib1);
            iao = ib1.iaop(ib2);
            assert(iao == ib2);
            iao = ib1.iaop(ic);
            assert(iao == ic);
            iao = ib2.iaop(ia);
            assert(iao == ia);
            iao = ib2.iaop(ib1);
            assert(iao == ib1);
            iao = ib2.iaop(ib2);
            assert(iao == ib2);
            iao = ib2.iaop(ic);
            assert(iao == ic);
            iao = ic.iaop(ia);
            assert(iao == ia);
            iao = ic.iaop(ib1);
            assert(iao == ib1);
            iao = ic.iaop(ib2);
            assert(iao == ib2);
            iao = ic.iaop(ic);
            assert(iao == ic);

            iao = ib1.ib1op(ib1);
            assert(iao == ib1);
            ib1o = ib1.ib1op(ib1);
            assert(ib1o == ib1);
            iao = ib1.ib1op(ic);
            assert(iao == ic);
            ib1o = ib1.ib1op(ic);
            assert(ib1o == ic);
            iao = ic.ib1op(ib1);
            assert(iao == ib1);
            ib1o = ic.ib1op(ib1);
            assert(ib1o == ib1);
            iao = ic.ib1op(ic);
            assert(iao == ic);
            ib1o = ic.ib1op(ic);
            assert(ib1o == ic);

            iao = ib2.ib2op(ib2);
            assert(iao == ib2);
            ib2o = ib2.ib2op(ib2);
            assert(ib2o == ib2);
            iao = ib2.ib2op(ic);
            assert(iao == ic);
            ib2o = ib2.ib2op(ic);
            assert(ib2o == ic);
            iao = ic.ib2op(ib2);
            assert(iao == ib2);
            ib2o = ic.ib2op(ib2);
            assert(ib2o == ib2);
            iao = ic.ib2op(ic);
            assert(iao == ic);
            ib2o = ic.ib2op(ic);
            assert(ib2o == ic);

            iao = ic.icop(ic);
            assert(iao == ic);
            ib1o = ic.icop(ic);
            assert(ib1o == ic);
            ib2o = ic.icop(ic);
            assert(ib2o == ic);
            ico = ic.icop(ic);
            assert(ico == ic);
            fprintf('ok\n');

            r = initial;
        end
    end
end
