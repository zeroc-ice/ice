% ConnectionObserver   Summary of ConnectionObserver
%
% The connection observer interface to instrument Ice connections.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from Instrumentation.ice by slice2matlab version 3.7.10

classdef (Abstract) ConnectionObserver < Ice.Instrumentation.Observer
    methods(Abstract)
        sentBytes(obj, num)
        receivedBytes(obj, num)
    end
end
