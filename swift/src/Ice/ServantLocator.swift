// Copyright (c) ZeroC, Inc.

import Foundation

/// An application-provided class that an object adapter uses to locate servants.
///
/// Servant locators are provided for backward compatibility with earlier versions of Ice.
/// You should consider using a default servant instead (see ``ObjectAdapter/addDefaultServant(servant:category:)``).
/// For more advanced use cases, you can create a middleware (see ``ObjectAdapter/use(_:)``).
public protocol ServantLocator: Sendable {
    /// Asks this servant locator to find and return a servant.
    ///
    /// - Remark: The caller (the object adapter) does not insert the returned servant into its Active Servant Map.
    ///   This must be done by the servant locator implementation, if this is desired.
    /// - Remark: If you call `locate` from your own code, you must also call ``finished(curr:servant:cookie:)``
    ///   when you have finished using the servant, provided that `locate` returned a non-nil servant.
    ///
    /// - Parameter curr: Information about the incoming request for which a servant is required.
    /// - Returns: A tuple containing:
    ///   - `returnValue`: The located servant of type `Dispatcher?`, or `nil` if no suitable servant was found.
    ///   - `cookie`: A "cookie" that will be passed to ``finished(curr:servant:cookie:)``.
    /// - Throws: The implementation can throw any exception, including ``UserException``.
    ///   The Ice runtime will marshal this exception in the response.
    func locate(_ curr: Current) throws -> (returnValue: Dispatcher?, cookie: AnyObject?)

    /// Notifies this servant locator that the dispatch on the servant returned by ``locate(_:)`` is complete.
    /// The object adapter calls this method only when ``locate(_:)`` returns a non-nil servant.
    ///
    /// - Parameters:
    ///   - curr: Information about the incoming request for which a servant was located.
    ///   - servant: The servant that was returned by ``locate(_:)``.
    ///   - cookie: The cookie that was returned by ``locate(_:)``.
    /// - Throws: The implementation can throw any exception, including ``UserException``.
    ///   The Ice runtime will marshal this exception in the response. If both the dispatch and `finished` throw
    ///   an exception, the exception thrown by `finished` prevails and is marshaled back to the client.
    func finished(curr: Current, servant: Dispatcher, cookie: AnyObject?) throws

    /// Notifies this servant locator that the object adapter in which it's installed is being deactivated.
    ///
    /// - Parameter category: The category with which this servant locator was registered.
    func deactivate(_ category: String)
}
