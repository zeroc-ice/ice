% InvocationTimeoutException   Summary of InvocationTimeoutException
%
% This exception indicates that an invocation failed because it timed
% out.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef InvocationTimeoutException < Ice.TimeoutException
    methods
        function obj = InvocationTimeoutException(ice_exid, ice_exmsg)
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:InvocationTimeoutException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.InvocationTimeoutException';
            end
            obj = obj@Ice.TimeoutException(ice_exid, ice_exmsg);
        end
        function id = ice_id(~)
            id = '::Ice::InvocationTimeoutException';
        end
    end
end
