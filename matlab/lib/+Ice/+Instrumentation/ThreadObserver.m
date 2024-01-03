% ThreadObserver   Summary of ThreadObserver
%
% The thread observer interface to instrument Ice threads. This can be threads from the Ice thread pool or utility
% threads used by the Ice core.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from Instrumentation.ice by slice2matlab version 3.7.10

classdef (Abstract) ThreadObserver < Ice.Instrumentation.Observer
    methods(Abstract)
        stateChanged(obj, oldState, newState)
    end
end
