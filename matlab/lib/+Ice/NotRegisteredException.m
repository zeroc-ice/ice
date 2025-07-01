classdef NotRegisteredException < Ice.LocalException
    %NOTREGISTEREDEXCEPTION The exception that is thrown when you attempt to find or deregister something that is
    %   not registered with the Ice runtime or Ice locator.
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
    methods (Hidden)
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
