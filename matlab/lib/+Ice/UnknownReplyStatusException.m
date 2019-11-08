% UnknownReplyStatusException   Summary of UnknownReplyStatusException
%
% This exception indicates that an unknown reply status has been received.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef UnknownReplyStatusException < Ice.ProtocolException
    methods
        function obj = UnknownReplyStatusException(ice_exid, ice_exmsg, reason)
            if nargin <= 2
                reason = '';
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:UnknownReplyStatusException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.UnknownReplyStatusException';
            end
            obj = obj@Ice.ProtocolException(ice_exid, ice_exmsg, reason);
        end
        function id = ice_id(~)
            id = '::Ice::UnknownReplyStatusException';
        end
    end
end
