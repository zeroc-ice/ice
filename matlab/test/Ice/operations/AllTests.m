%
% Copyright (c) ZeroC, Inc. All rights reserved.
%

classdef AllTests
    methods(Static)
        function r = allTests(helper)
            import Test.*;
            import M.*;

            communicator = helper.communicator();
            ref = ['test:', helper.getTestEndpoint()];
            base = communicator.stringToProxy(ref);
            cl = MyClassPrx.checkedCast(base);
            derived = MyDerivedClassPrx.checkedCast(cl);

            bprx = BPrx.checkedCast(communicator.stringToProxy(['b:', helper.getTestEndpoint()]));

            fprintf('testing twoway operations... ');
            Twoways.twoways(helper, cl, bprx);
            Twoways.twoways(helper, derived, bprx);
            derived.opDerived();
            fprintf('ok\n');

            fprintf('testing oneway operations... ');
            Oneways.oneways(cl);
            fprintf('ok\n');

            fprintf('testing twoway operations with AMI... ');
            TwowaysAMI.twowaysAMI(helper, cl);
            TwowaysAMI.twowaysAMI(helper, derived);
            fprintf('ok\n');

            fprintf('testing oneway operations with AMI... ');
            OnewaysAMI.onewaysAMI(cl);
            fprintf('ok\n');

            fprintf('testing batch oneway operations... ');
            BatchOneways.batchOneways(helper, cl);
            BatchOneways.batchOneways(helper, derived);
            fprintf('ok\n');

            fprintf('testing batch AMI oneway operations... ');
            BatchOnewaysAMI.batchOneways(cl);
            BatchOnewaysAMI.batchOneways(derived);
            fprintf('ok\n');

            r = cl;
        end
    end
end
