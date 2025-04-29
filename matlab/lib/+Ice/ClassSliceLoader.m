classdef (Sealed) ClassSliceLoader < Ice.SliceLoader
    % ClassSliceLoader   Implements SliceLoader using an array of meta classes.
    %
    % ClassSliceLoader methods:
    %   ClassSliceLoader - Constructs a ClassSliceLoader from an array of meta classes.
    %   newInstance - Creates a class or exception instance from a Slice type ID.
    methods
        function obj = ClassSliceLoader(metaclassArray)
            % ClassSliceLoader - Constructs a ClassSliceLoader from an array of meta classes.
            %
            % Parameters:
            %   metaclassArray (matlab.metadata.Class) - An array of matlab.metadata.Class objects.

            obj.typeIdToConstructorMap = configureDictionary('string', 'function_handle');

            for i = 1:length(metaclassArray)
                mc = metaclassArray(i);
                if ~isa(mc, 'matlab.metadata.Class')
                    throw(Ice.LocalException('Ice:ArgumentException', 'Expected array of matlab.metadata.Class.'));
                end
                typeId = Ice.ClassSliceLoader.resolveConstant(mc, 'TypeId');
                if isempty(typeId)
                    throw(Ice.LocalException('Ice:ArgumentException', ...
                        sprintf('%s is not a generated class and does not derive from one.', mc.Name)));
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
            constructor = lookup(obj.typeIdToConstructorMap, typeId, FallbackValue=obj.CreateEmptyArray);
            r = constructor();
        end
    end
    methods(Static, Access = private)
        function r = resolveConstant(mc, name)
            % resolveConstant - Resolves a constant value in a meta class.
            %
            % Parameters:
            %   mc (matlab.metadata.Class) - The meta class.
            %   name (char) - The name of the constant.
            %
            % Returns (char) - The resolved constant value.

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

    properties(Access = private)
        typeIdToConstructorMap
    end
    properties(Constant, Access = private)
        CreateEmptyArray = @() []
    end
end
