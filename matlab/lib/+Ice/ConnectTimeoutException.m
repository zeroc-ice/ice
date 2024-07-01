% ConnectTimeoutException   Summary of ConnectTimeoutException
%
% This exception indicates a connection establishment timeout condition.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.10

classdef ConnectTimeoutException < Ice.TimeoutException
    methods
        function obj = ConnectTimeoutException(errID, msg)
            if nargin == 0 || isempty(errID)
                errID = 'Ice:ConnectTimeoutException';
            end
            if nargin < 2 || isempty(msg)
                msg = 'Ice.ConnectTimeoutException';
            end
            obj = obj@Ice.TimeoutException(errID, msg);
        end
    end
end
