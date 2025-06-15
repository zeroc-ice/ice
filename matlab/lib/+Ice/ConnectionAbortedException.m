% ConnectionAbortedException   Summary of ConnectionAbortedException
%
% This exception indicates that the connection was closed forcefully.
%
% ConnectionAbortedException Properties:
%   closedByApplication - True if the connection was closed by the application, false if it was closed by the Ice runtime.

% Copyright (c) ZeroC, Inc.

classdef ConnectionAbortedException < Ice.LocalException
    properties
        % ConnectionAbortedException - True if the connection was closed by the application, false if it was closed by the Ice runtime.
        closedByApplication (1, 1) logical = false
    end
    methods
        function obj = ConnectionAbortedException(closedByApplication, errID, what)
            if nargin == 0 % default constructor
                superArgs = {};
            else
                assert(nargin == 3, 'Invalid number of arguments');
                superArgs = {errID, what};
            end
            obj@Ice.LocalException(superArgs{:});
            if nargin > 0
                obj.closedByApplication = closedByApplication;
            end
        end
    end
end
