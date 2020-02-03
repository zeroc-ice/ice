% ResponseSentException   Summary of ResponseSentException
%
% Indicates that the response to a request has already been sent;
% re-dispatching such a request is not possible.

% Copyright (c) ZeroC, Inc. All rights reserved.

classdef ResponseSentException < Ice.LocalException
    methods
        function obj = ResponseSentException(ice_exid, ice_exmsg)
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:ResponseSentException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.ResponseSentException';
            end
            obj = obj@Ice.LocalException(ice_exid, ice_exmsg);
        end
        function id = ice_id(~)
            id = '::Ice::ResponseSentException';
        end
    end
end
