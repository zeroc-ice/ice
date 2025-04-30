classdef (Sealed) CompositeSliceLoader < Ice.SliceLoader
    % CompositeSliceLoader   Implements SliceLoader by combining multiple Slice loaders.
    %
    % CompositeSliceLoader methods:
    %   CompositeSliceLoader - Constructs a CompositeSliceLoader.
    %   add - Adds a new Slice loader to the composite loader.
    %   newInstance - Creates a class or exception instance from a Slice type ID.
    methods
        function obj = CompositeSliceLoader(varargin)
            % Constructs a CompositeSliceLoader.
            %
            % Parameters:
            %   varargin - A variable number of Ice.SliceLoader objects.

            obj.sliceLoaders = cell(1, length(varargin));
            for i = 1:length(varargin)
                if isa(varargin{i}, 'Ice.SliceLoader')
                    obj.sliceLoaders{i} = varargin{i};
                else
                    error('Invalid argument type. Expected Ice.SliceLoader.');
                end
            end
        end

        function add(obj, loader)
            % Add a new Slice loader to the composite loader.
            %
            % Parameters:
            %   loader (Ice.SliceLoader) - The Slice loader to add.

            if isa(loader, 'Ice.SliceLoader')
                obj.sliceLoaders{end + 1} = loader;
            else
                error('Invalid argument type. Expected Ice.SliceLoader.');
            end
        end

        function r = newInstance(obj, typeId)
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
        sliceLoaders(1,:)
    end
end
