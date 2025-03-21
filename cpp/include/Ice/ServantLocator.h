// Copyright (c) ZeroC, Inc.

#ifndef ICE_SERVANT_LOCATOR_H
#define ICE_SERVANT_LOCATOR_H

#include "Current.h"
#include "ObjectF.h"

namespace Ice
{
    /// A servant locator is called by an object adapter to locate a servant that is not found in its active servant
    /// map.
    /// @see ObjectAdapter
    /// @see ObjectAdapter#addServantLocator
    /// @see ObjectAdapter#findServantLocator
    /// @headerfile Ice/Ice.h
    class ICE_API ServantLocator
    {
    public:
        virtual ~ServantLocator();

        /// Called before a request is dispatched if a servant cannot be found in the object adapter's active servant
        /// map. Note that the object adapter does not automatically insert the returned servant into its active servant
        /// map. This must be done by the servant locator implementation, if this is desired. <code>locate</code> can
        /// throw any user exception. If it does, that exception is marshaled back to the client. If the Slice
        /// definition for the corresponding operation includes that user exception, the client receives that user
        /// exception; otherwise, the client receives {@link UnknownUserException}. If <code>locate</code> throws any
        /// exception, the Ice run time does <em>not</em>call <code>finished</code>. <p class="Note">If you call
        /// <code>locate</code> from your own code, you must also call <code>finished</code> when you have finished
        /// using the servant, provided that <code>locate</code> returned a non-null servant.
        /// @param curr Information about the current operation for which a servant is required.
        /// @param cookie A "cookie" that will be passed to <code>finished</code>.
        /// @return The located servant, or null if no suitable servant has been found.
        /// @throws UserException The implementation can raise a UserException and the run time will marshal it as the
        /// result of the invocation.
        /// @see ObjectAdapter
        /// @see Current
        /// @see #finished
        virtual ObjectPtr locate(const Current& curr, std::shared_ptr<void>& cookie) = 0;

        /// Called by the object adapter after a request has been made. This operation is only called if
        /// <code>locate</code> was called prior to the request and returned a non-null servant. This operation can be
        /// used for cleanup purposes after a request. <code>finished</code> can throw any user exception. If it does,
        /// that exception is marshaled back to the client. If the Slice definition for the corresponding operation
        /// includes that user exception, the client receives that user exception; otherwise, the client receives {@link
        /// UnknownUserException}. If both the operation and <code>finished</code> throw an exception, the exception
        /// thrown by <code>finished</code> is marshaled back to the client.
        /// @param curr Information about the current operation call for which a servant was located by
        /// <code>locate</code>.
        /// @param servant The servant that was returned by <code>locate</code>.
        /// @param cookie The cookie that was returned by <code>locate</code>.
        /// @throws UserException The implementation can raise a UserException and the run time will marshal it as the
        /// result of the invocation.
        /// @see ObjectAdapter
        /// @see Current
        /// @see #locate
        virtual void finished(const Current& curr, const ObjectPtr& servant, const std::shared_ptr<void>& cookie) = 0;

        /// Called when the object adapter in which this servant locator is installed is destroyed.
        /// @param category Indicates for which category the servant locator is being deactivated.
        /// @see ObjectAdapter#destroy
        /// @see Communicator#shutdown
        /// @see Communicator#destroy
        virtual void deactivate(std::string_view category) = 0;
    };

    /// A shared pointer to a ServantLocator.
    using ServantLocatorPtr = std::shared_ptr<ServantLocator>;
}

#endif
