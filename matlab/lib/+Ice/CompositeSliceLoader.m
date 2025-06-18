classdef (Sealed) CompositeSliceLoader < Ice.SliceLoader
    % CompositeSliceLoader   Implements SliceLoader by combining multiple Slice loaders.
    %
    % CompositeSliceLoader methods:
    %   CompositeSliceLoader - Constructs a CompositeSliceLoader.
    %   add - Adds a new Slice loader to the composite loader.
    %   newInstance - Creates a class or exception instance from a Slice type ID.
    methods
        function obj = CompositeSliceLoader(sliceLoader)
            % Constructs a CompositeSliceLoader.
            %
            % Parameters:
            %   sliceLoader - One or more Ice.SliceLoader objects.

            arguments (Repeating)
                sliceLoader (1, 1) Ice.SliceLoader
            end
            obj.sliceLoaders = sliceLoader; % sliceLoader is a cell array
        end

        function add(obj, loader)
            % Add a new Slice loader to the composite loader.
            %
            % Parameters:
            %   loader (Ice.SliceLoader) - The Slice loader to add.

            arguments
                obj (1, 1) Ice.CompositeSliceLoader
                loader (1, 1) Ice.SliceLoader
            end
            obj.sliceLoaders{end + 1} = loader;
        end

        function r = newInstance(obj, typeId)
            arguments
                obj (1, 1) Ice.CompositeSliceLoader
                typeId (1, :) char
            end
            for i = 1:length(obj.sliceLoaders)
                r = obj.sliceLoaders{i}.newInstance(typeId);
                if ~isempty(r)
                    return;
                end
            end
            r = []; % No loader could create an instance.
        end
    end
    properties(Access = private)
        sliceLoaders (1, :) cell
    end
end
