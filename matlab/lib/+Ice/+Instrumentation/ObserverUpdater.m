% ObserverUpdater   Summary of ObserverUpdater
%
% The observer updater interface. This interface is implemented by the Ice run-time and an instance of this interface
% is provided by the Ice communicator on initialization to the CommunicatorObserver object set with the
% communicator initialization data. The Ice communicator calls CommunicatorObserver.setObserverUpdater to
% provide the observer updater.
% This interface can be used by add-ins implementing the CommunicatorObserver interface to update the
% observers of connections and threads.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from Instrumentation.ice by slice2matlab version 3.7.10

classdef (Abstract) ObserverUpdater < handle
    methods(Abstract)
        updateConnectionObservers(obj)
        updateThreadObservers(obj)
    end
end
