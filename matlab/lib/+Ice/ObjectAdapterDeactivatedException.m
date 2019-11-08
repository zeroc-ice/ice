% ObjectAdapterDeactivatedException   Summary of ObjectAdapterDeactivatedException
%
% This exception is raised if an attempt is made to use a deactivated
% ObjectAdapter.
%
% ObjectAdapterDeactivatedException Properties:
%   name - Name of the adapter.
%
% See also ObjectAdapter.deactivate, Communicator.shutdown

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef ObjectAdapterDeactivatedException < Ice.LocalException
    properties
        % name - Name of the adapter.
        name char
    end
    methods
        function obj = ObjectAdapterDeactivatedException(ice_exid, ice_exmsg, name)
            if nargin <= 2
                name = '';
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:ObjectAdapterDeactivatedException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.ObjectAdapterDeactivatedException';
            end
            obj = obj@Ice.LocalException(ice_exid, ice_exmsg);
            obj.name = name;
        end
        function id = ice_id(~)
            id = '::Ice::ObjectAdapterDeactivatedException';
        end
    end
end
