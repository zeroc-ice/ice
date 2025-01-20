% AlreadyRegisteredException   Summary of AlreadyRegisteredException
%
% An attempt was made to register something more than once with the Ice run time. This exception is raised if an
% attempt is made to register a servant, servant locator, facet, value factory, plug-in, object adapter, object, or
% user exception factory more than once for the same ID.
%
% AlreadyRegisteredException Properties:
%   kindOfObject - The kind of object that could not be removed: "servant", "facet", "object", "default servant", "servant locator", "value factory", "plugin", "object adapter", "object adapter with router", "replica group".
%   id - The ID (or name) of the object that is registered already.

%  Copyright (c) ZeroC, Inc.

classdef (Sealed) AlreadyRegisteredException < Ice.LocalException
    properties
        % kindOfObject - The kind of object that could not be removed: "servant", "facet", "object", "default servant",
        % "servant locator", "value factory", "plugin", "object adapter", "object adapter with router", "replica group".
        kindOfObject char
        % id - The ID (or name) of the object that is registered already.
        id char
    end
    methods
        % Convenience constructor without an errID or what message.
        function obj = AlreadyRegisteredException(kindOfObject, id)
            if nargin == 0 % default constructor
                superArgs = {};
                kindOfObject = '';
                id = '';
            else
                assert(nargin == 2, 'Invalid number of arguments');
                superArgs = {'Ice:AlreadyRegisteredException', sprintf('Another %s is already registered with ID ''%s''.', ...
                    kindOfObject, id)};
            end
            obj@Ice.LocalException(superArgs{:});
            obj.kindOfObject = kindOfObject;
            obj.id = id;
        end
    end
end
