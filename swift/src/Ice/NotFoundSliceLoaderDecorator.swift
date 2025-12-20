// Copyright (c) ZeroC, Inc.

import Synchronization

/// Decorates a ``SliceLoader`` to cache `nil` results.
final class NotFoundSliceLoaderDecorator: SliceLoader {
    private let decoratee: SliceLoader
    private let cacheSize: Int32
    private var logger: Logger?
    private let notFoundSet = Mutex<Set<String>>([])

    /// Creates a NotFoundSliceLoaderDecorator.
    ///
    /// - Parameters:
    ///   - decoratee: The SliceLoader to decorate.
    ///   - cacheSize: The maximum number of type IDs that can be cached.
    ///   - logger: The logger used to warn when the cache is full. It's nil when Ice.Warn.SliceLoader is set to 0.
    init(_ decoratee: SliceLoader, cacheSize: Int32, logger: Logger?) {
        self.decoratee = decoratee
        self.cacheSize = cacheSize
        self.logger = logger
    }

    func newInstance(_ typeId: String) -> AnyObject? {
        if notFoundSet.withLock({ $0.contains(typeId) }) {
            return nil
        }

        let instance = decoratee.newInstance(typeId)
        if instance == nil {
            var cacheFullLogger: Logger? = nil
            notFoundSet.withLock {
                if $0.count < cacheSize {
                    $0.insert(typeId)
                } else {
                    cacheFullLogger = logger
                    logger = nil  // we log only once outside the synchronization block
                }
            }
            if let cacheFullLogger = cacheFullLogger {
                cacheFullLogger.warning(
                    """
                    SliceLoader: Type ID '\(typeId)' not found and the not found cache is full. \
                    The cache size is set to \(cacheSize). You can increase the cache size by setting property \
                    Ice.SliceLoader.NotFoundCacheSize.
                    """
                )
            }
        }
        return instance
    }
}
