% AlreadyRegisteredException   Summary of AlreadyRegisteredException
%
% An attempt was made to register something more than once with the Ice run time. This exception is raised if an
% attempt is made to register a servant, servant locator, facet, value factory, plug-in, object adapter, object, or
% user exception factory more than once for the same ID.
%
% AlreadyRegisteredException Properties:
%   kindOfObject - The kind of object that could not be removed: "servant", "facet", "object", "default servant", "servant locator", "value factory", "plugin", "object adapter", "object adapter with router", "replica group".
%   id - The ID (or name) of the object that is registered already.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.10

classdef AlreadyRegisteredException < Ice.LocalException
    properties
        % kindOfObject - The kind of object that could not be removed: "servant", "facet", "object", "default servant",
        % "servant locator", "value factory", "plugin", "object adapter", "object adapter with router", "replica group".
        kindOfObject char
        % id - The ID (or name) of the object that is registered already.
        id char
    end
    methods
        function obj = AlreadyRegisteredException(errID, msg, kindOfObject, id)
            if nargin <= 2
                kindOfObject = '';
                id = '';
            end
            if nargin == 0 || isempty(errID)
                errID = 'Ice:AlreadyRegisteredException';
            end
            if nargin < 2 || isempty(msg)
                msg = 'Ice.AlreadyRegisteredException';
            end
            obj = obj@Ice.LocalException(errID, msg);
            obj.kindOfObject = kindOfObject;
            obj.id = id;
        end
    end
end
