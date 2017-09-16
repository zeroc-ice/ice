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
            vfm = communicator.getValueFactoryManager();
            vfm.add(@(id) BI(), Test.B.ice_staticId());
            vfm.add(@(id) CI(), Test.C.ice_staticId());
            vfm.add(@(id) DI(), Test.D.ice_staticId());
            vfm.add(@(id) EI(), Test.E.ice_staticId());
            vfm.add(@(id) FI(), Test.F.ice_staticId());
            vfm.add(@(id) II(), Test.IPrx.ice_staticId());
            vfm.add(@(id) JI(), Test.JPrx.ice_staticId());
            vfm.add(@(id) HI(), Test.H.ice_staticId());

            initial = AllTests.allTests(obj);
            initial.shutdown();
            r = 0;
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
