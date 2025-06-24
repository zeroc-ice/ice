classdef (Sealed) DefaultSliceLoader < Ice.SliceLoader
    %DEFAULTSLICELOADER Implements SliceLoader by converting type IDs into MATLAB class names.
    %
    %   DefaultSliceLoader methods:
    %     newInstance - Creates a class or exception instance from a Slice type ID.
    %
    %   DefaultSliceLoader Properties:
    %     Instance - The singleton instance of DefaultSliceLoader.

    % Copyright (c) ZeroC, Inc.

    methods
        function r = newInstance(obj, typeId)
            arguments
                obj (1, 1) IceInternal.DefaultSliceLoader
                typeId (1, :) char
            end

            if startsWith(typeId, '::')
                % First check the cache.
                constructor = lookup(obj.typeIdToConstructorMap, typeId, FallbackValue=obj.CreateEmptyArray);
                if isequal(constructor, obj.CreateEmptyArray)
                    % Not found
                    typeIdElements = strsplit(typeId, '::');
                    typeIdElements = typeIdElements(2:end); % Skip empty leading element.
                    className = strjoin(typeIdElements, '.');
                    if exist(className, 'class')
                        constructor = str2func(className);
                        obj.typeIdToConstructorMap(typeId) = constructor;
                    end
                end
                r = constructor();
            else
                r = []; % This implementation does not resolve compact IDs.
            end
        end
    end
    methods(Access = private)
        function obj = DefaultSliceLoader()
            obj.typeIdToConstructorMap = configureDictionary('string', 'function_handle');
        end
    end
    properties(Constant)
        % The singleton instance of DefaultSliceLoader.
        Instance = IceInternal.DefaultSliceLoader()
    end
    properties(Constant, Access = private)
        CreateEmptyArray = @() []
    end
    properties(Access = private)
        typeIdToConstructorMap
    end
end
