% CompressionException   Summary of CompressionException
%
% This exception indicates a problem with compressing or uncompressing data.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef CompressionException < Ice.ProtocolException
    methods
        function obj = CompressionException(ice_exid, ice_exmsg, reason)
            if nargin <= 2
                reason = '';
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:CompressionException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.CompressionException';
            end
            obj = obj@Ice.ProtocolException(ice_exid, ice_exmsg, reason);
        end
        function id = ice_id(~)
            id = '::Ice::CompressionException';
        end
    end
end
