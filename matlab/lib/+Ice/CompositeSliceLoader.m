classdef (Sealed) CompositeSliceLoader < Ice.SliceLoader
    %COMPOSITESLICELOADER Implements Ice.SliceLoader by combining multiple Slice loaders.
    %
    %   CompositeSliceLoader Methods:
    %     CompositeSliceLoader - Constructs a new CompositeSliceLoader.
    %     add - Adds a new Slice loader to the composite loader.
    %     newInstance - Creates a class or exception instance from a Slice type ID.

    % Copyright (c) ZeroC, Inc.

    methods
        function obj = CompositeSliceLoader(sliceLoader)
            %COMPOSITESLICELOADER Constructs a new CompositeSliceLoader.
            %
            %   Input Arguments (Repeating)
            %     sliceLoader - A Slice loader to add to the composite loader.
            %       Ice.SliceLoader scalar
            %
            %   Output Arguments
            %     obj - The new CompositeSliceLoader.
            %       Ice.CompositeSliceLoader scalar

            arguments (Repeating)
                sliceLoader (1, 1) Ice.SliceLoader
            end
            obj.sliceLoaders = sliceLoader; % sliceLoader is a cell array
        end

        function add(obj, loader)
            %ADD Adds a new Slice loader to the composite loader.
            %
            %   Input Arguments
            %     loader - The Slice loader to add.
            %       Ice.SliceLoader scalar

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
