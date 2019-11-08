% SecurityException   Summary of SecurityException
%
% This exception indicates a failure in a security subsystem,
% such as the IceSSL plug-in.
%
% SecurityException Properties:
%   reason - The reason for the failure.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef SecurityException < Ice.LocalException
    properties
        % reason - The reason for the failure.
        reason char
    end
    methods
        function obj = SecurityException(ice_exid, ice_exmsg, reason)
            if nargin <= 2
                reason = '';
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:SecurityException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.SecurityException';
            end
            obj = obj@Ice.LocalException(ice_exid, ice_exmsg);
            obj.reason = reason;
        end
        function id = ice_id(~)
            id = '::Ice::SecurityException';
        end
    end
end
