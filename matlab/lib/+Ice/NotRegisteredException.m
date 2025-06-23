classdef NotRegisteredException < Ice.LocalException
    %NOTREGISTEREDEXCEPTION An attempt was made to find or deregister something that is not registered with the Ice
    %   run time or Ice locator. This exception is raised if an attempt is made to remove a servant, servant locator,
    %   facet, plug-in, object adapter, object, or user exception factory that is not currently registered. It's also
    %   raised if the Ice locator can't find an object or object adapter when resolving an indirect proxy or when an
    %   object adapter is activated.
    %
    %   NotRegisteredException Properties:
    %     kindOfObject - The kind of object that could not be removed.
    %     id - The ID (or name) of the object that could not be removed.

    % Copyright (c) ZeroC, Inc.

    properties
        %KINDOFOBJECT The kind of object that could not be removed.
        %   character vector
        kindOfObject (1, :) char = ''

        %ID The ID (or name) of the object that could not be removed.
        %   character vector
        id (1, :) char = ''
    end
    methods(Hidden)
        % Convenience constructor without an errID or what message.
        function obj = NotRegisteredException(kindOfObject, id)
            if nargin == 0 % default constructor
                superArgs = {};
            else
                assert(nargin == 2, 'Invalid number of arguments');
                superArgs = {'Ice:NotRegisteredException', sprintf('No %s is registered with ID ''%s''.', ...
                    kindOfObject, id)};
            end
            obj@Ice.LocalException(superArgs{:});
            if nargin > 0
                obj.kindOfObject = kindOfObject;
                obj.id = id;
            end
        end
    end
end
