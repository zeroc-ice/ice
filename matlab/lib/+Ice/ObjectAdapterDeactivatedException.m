% ObjectAdapterDeactivatedException   Summary of ObjectAdapterDeactivatedException
%
% This exception is raised if an attempt is made to use a deactivated ObjectAdapter.
%
% ObjectAdapterDeactivatedException Properties:
%   name - Name of the adapter.
%
% See also ObjectAdapter.deactivate, Communicator.shutdown

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.10

classdef ObjectAdapterDeactivatedException < Ice.LocalException
    properties
        % name - Name of the adapter.
        name char
    end
    methods
        function obj = ObjectAdapterDeactivatedException(errID, msg, name)
            if nargin <= 2
                name = '';
            end
            if nargin == 0 || isempty(errID)
                errID = 'Ice:ObjectAdapterDeactivatedException';
            end
            if nargin < 2 || isempty(msg)
                msg = 'Ice.ObjectAdapterDeactivatedException';
            end
            obj = obj@Ice.LocalException(errID, msg);
            obj.name = name;
        end
    end
end
