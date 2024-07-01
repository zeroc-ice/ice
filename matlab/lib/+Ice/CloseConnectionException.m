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
        function obj = CloseConnectionException(errID, msg, reason)
            if nargin <= 2
                reason = '';
            end
            if nargin == 0 || isempty(errID)
                errID = 'Ice:CloseConnectionException';
            end
            if nargin < 2 || isempty(msg)
                msg = 'Ice.CloseConnectionException';
            end
            obj = obj@Ice.ProtocolException(errID, msg, reason);
        end
    end
end
