% CloseCallback   Summary of CloseCallback
%
% An application can implement this interface to receive notifications when a connection closes.
%
% See also Ice.Connection.setCloseCallback

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from Connection.ice by slice2matlab version 3.8.50

classdef (Abstract) CloseCallback < handle
    methods(Abstract)
        closed(obj, con)
    end
end
