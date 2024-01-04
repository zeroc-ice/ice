% HeartbeatCallback   Summary of HeartbeatCallback
%
% An application can implement this interface to receive notifications when a connection receives a heartbeat
% message.
%
% See also Ice.Connection.setHeartbeatCallback

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from Connection.ice by slice2matlab version 3.7.10

classdef (Abstract) HeartbeatCallback < handle
    methods(Abstract)
        heartbeat(obj, con)
    end
end
