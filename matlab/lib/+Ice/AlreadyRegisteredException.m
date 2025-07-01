classdef (Sealed) AlreadyRegisteredException < Ice.LocalException
    %ALREADYREGISTEREDEXCEPTION The exception that is thrown when you attempt to register an object more than once with
    %   the Ice runtime.
    %
    %   AlreadyRegisteredException Properties:
    %     kindOfObject - The kind of object that could not be removed.
    %     id - The ID (or name) of the object that is registered already.

    % Copyright (c) ZeroC, Inc.

    properties
        %KINDOFOBJECT The kind of object that could not be removed.
        %   character vector
        kindOfObject (1, :) char = ''

        %ID The ID (or name) of the object that is registered already.
        %   character vector
        id (1, :) char = ''
    end
    methods (Hidden)
        function obj = AlreadyRegisteredException(kindOfObject, id)
            % Convenience constructor without an errID or what message.
            if nargin == 0 % default constructor
                superArgs = {};
            else
                assert(nargin == 2, 'Invalid number of arguments');
                superArgs = {'Ice:AlreadyRegisteredException', sprintf('Another %s is already registered with ID ''%s''.', ...
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
