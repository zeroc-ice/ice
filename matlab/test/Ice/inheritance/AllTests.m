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
            import Test.*;

            communicator = app.communicator();

            fprintf('testing stringToProxy... ');

            ref = ['initial:', app.getTestEndpoint(0)];
            base = communicator.stringToProxy(ref);
            assert(~isempty(base));
            fprintf('ok\n');

            fprintf('testing checked cast... ');

            initial = InitialPrx.checkedCast(base);
            assert(~isempty(initial));
            assert(initial == base);
            fprintf('ok\n');

            fprintf('getting proxies for class hierarchy... ');

            ca = initial.caop();
            cb = initial.cbop();
            cc = initial.ccop();
            cd = initial.cdop();
            assert(ca ~= cb);
            assert(ca ~= cc);
            assert(ca ~= cd);
            assert(cb ~= cc);
            assert(cb ~= cd);
            assert(cc ~= cd);
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

            fprintf('invoking proxy operations on class hierarchy... ');

            cao = [];
            cbo = [];
            cco = [];

            cao = ca.caop(ca);
            assert(cao == ca);
            cao = ca.caop(cb);
            assert(cao == cb);
            cao = ca.caop(cc);
            assert(cao == cc);
            cao = cb.caop(ca);
            assert(cao == ca);
            cao = cb.caop(cb);
            assert(cao == cb);
            cao = cb.caop(cc);
            assert(cao == cc);
            cao = cc.caop(ca);
            assert(cao == ca);
            cao = cc.caop(cb);
            assert(cao == cb);
            cao = cc.caop(cc);
            assert(cao == cc);

            cao = cb.cbop(cb);
            assert(cao == cb);
            cbo = cb.cbop(cb);
            assert(cbo == cb);
            cao = cb.cbop(cc);
            assert(cao == cc);
            cbo = cb.cbop(cc);
            assert(cbo == cc);
            cao = cc.cbop(cb);
            assert(cao == cb);
            cbo = cc.cbop(cb);
            assert(cbo == cb);
            cao = cc.cbop(cc);
            assert(cao == cc);
            cbo = cc.cbop(cc);
            assert(cbo == cc);

            cao = cc.ccop(cc);
            assert(cao == cc);
            cbo = cc.ccop(cc);
            assert(cbo == cc);
            cco = cc.ccop(cc);
            assert(cco == cc);
            fprintf('ok\n');

            fprintf('ditto, but for interface hierarchy... ');

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

            fprintf('ditto, but for class implementing interfaces... ');

            cao = cd.caop(cd);
            assert(cao == cd);
            cbo = cd.cbop(cd);
            assert(cbo == cd);
            cco = cd.ccop(cd);
            assert(cco == cd);

            iao = cd.iaop(cd);
            assert(iao == cd);
            ib1o = cd.ib1op(cd);
            assert(ib1o == cd);
            ib2o = cd.ib2op(cd);
            assert(ib2o == cd);

            cao = cd.cdop(cd);
            assert(cao == cd);
            cbo = cd.cdop(cd);
            assert(cbo == cd);
            cco = cd.cdop(cd);
            assert(cco == cd);

            iao = cd.cdop(cd);
            assert(iao == cd);
            ib1o = cd.cdop(cd);
            assert(ib1o == cd);
            ib2o = cd.cdop(cd);
            assert(ib2o == cd);
            fprintf('ok\n');

            r = initial;
        end
    end
end
