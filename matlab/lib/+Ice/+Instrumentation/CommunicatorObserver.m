% CommunicatorObserver   Summary of CommunicatorObserver
%
% The communicator observer interface used by the Ice run-time to obtain and update observers for its observable
% objects. This interface should be implemented by add-ins that wish to observe Ice objects in order to collect
% statistics. An instance of this interface can be provided to the Ice run-time through the Ice communicator
% initialization data.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from Instrumentation.ice by slice2matlab version 3.7.10

classdef (Abstract) CommunicatorObserver < handle
    methods(Abstract)
        result = getConnectionEstablishmentObserver(obj, endpt, connector)
        result = getEndpointLookupObserver(obj, endpt)
        result = getConnectionObserver(obj, c, e, s, o)
        result = getThreadObserver(obj, parent, id, s, o)
        result = getInvocationObserver(obj, prx, operation, ctx)
        result = getDispatchObserver(obj, c, size)
        setObserverUpdater(obj, updater)
    end
end
