classdef (Sealed) NotFoundSliceLoaderDecorator < Ice.SliceLoader
    %NOTFOUNDSLICELOADERDECORATOR Decorates a SliceLoader to cache not found (empty array) results.
    %
    %   NotFoundSliceLoaderDecorator Methods:
    %     NotFoundSliceLoaderDecorator - Constructs a NotFoundSliceLoaderDecorator.
    %     newInstance - Creates a class or exception instance from a Slice type ID.

    methods
        function obj = NotFoundSliceLoaderDecorator(decoratee, cacheSize, logger)
            %NOTFOUNDSLICELOADERDECORATOR Constructs a NotFoundSliceLoaderDecorator.
            %
            %   Input Arguments
            %     decoratee - The SliceLoader to decorate.
            %       Ice.SliceLoader scalar
            %     cacheSize - The maximum number of type IDs that can be cached.
            %       int32 scalar
            %     logger - The logger used to warn when the cache is full. It's empty when Ice.Warn.SliceLoader is set
            %       to 0.
            %       Ice.Logger scalar | Ice.Logger empty array

            obj.decoratee = decoratee;
            obj.cacheSize = cacheSize;
            obj.logger = logger;
            obj.notFoundSet = configureDictionary('char', 'logical');
        end

        function r = newInstance(obj, typeId)
            if isKey(obj.notFoundSet, typeId)
                r = [];
                return;
            end

            r = obj.decoratee.newInstance(typeId);
            if isempty(r)
                if numEntries(obj.notFoundSet) < obj.cacheSize
                    obj.notFoundSet(typeId) = true;
                else
                    if ~isempty(obj.logger)
                        obj.logger.warning(sprintf( ...
                            ['SliceLoader: Type ID %s not found and the not found cache is full. ' ...
                            'The cache size is set to %d. You can increase the cache size by setting property ' ...
                            'Ice.SliceLoader.NotFoundCacheSize.'], ...
                            typeId, obj.cacheSize));
                        obj.logger = []; % Warn at most once.
                    end
                end
            end
        end
    end
    properties(Access = private)
        decoratee
        cacheSize
        logger
        notFoundSet
    end
end
