// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicReference;

/**
 * Decorates SliceLoader to cache null results.
 */
final class NotFoundSliceLoaderDecorator implements SliceLoader {
    private final SliceLoader _decoratee;
    private final int _cacheSize;
    private final AtomicReference<Logger> _logger;
    private final Set<String> _notFoundSet = ConcurrentHashMap.newKeySet();

    /**
     * Creates a NotFoundSliceLoaderDecorator.
     *
     * @param decoratee The SliceLoader to decorate.
     * @param cacheSize The maximum number of type IDs that can be cached.
     * @param logger The logger used to warn when the cache is full. It's null when Ice.Warn.SliceLoader is set to 0.
     */
    NotFoundSliceLoaderDecorator(SliceLoader decoratee, int cacheSize, Logger logger) {
        _decoratee = decoratee;
        _cacheSize = cacheSize;
        _logger = new AtomicReference<>(logger);
    }

    @Override
    public java.lang.Object newInstance(String typeId) {
        if (_notFoundSet.contains(typeId)) {
            return null;
        }

        java.lang.Object instance = _decoratee.newInstance(typeId);
        if (instance == null) {
            if (_notFoundSet.size() < _cacheSize) {
                _notFoundSet.add(typeId);
            } else {
                Logger logger = _logger.getAndSet(null); // we warn at most once.
                if (logger != null) {
                    logger.warning(
                        String.format(
                            "SliceLoader: Type ID '%s' not found and the not found cache is full. "
                                + "The cache size is set to %d. You can increase the cache size by setting property "
                                + "Ice.SliceLoader.NotFoundCacheSize.",
                            typeId, _cacheSize));
                }
            }
        }
        return instance;
    }
}
