% IllegalServantException   Summary of IllegalServantException
%
% This exception is raised to reject an illegal servant (typically
% a null servant)
%
% IllegalServantException Properties:
%   reason - Describes why this servant is illegal.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef IllegalServantException < Ice.LocalException
    properties
        % reason - Describes why this servant is illegal.
        reason char
    end
    methods
        function obj = IllegalServantException(ice_exid, ice_exmsg, reason)
            if nargin <= 2
                reason = '';
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:IllegalServantException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.IllegalServantException';
            end
            obj = obj@Ice.LocalException(ice_exid, ice_exmsg);
            obj.reason = reason;
        end
        function id = ice_id(~)
            id = '::Ice::IllegalServantException';
        end
    end
end
