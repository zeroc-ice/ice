// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

/// <summary>
/// An application-provided class that an object adapter uses to locate servants.
/// </summary>
/// <remarks>Servant locators are provided for backward compatibility with earlier versions of Ice. You should
/// consider using a default servant instead (see <see cref="ObjectAdapter.addDefaultServant"/>). For more advanced use
/// cases, you can create a middleware (see <see cref="ObjectAdapter.use"/>).</remarks>
/// <seealso cref="ObjectAdapter.addServantLocator"/>
public interface ServantLocator
{
    /// <summary>
    /// Asks this servant locator to find and return a servant.
    /// </summary>
    /// <param name="curr">Information about the incoming request being dispatched.</param>
    /// <param name="cookie">A "cookie" that will be passed to <see cref="finished"/>.</param>
    /// <returns>The located servant, or null if no suitable servant was found.</returns>
    /// <remarks>The caller (the object adapter) does not insert the returned servant into its Active Servant Map.
    /// The implementation can throw any exception, including user exceptions. The Ice runtime marshals this
    /// exception in the response.</remarks>
    Object? locate(Current curr, out object? cookie);

    /// <summary>
    /// Notifies this servant locator that the dispatch on the servant returned by <see cref="locate"/> is complete.
    /// The object adapter calls this method only when <see cref="locate"/> returns a non-null servant.
    /// </summary>
    /// <param name="curr">Information about the incoming request being dispatched.</param>
    /// <param name="servant">The servant that was returned by <see cref="locate"/>.</param>
    /// <param name="cookie">The cookie that was returned by <see cref="locate"/>.</param>
    /// <remarks>The implementation can throw any exception, including user exceptions. The Ice runtime marshals this
    /// exception in the response. If both the dispatch and <c>finished</c> throw an exception, the exception thrown by
    /// <c>finished</c> prevails and is marshaled back to the client.</remarks>
    void finished(Current curr, Object servant, object? cookie);

    /// <summary>
    /// Notifies this servant locator that the object adapter in which it's installed is being deactivated.
    /// </summary>
    /// <param name="category">The category with which this servant locator was registered.</param>
    /// <seealso cref="ObjectAdapter.destroy"/>
    void deactivate(string category);
}
