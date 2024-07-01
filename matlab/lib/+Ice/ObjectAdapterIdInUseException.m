% ObjectAdapterIdInUseException   Summary of ObjectAdapterIdInUseException
%
% This exception is raised if an ObjectAdapter cannot be activated. This happens if the Locator
% detects another active ObjectAdapter with the same adapter id.
%
% ObjectAdapterIdInUseException Properties:
%   id - Adapter ID.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.10

classdef ObjectAdapterIdInUseException < Ice.LocalException
    properties
        % id - Adapter ID.
        id char
    end
    methods
        function obj = ObjectAdapterIdInUseException(errID, msg, id)
            if nargin <= 2
                id = '';
            end
            if nargin == 0 || isempty(errID)
                errID = 'Ice:ObjectAdapterIdInUseException';
            end
            if nargin < 2 || isempty(msg)
                msg = 'Ice.ObjectAdapterIdInUseException';
            end
            obj = obj@Ice.LocalException(errID, msg);
            obj.id = id;
        end
    end
end
