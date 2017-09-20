%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef Client < Application
    methods
        function r = run(obj, args)
            communicator = obj.communicator();

            %
            % Remote tests
            %
            vfm = communicator.getValueFactoryManager();
            vfm.add(@(id) BI(), test.Ice.objects.Test.B.ice_staticId());
            vfm.add(@(id) CI(), test.Ice.objects.Test.C.ice_staticId());
            vfm.add(@(id) DI(), test.Ice.objects.Test.D.ice_staticId());
            vfm.add(@(id) EI(), test.Ice.objects.Test.E.ice_staticId());
            vfm.add(@(id) FI(), test.Ice.objects.Test.F.ice_staticId());
            vfm.add(@(id) II(), test.Ice.objects.Test.IPrx.ice_staticId());
            vfm.add(@(id) JI(), test.Ice.objects.Test.JPrx.ice_staticId());
            vfm.add(@(id) HI(), test.Ice.objects.Test.H.ice_staticId());

            initial = AllTests.allTests(obj);
            initial.shutdown();

            %
            % Local tests
            %
            vfm.add(@(id) CB1I(), test.Ice.objects.LocalTest.CB1.ice_staticId());
            vfm.add(@(id) CB2I(), test.Ice.objects.LocalTest.CB2.ice_staticId());
            vfm.add(@(id) CB3I(), test.Ice.objects.LocalTest.CB3.ice_staticId());
            vfm.add(@(id) CB4I(), test.Ice.objects.LocalTest.CB4.ice_staticId());
            vfm.add(@(id) CB5I(), test.Ice.objects.LocalTest.CB5.ice_staticId());
            vfm.add(@(id) CB6I(), test.Ice.objects.LocalTest.CB6.ice_staticId());
            vfm.add(@(id) CB7I(), test.Ice.objects.LocalTest.CB7.ice_staticId());
            vfm.add(@(id) CB8I(), test.Ice.objects.LocalTest.CB8.ice_staticId());
            LocalTests.localTests(obj);

            r = 0;
        end
    end
    methods(Access=protected)
        function [r, remArgs] = getInitData(obj, args)
            [initData, remArgs] = getInitData@Application(obj, args);
            initData.properties_.setProperty('Ice.Package.Test', 'test.Ice.objects');
            initData.properties_.setProperty('Ice.Package.LocalTest', 'test.Ice.objects');
            r = initData;
        end
    end
    methods(Static)
        function start(args)
            addpath('generated');
            if ~libisloaded('icematlab')
                loadlibrary('icematlab')
            end
            c = Client();
            status = c.main('Client', args);
        end
    end
end
