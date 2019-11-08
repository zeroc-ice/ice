% ProxyUnmarshalException   Summary of ProxyUnmarshalException
%
% This exception is raised if inconsistent data is received while unmarshaling a proxy.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef ProxyUnmarshalException < Ice.MarshalException
    methods
        function obj = ProxyUnmarshalException(ice_exid, ice_exmsg, reason)
            if nargin <= 2
                reason = '';
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:ProxyUnmarshalException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.ProxyUnmarshalException';
            end
            obj = obj@Ice.MarshalException(ice_exid, ice_exmsg, reason);
        end
        function id = ice_id(~)
            id = '::Ice::ProxyUnmarshalException';
        end
    end
end
