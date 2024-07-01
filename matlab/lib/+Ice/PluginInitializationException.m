% PluginInitializationException   Summary of PluginInitializationException
%
% This exception indicates that a failure occurred while initializing a plug-in.
%
% PluginInitializationException Properties:
%   reason - The reason for the failure.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.10

classdef PluginInitializationException < Ice.LocalException
    properties
        % reason - The reason for the failure.
        reason char
    end
    methods
        function obj = PluginInitializationException(errID, msg, reason)
            if nargin <= 2
                reason = '';
            end
            if nargin == 0 || isempty(errID)
                errID = 'Ice:PluginInitializationException';
            end
            if nargin < 2 || isempty(msg)
                msg = 'Ice.PluginInitializationException';
            end
            obj = obj@Ice.LocalException(errID, msg);
            obj.reason = reason;
        end
    end
end
