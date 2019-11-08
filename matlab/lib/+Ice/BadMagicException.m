% BadMagicException   Summary of BadMagicException
%
% This exception indicates that a message did not start with the expected
% magic number ('I', 'c', 'e', 'P').
%
% BadMagicException Properties:
%   badMagic - A sequence containing the first four bytes of the incorrect message.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef BadMagicException < Ice.ProtocolException
    properties
        % badMagic - A sequence containing the first four bytes of the incorrect message.
        badMagic
    end
    methods
        function obj = BadMagicException(ice_exid, ice_exmsg, reason, badMagic)
            if nargin <= 2
                reason = '';
                badMagic = [];
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:BadMagicException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.BadMagicException';
            end
            obj = obj@Ice.ProtocolException(ice_exid, ice_exmsg, reason);
            obj.badMagic = badMagic;
        end
        function id = ice_id(~)
            id = '::Ice::BadMagicException';
        end
    end
end
