% ConnectionNotValidatedException   Summary of ConnectionNotValidatedException
%
% This exception is raised if a message is received over a connection
% that is not yet validated.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef ConnectionNotValidatedException < Ice.ProtocolException
    methods
        function obj = ConnectionNotValidatedException(ice_exid, ice_exmsg, reason)
            if nargin <= 2
                reason = '';
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:ConnectionNotValidatedException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.ConnectionNotValidatedException';
            end
            obj = obj@Ice.ProtocolException(ice_exid, ice_exmsg, reason);
        end
        function id = ice_id(~)
            id = '::Ice::ConnectionNotValidatedException';
        end
    end
end
