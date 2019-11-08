% UnmarshalOutOfBoundsException   Summary of UnmarshalOutOfBoundsException
%
% This exception is raised if an out-of-bounds condition occurs during unmarshaling.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef UnmarshalOutOfBoundsException < Ice.MarshalException
    methods
        function obj = UnmarshalOutOfBoundsException(ice_exid, ice_exmsg, reason)
            if nargin <= 2
                reason = '';
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:UnmarshalOutOfBoundsException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.UnmarshalOutOfBoundsException';
            end
            obj = obj@Ice.MarshalException(ice_exid, ice_exmsg, reason);
        end
        function id = ice_id(~)
            id = '::Ice::UnmarshalOutOfBoundsException';
        end
    end
end
