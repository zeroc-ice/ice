% MemoryLimitException   Summary of MemoryLimitException
%
% This exception is raised when Ice receives a request or reply
% message whose size exceeds the limit specified by the
% Ice.MessageSizeMax property.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef MemoryLimitException < Ice.MarshalException
    methods
        function obj = MemoryLimitException(ice_exid, ice_exmsg, reason)
            if nargin <= 2
                reason = '';
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:MemoryLimitException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.MemoryLimitException';
            end
            obj = obj@Ice.MarshalException(ice_exid, ice_exmsg, reason);
        end
        function id = ice_id(~)
            id = '::Ice::MemoryLimitException';
        end
    end
end
