%{
**********************************************************************

Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef AllTests
    methods(Static)
        function r = allTests(app)
            import Test.*;

            communicator = app.communicator();
            ref = ['test:', app.getTestEndpoint(0)];
            base = communicator.stringToProxy(ref);
            cl = MyClassPrx.checkedCast(base);
            derived = MyDerivedClassPrx.checkedCast(cl);

            fprintf('testing twoway operations... ');
            Twoways.twoways(app, cl);
            Twoways.twoways(app, derived);
            derived.opDerived();
            fprintf('ok\n');

            fprintf('testing oneway operations... ');
            Oneways.oneways(app, cl);
            fprintf('ok\n');

            fprintf('testing twoway operations with AMI... ');
            TwowaysAMI.twowaysAMI(app, cl);
            TwowaysAMI.twowaysAMI(app, derived);
            fprintf('ok\n');

            fprintf('testing oneway operations with AMI... ');
            OnewaysAMI.onewaysAMI(app, cl);
            fprintf('ok\n');

            fprintf('testing batch oneway operations... ');
            BatchOneways.batchOneways(app, cl);
            BatchOneways.batchOneways(app, derived);
            fprintf('ok\n');

            fprintf('testing batch AMI oneway operations... ');
            BatchOnewaysAMI.batchOneways(app, cl);
            BatchOnewaysAMI.batchOneways(app, derived);
            fprintf('ok\n');

            r = cl;
        end
    end
end
