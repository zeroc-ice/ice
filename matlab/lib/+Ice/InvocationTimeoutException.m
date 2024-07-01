% InvocationTimeoutException   Summary of InvocationTimeoutException
%
% This exception indicates that an invocation failed because it timed out.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.10

classdef InvocationTimeoutException < Ice.TimeoutException
    methods
        function obj = InvocationTimeoutException(errID, msg)
            if nargin == 0 || isempty(errID)
                errID = 'Ice:InvocationTimeoutException';
            end
            if nargin < 2 || isempty(msg)
                msg = 'Ice.InvocationTimeoutException';
            end
            obj = obj@Ice.TimeoutException(errID, msg);
        end
    end
end
