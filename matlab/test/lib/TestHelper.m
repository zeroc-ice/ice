%{
**********************************************************************

Copyright (c) 2003-present ZeroC, Inc. All rights reserved.

**********************************************************************
%}

classdef TestHelper < handle
    methods
        function obj = TestHelper()
            obj.communicator_ = {};
        end

        function endpoint = getTestEndpoint(obj, varargin)
            num = 0;
            protocol = '';

            nargs = length(varargin);
            if nargs == 0
                properties = obj.communicator_.getProperties();
            else
                if isa(varargin{1}, 'Ice.Properties')
                    properties = varargin{1};
                    if nargs > 1
                        if isnumeric(varargin{2})
                            num = varargin{2};
                            if nargs > 2
                                protocol = varargin{3};
                            end
                        else
                            protocol = varargin{2};
                        end
                    end
                else
                    properties = obj.communicator_.getProperties();
                    if isnumeric(varargin{1})
                        num = varargin{1};
                        if nargs > 1
                            protocol = varargin{2};
                        end
                    else
                        protocol = varargin{1};
                    end
                end
            end

            if length(protocol) == 0
                protocol = properties.getPropertyWithDefault('Ice.Default.Protocol', 'default');
            end
            port = properties.getPropertyAsIntWithDefault('Test.BasePort', 12010) + num;
            endpoint = sprintf('%s -p %d', protocol, port);
        end

        function host = getTestHost(obj, varargin)
            nargs = length(varargin);
            if nargs == 0
                properties = obj.communicator_.getProperties();
            else
                properties = varargin{1};
            end
            host = properties.getPropertyWithDefault('Ice.Default.Host', '127.0.0.1');
        end

        function host = getTestProtocol(obj, varargin)
            nargs = length(varargin);
            if nargs == 0
                properties = obj.communicator_.getProperties();
            else
                properties = varargin{1};
            end
            host = properties.getPropertyWithDefault('Ice.Default.Protocol', 'tcp');
        end

        function port = getTestPort(obj, varargin)
            nargs = length(varargin);
            num = 0;
            if nargs == 0
                properties = obj.communicator_.getProperties();
            else
                if isa(varargin{1}, 'Ice.Properties')
                    properties = varargin{1};
                    if nargs > 1
                        num = varargin{2};
                    end
                else
                    num = varargin{1};
                end
            end
            port = properties.getPropertyAsIntWithDefault('Test.BasePort', 12010) + num;
        end

        function [properties, args] = createTestProperties(~, varargin)
            nargs = length(varargin);
            if nargs == 0
                args = {};
            else
                args = varargin{1};
            end
            [properties, args] = Ice.createProperties(args);
            args = properties.parseCommandLineOptions('Test', args);
        end

        function communicator = initialize(obj, varargin)
            nargs = length(varargin);
            if nargs > 0
                if isa(varargin{1}, 'Ice.InitializationData')
                    initData = varargin{1};
                else
                    initData = Ice.InitializationData();
                    if isa(varargin{1}, 'Ice.Properties')
                        initData.properties_ = varargin{1};
                    else
                        initData.properties_ = obj.createTestProperties(varargin{1});
                    end
                end
            end
            communicator = Ice.initialize(initData);
            if(isempty(obj.communicator_))
                obj.communicator_ = communicator;
            end
        end

        function communicator = communicator(obj)
            communicator = obj.communicator_;
        end

        function shutdown(obj)
            if(~isempty(obj.communicator_))
                obj.communicator_.shutdown();
            end
        end
    end
    properties(Access=private)
        communicator_ = {};
    end
end
