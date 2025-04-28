classdef (Sealed, Hidden) DefaultSliceLoader < Ice.SliceLoader
    % DefaultSliceLoader   Summary of DefaultSliceLoader
    %
    % Implements SliceLoader by converting the type ID into a MATLAB class name.

    methods
        function r = newInstance(obj, typeId)
            if startsWith(typeId, '::')
                % check cache first
                constructor = lookup(obj.typeIdToConstructorMap, typeId, FallbackValue=[]);
                if isempty(constructor)
                    typeIdElements = strsplit(typeId, '::');
                    typeIdElements = typeIdElements(2:end); % Skip empty leading element.
                    className = strjoin(typeIdElements, '.');
                    if exist(className, 'class')
                        constructor = str2func(className);
                        obj.typeIdToConstructorMap(typeId) = constructor;
                    end
                end
                if isempty(constructor)
                    r = [];
                else
                    r = constructor();
                end
            else
                r = []; % This implementation does not resolve compact IDs.
            end
        end
    end
    methods(Access = private)
        function obj = DefaultSliceLoader()
            obj.typeIdToConstructorMap = dictionary('string', 'function_handle');
        end
    end
    properties(Constant)
        % DefaultSliceLoader singleton instance.
        Instance = Ice.DefaultSliceLoader()
    end
    properties(Access = private)
        typeIdToConstructorMap
    end
end
