% MarshalException   Summary of MarshalException
%
% This exception is raised for errors during marshaling or unmarshaling data.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef MarshalException < Ice.ProtocolException
    methods
        function obj = MarshalException(ice_exid, ice_exmsg, reason)
            if nargin <= 2
                reason = '';
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:MarshalException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.MarshalException';
            end
            obj = obj@Ice.ProtocolException(ice_exid, ice_exmsg, reason);
        end
        function id = ice_id(~)
            id = '::Ice::MarshalException';
        end
    end
end
