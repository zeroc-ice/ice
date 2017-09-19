%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef CommunicatorHolder < handle
    methods
        function obj = CommunicatorHolder(varargin)
            [obj.communicator, obj.args] = Ice.initialize(varargin{:});
        end
        function delete(obj)
            if ~isempty(obj.communicator)
                obj.communicator.destroy()
            end
        end
        function r = release(obj)
            r = obj.communicator;
            obj.communicator = [];
        end
    end
    properties
        communicator
        args
    end
end
