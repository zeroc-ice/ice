% UnsupportedEncodingException   Summary of UnsupportedEncodingException
%
% This exception indicates an unsupported data encoding version.
%
% UnsupportedEncodingException Properties:
%   bad - The version of the unsupported encoding.
%   supported - The version of the encoding that is supported.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef UnsupportedEncodingException < Ice.ProtocolException
    properties
        % bad - The version of the unsupported encoding.
        bad Ice.EncodingVersion
        % supported - The version of the encoding that is supported.
        supported Ice.EncodingVersion
    end
    methods
        function obj = UnsupportedEncodingException(ice_exid, ice_exmsg, reason, bad, supported)
            if nargin <= 2
                reason = '';
                bad = Ice.EncodingVersion();
                supported = Ice.EncodingVersion();
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:UnsupportedEncodingException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.UnsupportedEncodingException';
            end
            obj = obj@Ice.ProtocolException(ice_exid, ice_exmsg, reason);
            obj.bad = bad;
            obj.supported = supported;
        end
        function id = ice_id(~)
            id = '::Ice::UnsupportedEncodingException';
        end
    end
end
