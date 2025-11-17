// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * An application-provided class that an object adapter uses to locate servants.
 *
 * <p>Servant locators are provided for backward compatibility with earlier versions of Ice.
 * You should consider using a default servant instead (see {@link ObjectAdapter#addDefaultServant}).
 * For more advanced use cases, you can create a middleware (see {@link ObjectAdapter#use}).
 *
 * @see ObjectAdapter#addServantLocator
 * @see ObjectAdapter#findServantLocator
 */
public interface ServantLocator {
    /** Holds the result of calling {@link ServantLocator#locate}. */
    public static class LocateResult {
        /** Constructs an empty LocateResult with a {@code null} located servant. */
        public LocateResult() {}

        /**
         * Constructs a LocateResult from the return value of {@link ServantLocator#locate} and a cookie.
         *
         * @param returnValue the located servant, or {@code null} if no suitable servant was found
         * @param cookie a "cookie" that will be passed to {@link ServantLocator#finished}
         */
        public LocateResult(com.zeroc.Ice.Object returnValue, java.lang.Object cookie) {
            this.returnValue = returnValue;
            this.cookie = cookie;
        }

        /** The located servant, or {@code null} if no suitable servant was found. */
        public Object returnValue;

        /** A "cookie" that will be passed to {@link ServantLocator#finished}. */
        public java.lang.Object cookie;
    }

    /**
     * Asks this servant locator to find and return a servant.
     *
     * <p class="Note">The caller (the object adapter) does not insert the returned servant into its Active Servant Map.
     * This must be done by the servant locator implementation, if this is desired.
     *
     * <p class="Note">If you call {@code locate} from your own code, you must also call {@link #finished}
     * when you have finished using the servant, provided that {@code locate} returned a non-null servant.
     *
     * @param curr information about the incoming request for which a servant is required
     * @return an instance of {@link ServantLocator.LocateResult} containing the located servant,
     *     or {@code null} if no suitable servant was found
     * @throws UserException The implementation can throw any exception, including {@link UserException}.
     *     The Ice runtime will marshal this exception in the response.
     * @see ObjectAdapter
     */
    ServantLocator.LocateResult locate(Current curr) throws UserException;

    /**
     * Notifies this servant locator that the dispatch on the servant returned by {@link #locate} is complete.
     * The object adapter calls this method only when {@link #locate} returns a non-null servant.
     *
     * @param curr information about the incoming request for which a servant was located
     * @param servant the servant that was returned by {@link #locate}
     * @param cookie the cookie that was returned by {@link #locate}
     * @throws UserException The implementation can throw any exception, including {@link UserException}.
     *     The Ice runtime will marshal this exception in the response. If both the dispatch and {@code finished} throw
     *     an exception, the exception thrown by {@code finished} prevails and is marshaled back to the client.
     */
    void finished(Current curr, Object servant, java.lang.Object cookie) throws UserException;

    /**
     * Notifies this servant locator that the object adapter in which it's installed is being deactivated.
     *
     * @param category the category with which this servant locator was registered
     * @see ObjectAdapter#destroy
     */
    void deactivate(String category);
}
