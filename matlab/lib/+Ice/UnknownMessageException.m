% UnknownMessageException   Summary of UnknownMessageException
%
% This exception indicates that an unknown protocol message has been received.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef UnknownMessageException < Ice.ProtocolException
    methods
        function obj = UnknownMessageException(ice_exid, ice_exmsg, reason)
            if nargin <= 2
                reason = '';
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:UnknownMessageException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.UnknownMessageException';
            end
            obj = obj@Ice.ProtocolException(ice_exid, ice_exmsg, reason);
        end
        function id = ice_id(~)
            id = '::Ice::UnknownMessageException';
        end
    end
end
