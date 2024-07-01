% TimeoutException   Summary of TimeoutException
%
% This exception indicates a timeout condition.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.10

classdef TimeoutException < Ice.LocalException
    methods
        function obj = TimeoutException(errID, msg)
            if nargin == 0 || isempty(errID)
                errID = 'Ice:TimeoutException';
            end
            if nargin < 2 || isempty(msg)
                msg = 'Ice.TimeoutException';
            end
            obj = obj@Ice.LocalException(errID, msg);
        end
    end
end
