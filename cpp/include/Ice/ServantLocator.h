// Copyright (c) ZeroC, Inc.

#ifndef ICE_SERVANT_LOCATOR_H
#define ICE_SERVANT_LOCATOR_H

#include "Current.h"
#include "ObjectF.h"

namespace Ice
{
    /// An application-provided class that an object adapter uses to locate servants.
    /// @remark Servant locators are provided for backward compatibility with earlier versions of Ice. You should
    /// consider using a default servant instead (see ObjectAdapter::addDefaultServant). For more advanced use cases,
    /// you can create a middleware (see ObjectAdapter::use).
    /// @headerfile Ice/Ice.h
    /// @see ObjectAdapter::addServantLocator
    class ICE_API ServantLocator
    {
    public:
        virtual ~ServantLocator();

        /// Asks this servant locator to find and return a servant.
        /// @remark The caller (the object adapter) does not insert the returned servant into its Active Servant Map.
        /// @remark The implementation can throw any exception, including UserException. The Ice runtime marshals this
        /// exception in the response.
        /// @param curr Information about the incoming request being dispatched.
        /// @param cookie A "cookie" that will be passed to #finished.
        /// @return The located servant, or nullptr if no suitable servant was found.
        virtual ObjectPtr locate(const Current& curr, std::shared_ptr<void>& cookie) = 0;

        /// Notifies this servant locator that the dispatch on the servant returned by #locate is complete.
        /// The object adapter calls this function only when #locate returns a non-null servant.
        /// @remark The implementation can throw any exception, including UserException. The Ice runtime marshals this
        /// exception in the response. If both the dispatch and `finished` throw an exception, the exception thrown by
        /// `finished` prevails and is marshaled back to the client.
        /// @param curr Information about the incoming request being dispatched.
        /// @param servant The servant that was returned by #locate.
        /// @param cookie The cookie that was returned by #locate.
        virtual void finished(const Current& curr, const ObjectPtr& servant, const std::shared_ptr<void>& cookie) = 0;

        /// Notifies this servant locator that the object adapter in which it's installed is being deactivated.
        /// @param category The category with which this servant locator was registered.
        /// @see ObjectAdapter#destroy
        virtual void deactivate(std::string_view category) = 0;
    };

    /// A shared pointer to a ServantLocator.
    using ServantLocatorPtr = std::shared_ptr<ServantLocator>;
}

#endif
