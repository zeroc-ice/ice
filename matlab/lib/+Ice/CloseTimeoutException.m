% CloseTimeoutException   Summary of CloseTimeoutException
%
% This exception indicates a connection closure timeout condition.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.10

classdef CloseTimeoutException < Ice.TimeoutException
    methods
        function obj = CloseTimeoutException(errID, msg)
            if nargin == 0 || isempty(errID)
                errID = 'Ice:CloseTimeoutException';
            end
            if nargin < 2 || isempty(msg)
                msg = 'Ice.CloseTimeoutException';
            end
            obj = obj@Ice.TimeoutException(errID, msg);
        end
    end
end
