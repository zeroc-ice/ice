classdef NotRegisteredException < Ice.LocalException
    %NOTREGISTEREDEXCEPTION An attempt was made to find or deregister something that is not registered with Ice.
    %
    %   NotRegisteredException Properties:
    %     kindOfObject - The kind of object that is not registered.
    %     id - The ID (or name) of the object that is not registered.

    % Copyright (c) ZeroC, Inc.

    properties
        %KINDOFOBJECT The kind of object that is not registered.
        %   character vector
        kindOfObject (1, :) char = ''

        %ID The ID (or name) of the object that is not registered.
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
