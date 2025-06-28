classdef (Sealed) ClassSliceLoader < Ice.SliceLoader
    %CLASSSLICELOADER Implements Ice.SliceLoader using meta classes.
    %
    %   ClassSliceLoader Methods:
    %     ClassSliceLoader - Constructs a ClassSliceLoader from one or more meta classes.
    %     newInstance - Creates a class or exception instance from a Slice type ID.

    % Copyright (c) ZeroC, Inc.

    methods
        function obj = ClassSliceLoader(metaclass)
            %CLASSSLICELOADER Constructs a ClassSliceLoader from one or more meta classes.
            %
            %   Input Arguments (Repeating)
            %     metaclass - The meta class of a generated class or exception.
            %       matlab.metadata.Class
            %
            %   Output Arguments
            %     obj - The new ClassSliceLoader.
            %       Ice.ClassSliceLoader scalar

            arguments (Repeating)
                metaclass (1, 1) matlab.metadata.Class
            end

            obj.typeIdToConstructorMap = configureDictionary('string', 'function_handle');

            for i = 1:length(metaclass)
                mc = metaclass{i};
                typeId = Ice.ClassSliceLoader.resolveConstant(mc, 'TypeId');
                if isempty(typeId)
                    error('Ice:ArgumentException', ...
                        '%s is not a generated class and does not derive from one.', mc.Name);
                else
                    obj.typeIdToConstructorMap(typeId) = str2func(mc.Name);
                    compactId = Ice.ClassSliceLoader.resolveConstant(mc, 'CompactId');
                    if ~isempty(compactId)
                        obj.typeIdToConstructorMap(compactId) = str2func(mc.Name);
                    end
                end
            end
        end

        function r = newInstance(obj, typeId)
            arguments
                obj (1, 1) Ice.ClassSliceLoader
                typeId (1, :) char
            end
            constructor = lookup(obj.typeIdToConstructorMap, typeId, FallbackValue=obj.CreateEmptyArray);
            r = constructor();
        end
    end
    methods (Static,  Access = private)
        function r = resolveConstant(mc, name)
            % Resolves a constant value in a meta class.
            %
            %   Input Arguments
            %     mc - The meta class.
            %     name - The name of the constant.
            %
            %   Output Arguments
            %     r - The resolved constant value.

            for i = 1:length(mc.PropertyList)
                prop = mc.PropertyList(i);
                if strcmp(prop.Name, name) && prop.Constant
                    r = prop.DefaultValue;
                    return;
                end
            end
            % Not found, try base classes recursively.
            for i = 1:length(mc.SuperclassList)
                superClass = mc.SuperclassList(i);
                r = Ice.ClassSliceLoader.resolveConstant(superClass, name);
                if ~isempty(r)
                    return;
                end
            end
            r = []; % Not found in this class or any superclasses.
        end
    end

    properties (Access = private)
        typeIdToConstructorMap
    end
    properties (Constant, Access = private)
        CreateEmptyArray = @() []
    end
end
