% ObjectAdapterIdInUseException   Summary of ObjectAdapterIdInUseException
%
% This exception is raised if an ObjectAdapter cannot be activated.
%
% This happens if the Locator detects another active ObjectAdapter with
% the same adapter id.
%
% ObjectAdapterIdInUseException Properties:
%   id - Adapter ID.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef ObjectAdapterIdInUseException < Ice.LocalException
    properties
        % id - Adapter ID.
        id char
    end
    methods
        function obj = ObjectAdapterIdInUseException(ice_exid, ice_exmsg, id)
            if nargin <= 2
                id = '';
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:ObjectAdapterIdInUseException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.ObjectAdapterIdInUseException';
            end
            obj = obj@Ice.LocalException(ice_exid, ice_exmsg);
            obj.id = id;
        end
        function id = ice_id(~)
            id = '::Ice::ObjectAdapterIdInUseException';
        end
    end
end
