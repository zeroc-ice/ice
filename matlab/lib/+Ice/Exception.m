classdef (Abstract) Exception < MException
    % Exception   Summary of Exception
    %
    % Base class for Ice local and system exceptions.

    % Copyright (c) ZeroC, Inc. All rights reserved.

    methods(Abstract)
        ice_id(obj)
    end
    methods
        function obj = Exception(id, msg)
            obj = obj@MException(id, msg)
        end
    end
end
