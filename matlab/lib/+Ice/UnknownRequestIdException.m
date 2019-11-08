% UnknownRequestIdException   Summary of UnknownRequestIdException
%
% This exception indicates that a response for an unknown request ID has been
% received.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef UnknownRequestIdException < Ice.ProtocolException
    methods
        function obj = UnknownRequestIdException(ice_exid, ice_exmsg, reason)
            if nargin <= 2
                reason = '';
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:UnknownRequestIdException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.UnknownRequestIdException';
            end
            obj = obj@Ice.ProtocolException(ice_exid, ice_exmsg, reason);
        end
        function id = ice_id(~)
            id = '::Ice::UnknownRequestIdException';
        end
    end
end
