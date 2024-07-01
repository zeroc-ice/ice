classdef (Abstract) LocalException < Ice.Exception
    % LocalException   Summary of LocalException
    %
    % Base class for all Ice exceptions not defined in Slice.
    %
    % Copyright (c) ZeroC, Inc. All rights reserved.

    methods(Abstract)
        ice_id(obj)
    end
    methods
        function obj = LocalException(id, msg)
            obj = obj@Ice.Exception(id, msg);
        end
    end
end
