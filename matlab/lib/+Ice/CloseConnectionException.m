% CloseConnectionException   Summary of CloseConnectionException
%
% This exception indicates that the connection has been gracefully shut down by the server. The operation call that
% caused this exception has not been executed by the server. In most cases you will not get this exception, because
% the client will automatically retry the operation call in case the server shut down the connection. However, if
% upon retry the server shuts down the connection again, and the retry limit has been reached, then this exception is
% propagated to the application code.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.10

classdef CloseConnectionException < Ice.ProtocolException
    methods
        function obj = CloseConnectionException(ice_exid, ice_exmsg, reason)
            if nargin <= 2
                reason = '';
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:CloseConnectionException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.CloseConnectionException';
            end
            obj = obj@Ice.ProtocolException(ice_exid, ice_exmsg, reason);
        end
        function id = ice_id(~)
            id = '::Ice::CloseConnectionException';
        end
    end
end
