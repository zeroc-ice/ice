classdef (Abstract) LocalException < Ice.Exception
    % LocalException   Summary of LocalException
    %
    % Base class for all Ice run-time exceptions.

    % Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.

    methods
        function obj = LocalException(id, msg)
            obj = obj@Ice.Exception(id, msg)
        end
    end
end
