%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef (Abstract) Application < handle
    methods(Sealed)
        %
        % main() initializes the Communicator, calls run(), and destroys
        % the Communicator upon return from run(). It thereby handles
        % all exceptions properly, i.e., error messages are printed
        % if exceptions propagate to main(), and the Communicator is
        % always destroyed, regardless of exceptions.
        %
        function r = main(obj, appName, varargin)
            if ~isempty(obj.communicator_)
                printf('%s: only one instance of the Application class can be used', appName);
                r = 1;
                return;
            end

            obj.testName_ = appName;

            args = {};
            initializationData = [];

            if length(varargin) >= 1
                args = varargin{1};
            end
            if length(varargin) >= 2
                initializationData = varargin{2};
            end

            %
            % We parse the properties here to extract Ice.ProgramName.
            %
            if isempty(initializationData)
                [initializationData, args] = obj.getInitData(args);
            end

            if ~isempty(initializationData)
                initData = initializationData.clone();
            else
                initData = Ice.InitializationData();
            end

            [initData.properties_, args] = Ice.createProperties(args, initData.properties_);

            %TBD
            % If the process logger is the default logger, we replace it with a
            % a logger that uses the program name as the prefix.
            %
            %if(Util.getProcessLogger() instanceof LoggerI)
            %{
            %    Util.setProcessLogger(new LoggerI(initData.properties_.getProperty('Ice.ProgramName'), ''));
            %}

            status = 0;

            try
                [communicator, args] = Ice.initialize(args, initData);
                obj.communicator_ = communicator;

                status = obj.run(args);
            catch ex
                fprintf('%s: caught %s', obj.testName_, ex.identifier);
                disp(getReport(ex, 'extended'));
                status = 1;
            end
            obj.communicator_.destroy();
            obj.communicator_ = [];

            r = status;
        end
        function r = initialize(obj, varargin)
            assert(length(varargin) <= 1);
            if length(varargin) == 0
                r = Ice.initialize();
            else
                r = Ice.initialize(varargin{1});
            end
        end
        function setLogger(obj, logger)
            obj.logger_ = logger;
        end
        function r = appName(obj)
            r = obj.testName_;
        end
        function r = communicator(obj)
            r = obj.communicator_;
        end
        function r = createInitializationData(obj)
            initData = Ice.InitializationData();
            initData.logger = obj.logger_;
            r = initData;
        end
        function r = getTestEndpoint(obj, num, prot)
            r = Application.getTestEndpointWithProperties(obj.communicator_.getProperties(), num, prot);
        end
        function r = getTestHost(obj)
            r = Application.getTestHostWithProperties(obj.communicator_.getProperties());
        end
        function r = getTestProtocol(obj)
            r = Application.getTestProtocolWithProperties(obj.communicator_.getProperties());
        end
        function r = getTestPort(obj, num)
            r = Application.getTestPortWithProperties(obj.communicator_.getProperties(), num);
        end
    end
    methods(Static)
        function r = getTestEndpointWithProperties(props, num, prot)
            protocol = prot;
            if length(protocol) == 0
                protocol = props.getPropertyWithDefault('Ice.Default.Protocol', 'default');
            end

            basePort = props.getPropertyAsIntWithDefault('Test.BasePort', 12010);

            r = sprintf('%s -p %d', protocol, basePort + num);
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
    methods(Abstract)
        r = run(obj, args)
    end
    methods(Access=protected)
        %
        % Hook to override the initialization data. This hook is
        % necessary because some properties must be set prior to
        % communicator initialization.
        %
        function [initData, remArgs] = getInitData(obj, args)
            initData = obj.createInitializationData();
            [initData.properties_, remArgs] = Ice.createProperties(args);
            remArgs = initData.properties_.parseCommandLineOptions('Test', remArgs);
        end
    end
    properties(Access=private)
        testName_
        communicator_
        logger_
    end
end
