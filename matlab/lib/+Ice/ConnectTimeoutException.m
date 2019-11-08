% ConnectTimeoutException   Summary of ConnectTimeoutException
%
% This exception indicates a connection establishment timeout condition.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef ConnectTimeoutException < Ice.TimeoutException
    methods
        function obj = ConnectTimeoutException(ice_exid, ice_exmsg)
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:ConnectTimeoutException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.ConnectTimeoutException';
            end
            obj = obj@Ice.TimeoutException(ice_exid, ice_exmsg);
        end
        function id = ice_id(~)
            id = '::Ice::ConnectTimeoutException';
        end
    end
end
