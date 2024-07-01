classdef (Abstract) LocalException < Ice.Exception
    % LocalException   Summary of LocalException
    %
    % Base class for all Ice exceptions not defined in Slice.
    %
    % Copyright (c) ZeroC, Inc. All rights reserved.

    methods
        function obj = LocalException(varargin)
            obj = obj@Ice.Exception(varargin{:});
        end
    end
end
