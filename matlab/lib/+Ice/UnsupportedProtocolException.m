% UnsupportedProtocolException   Summary of UnsupportedProtocolException
%
% This exception indicates an unsupported protocol version.
%
% UnsupportedProtocolException Properties:
%   bad - The version of the unsupported protocol.
%   supported - The version of the protocol that is supported.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef UnsupportedProtocolException < Ice.ProtocolException
    properties
        % bad - The version of the unsupported protocol.
        bad Ice.ProtocolVersion
        % supported - The version of the protocol that is supported.
        supported Ice.ProtocolVersion
    end
    methods
        function obj = UnsupportedProtocolException(ice_exid, ice_exmsg, reason, bad, supported)
            if nargin <= 2
                reason = '';
                bad = Ice.ProtocolVersion();
                supported = Ice.ProtocolVersion();
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:UnsupportedProtocolException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.UnsupportedProtocolException';
            end
            obj = obj@Ice.ProtocolException(ice_exid, ice_exmsg, reason);
            obj.bad = bad;
            obj.supported = supported;
        end
        function id = ice_id(~)
            id = '::Ice::UnsupportedProtocolException';
        end
    end
end
