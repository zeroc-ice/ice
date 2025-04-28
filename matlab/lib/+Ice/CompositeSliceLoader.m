classdef (Sealed) CompositeSliceLoader < Ice.SliceLoader
    methods
        function obj = CompositeSliceLoader(varargin)
            % Constructor for CompositeSliceLoader.
            % Accepts a variable number of Ice.SliceLoader objects as arguments.

            obj.sliceLoaders = cell(1, length(varargin));
            for i = 1:length(varargin)
                if isa(varargin{i}, 'Ice.SliceLoader')
                    obj.sliceLoaders{i} = varargin{i};
                else
                    error('Invalid argument type. Expected Ice.SliceLoader.');
                end
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

        function add(obj, loader)
            % Add a new Slice loader to the composite loader.

            if isa(loader, 'Ice.SliceLoader')
                obj.sliceLoaders{end + 1} = loader;
            else
                error('Invalid argument type. Expected Ice.SliceLoader.');
            end
        end
    end
    properties(Access = private)
        sliceLoaders(1,:)
    end
end
