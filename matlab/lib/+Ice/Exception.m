classdef (Abstract) Exception < MException
    % Exception   Summary of Exception
    %
    % Base class for all Ice exceptions.
    %
    % Copyright (c) ZeroC, Inc. All rights reserved.

    methods
        function obj = Exception(varargin)
            obj = obj@MException(varargin{:});
        end
    end
end
