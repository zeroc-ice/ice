% CloseTimeoutException   Summary of CloseTimeoutException
%
% This exception indicates a connection closure timeout condition.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.10

classdef CloseTimeoutException < Ice.TimeoutException
    methods
        function obj = CloseTimeoutException(ice_exid, ice_exmsg)
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:CloseTimeoutException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.CloseTimeoutException';
            end
            obj = obj@Ice.TimeoutException(ice_exid, ice_exmsg);
        end
        function id = ice_id(~)
            id = '::Ice::CloseTimeoutException';
        end
    end
end
