classdef (Sealed) ClassSliceLoader < Ice.SliceLoader
    % ClassSliceLoader   Summary of ClassSliceLoader
    %
    % Implements SliceLoader by ...

    methods
        function obj = ClassSliceLoader(metaclassArray)
            obj.typeIdToConstructorMap = configureDictionary('string', 'function_handle');

            for i = 1:length(metaclassArray)
                mc = metaclassArray(i);
                if ~isa(mc, 'matlab.metadata.Class')
                    error('ClassSliceLoader:InvalidArgument', 'Expected array of matlab.metadata.Class.');
                end
                for j = 1:length(mc.PropertyList)
                    prop = mc.PropertyList(j);
                    if strcmp(prop.Name, 'TypeId')
                        obj.typeIdToConstructorMap(prop.DefaultValue) = str2func(mc.Name);
                    elseif strcmp(prop.Name, 'CompactId')
                        obj.typeIdToConstructorMap(prop.DefaultValue) = str2func(mc.Name);
                    end
                end
            end
        end

        function r = newInstance(obj, typeId)
            constructor = lookup(obj.typeIdToConstructorMap, typeId, FallbackValue=obj.CreateEmptyArray);
            r = constructor();
        end
    end
    properties(Access = private)
        typeIdToConstructorMap
    end
    properties(Constant, Access = private)
        CreateEmptyArray = @() []
    end
end
