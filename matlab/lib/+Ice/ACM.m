% ACM   Summary of ACM
%
% A collection of Active Connection Management configuration settings.
%
% ACM Properties:
%   timeout - A timeout value in seconds.
%   close - The close semantics.
%   heartbeat - The heartbeat semantics.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from Connection.ice by slice2matlab version 3.7.3

classdef ACM
    properties
        % timeout - A timeout value in seconds.
        timeout int32
        % close - The close semantics.
        close Ice.ACMClose
        % heartbeat - The heartbeat semantics.
        heartbeat Ice.ACMHeartbeat
    end
    methods
        function obj = ACM(timeout, close, heartbeat)
            if nargin > 0
                obj.timeout = timeout;
                obj.close = close;
                obj.heartbeat = heartbeat;
            else
                obj.timeout = 0;
                obj.close = Ice.ACMClose.CloseOff;
                obj.heartbeat = Ice.ACMHeartbeat.HeartbeatOff;
            end
        end
        function r = eq(obj, other)
            r = isequal(obj, other);
        end
        function r = ne(obj, other)
            r = ~isequal(obj, other);
        end
    end
end
