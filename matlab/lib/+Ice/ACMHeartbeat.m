% ACMHeartbeat   Summary of ACMHeartbeat
%
% Specifies the heartbeat semantics for Active Connection Management.
%
% ACMHeartbeat Properties:
%   HeartbeatOff - Disables heartbeats.
%   HeartbeatOnDispatch - Send a heartbeat at regular intervals if the connection is idle and only if there are pending dispatch.
%   HeartbeatOnIdle - Send a heartbeat at regular intervals when the connection is idle.
%   HeartbeatAlways - Send a heartbeat at regular intervals until the connection is closed.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from Connection.ice by slice2matlab version 3.7.3

classdef ACMHeartbeat < uint8
    enumeration
        % Disables heartbeats.
        HeartbeatOff (0)
        % Send a heartbeat at regular intervals if the connection is idle and only if there are pending dispatch.
        HeartbeatOnDispatch (1)
        % Send a heartbeat at regular intervals when the connection is idle.
        HeartbeatOnIdle (2)
        % Send a heartbeat at regular intervals until the connection is closed.
        HeartbeatAlways (3)
    end
    methods(Static)
        function r = ice_getValue(v)
            switch v
                case 0
                    r = Ice.ACMHeartbeat.HeartbeatOff;
                case 1
                    r = Ice.ACMHeartbeat.HeartbeatOnDispatch;
                case 2
                    r = Ice.ACMHeartbeat.HeartbeatOnIdle;
                case 3
                    r = Ice.ACMHeartbeat.HeartbeatAlways;
                otherwise
                    throw(Ice.MarshalException('', '', sprintf('enumerator value %d is out of range', v)));
            end
        end
    end
end
