
% PluginInitializationException   Summary of PluginInitializationException
%
% This exception indicates that a failure occurred while initializing
% a plug-in.
%
% PluginInitializationException Properties:
%   reason - The reason for the failure.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef PluginInitializationException < Ice.LocalException
    properties
        % reason - The reason for the failure.
        reason char
    end
    methods
        function obj = PluginInitializationException(ice_exid, ice_exmsg, reason)
            if nargin <= 2
                reason = '';
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:PluginInitializationException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.PluginInitializationException';
            end
            obj = obj@Ice.LocalException(ice_exid, ice_exmsg);
            obj.reason = reason;
        end
        function id = ice_id(~)
            id = '::Ice::PluginInitializationException';
        end
    end
end
