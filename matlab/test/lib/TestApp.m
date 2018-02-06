%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef TestApp < handle
    methods(Static)
        function [initData, extraArgs] = createInitData(appName, args)
            initData = Ice.InitializationData();
            a = [appName, args]; % Insert appName as the first argument -- this becomes the value of Ice.ProgramName.
            [initData.properties_, extraArgs] = Ice.createProperties(a);
            extraArgs = initData.properties_.parseCommandLineOptions('Test', extraArgs);
        end
    end
    methods
        function obj = TestApp(communicator)
            obj.communicator_ = communicator;
            obj.properties_ = communicator.getProperties();
            obj.appName_ = obj.properties_.getProperty('Ice.ProgramName');
        end
        function r = appName(obj)
            r = obj.appName_;
        end
        function r = communicator(obj)
            r = obj.communicator_;
        end
        function initData = cloneInitData(obj)
            initData = Ice.InitializationData();
            initData.properties_ = obj.properties_.clone();
        end
        function r = getTestEndpoint(obj, num, prot)
            if nargin == 2
                prot = '';
            end
            r = TestApp.getTestEndpointWithProperties(obj.properties_, num, prot);
        end
        function r = getTestHost(obj)
            r = TestApp.getTestHostWithProperties(obj.properties_);
        end
        function r = getTestProtocol(obj)
            r = TestApp.getTestProtocolWithProperties(obj.properties_);
        end
        function r = getTestPort(obj, num)
            r = TestApp.getTestPortWithProperties(obj.properties_, num);
        end
    end
    methods(Static, Access=private)
        function r = getTestEndpointWithProperties(props, num, prot)
            if length(prot) == 0
                prot = props.getPropertyWithDefault('Ice.Default.Protocol', 'default');
            end

            basePort = props.getPropertyAsIntWithDefault('Test.BasePort', 12010);

            r = sprintf('%s -p %d', prot, basePort + num);
        end
        function r = getTestHostWithProperties(props)
            r = props.getPropertyWithDefault('Ice.Default.Host', '127.0.0.1');
        end
        function r = getTestProtocolWithProperties(props)
            r = props.getPropertyWithDefault('Ice.Default.Protocol', 'tcp');
        end
        function r = getTestPortWithProperties(props, num)
            r = props.getPropertyAsIntWithDefault('Test.BasePort', 12010) + num;
        end
    end
    properties(Access=private)
        appName_
        communicator_
        properties_
    end
end
