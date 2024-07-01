% InitializationException   Summary of InitializationException
%
% This exception is raised when a failure occurs during initialization.
%
% InitializationException Properties:
%   reason - The reason for the failure.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.10

classdef InitializationException < Ice.LocalException
    properties
        % reason - The reason for the failure.
        reason char
    end
    methods
        function obj = InitializationException(errID, msg, reason)
            if nargin <= 2
                reason = '';
            end
            if nargin == 0 || isempty(errID)
                errID = 'Ice:InitializationException';
            end
            if nargin < 2 || isempty(msg)
                msg = 'Ice.InitializationException';
            end
            obj = obj@Ice.LocalException(errID, msg);
            obj.reason = reason;
        end
    end
end
