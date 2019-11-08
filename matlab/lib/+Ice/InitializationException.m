% InitializationException   Summary of InitializationException
%
% This exception is raised when a failure occurs during initialization.
%
% InitializationException Properties:
%   reason - The reason for the failure.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef InitializationException < Ice.LocalException
    properties
        % reason - The reason for the failure.
        reason char
    end
    methods
        function obj = InitializationException(ice_exid, ice_exmsg, reason)
            if nargin <= 2
                reason = '';
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:InitializationException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.InitializationException';
            end
            obj = obj@Ice.LocalException(ice_exid, ice_exmsg);
            obj.reason = reason;
        end
        function id = ice_id(~)
            id = '::Ice::InitializationException';
        end
    end
end
