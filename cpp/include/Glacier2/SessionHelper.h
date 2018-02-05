// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef GLACIER2_SESSION_HELPER_H
#define GLACIER2_SESSION_HELPER_H

#include <IceUtil/Shared.h>
#include <IceUtil/Handle.h>
#include <IceUtil/Thread.h>
#include <IceUtil/Mutex.h>

#include <Ice/Initialize.h>
#include <Ice/Properties.h>
#include <Ice/Communicator.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/Ice.h>

#include <Glacier2/Session.h>
#include <Glacier2/Router.h>

#include <map>
#include <string>

namespace Glacier2
{

/** The IANA-registered port number for Glacier2 via SSL. */
const int GLACIER2_SSL_PORT = 4064;
/** The IANA-registered port number for Glacier2 via TCP. */
const int GLACIER2_TCP_PORT = 4063;

/**
 * Encapsulates a Glacier2 session and provides much of the same functionality as Glacier2::Application
 * but better suited for graphical applications.
 * \headerfile Glacier2/Glacier2.h
 */
class GLACIER2_API SessionHelper
#ifndef ICE_CPP11_MAPPING
    : public virtual IceUtil::Shared
#endif
{
public:
    virtual ~SessionHelper();

    /**
     * Initiates the destruction of the Glacier2 session, including the communicator created by the SessionHelper.
     */
    virtual void destroy() = 0;

    /**
     * Obtains the communicator created by the SessionHelper.
     * @return The communicator object.
     * @throws SessionNotExistException if no session is currently active.
     */
    virtual Ice::CommunicatorPtr communicator() const = 0;

    /**
     * Obtains the category that must be used in the identities of all callback objects.
     * @return The identity category.
     * @throws SessionNotExistException if no session is currently active.
     */
    virtual std::string categoryForClient() const = 0;

    /**
     * Adds a servant to the callback object adapter using a UUID for the identity name.
     * Also see Ice::ObjectAdapter::addWithUUID.
     * @param servant The servant to add to the callback object adapter's active servant table.
     * @return A proxy for the object.
     * @throws SessionNotExistException if no session is currently active.
     */
    virtual Ice::ObjectPrxPtr addWithUUID(const Ice::ObjectPtr& servant) = 0;

    /**
     * Obtains a proxy for the Glacier2 session.
     * @return The session proxy, or a nil proxy if no session is currently active.
     */
    virtual SessionPrxPtr session() const = 0;

    /**
     * Determines whether the session is active.
     * @return True if the session is currently active, false otherwise.
     */
    virtual bool isConnected() const = 0;

    /**
     * Obtains the callback object adapter. This object adapter is only created if the session factory
     * was configured to do so using SessionFactoryHelper::setUseCallbacks.
     * @return The object adapter, or nil if no object adapter was created.
     * @throws SessionNotExistException if no session is currently active.
     */
    virtual Ice::ObjectAdapterPtr objectAdapter() = 0;

#ifndef ICE_CPP11_MAPPING
    bool operator==(const SessionHelper&) const;
    bool operator!=(const SessionHelper&) const;
#endif

};
ICE_DEFINE_PTR(SessionHelperPtr, SessionHelper);

/**
 * Allows an application to receive notification about events in the lifecycle of a Glacier2 session.
 * \headerfile Glacier2/Glacier2.h
 */
class GLACIER2_API SessionCallback
#ifndef ICE_CPP11_MAPPING
    : public virtual IceUtil::Shared
#endif
{
public:
    virtual ~SessionCallback();

    /**
     * Called after successfully initializing a communicator.
     * @param session The corresponding session helper.
     */
    virtual void createdCommunicator(const SessionHelperPtr& session) = 0;

    /**
     * Called after successfully establishing the Glacier2 session.
     * @param session The corresponding session helper.
     */
    virtual void connected(const SessionHelperPtr& session) = 0;

    /**
     * Called after the Glacier2 session is destroyed.
     * @param session The corresponding session helper.
     */
    virtual void disconnected(const SessionHelperPtr& session) = 0;

    /**
     * Called if a failure occurred while attempting to establish a Glacier2 session.
     * @param session The corresponding session helper.
     * @param ex The exception that caused the failure.
     */
    virtual void connectFailed(const SessionHelperPtr& session, const Ice::Exception& ex) = 0;
};
ICE_DEFINE_PTR(SessionCallbackPtr, SessionCallback);

/// \cond INTERNAL
class SessionThreadCallback;
/// \endcond

/**
 * Facilitates the creation of new Glacier2 sessions.
 * \headerfile Glacier2/Glacier2.h
 */
class GLACIER2_API SessionFactoryHelper
#ifdef ICE_CPP11_MAPPING
    : public std::enable_shared_from_this<SessionFactoryHelper>
#else
    : public virtual IceUtil::Shared
#endif
{
    /// \cond INTERNAL
    friend class SessionThreadCallback; // To access thread functions
    /// \endcond

public:

    /**
     * This constructor is useful when your application has no other configuration requirements.
     * The constructor allocates an InitializationData object and a new property set.
     * @param callback The callback object (must not be nil).
     */
    SessionFactoryHelper(const SessionCallbackPtr& callback);

    /**
     * Use this constructor when you want to provide your own instance of InitializationData.
     * @param initData Initialization data for the communicator.
     * @param callback The callback object (must not be nil).
     */
    SessionFactoryHelper(const Ice::InitializationData& initData, const SessionCallbackPtr& callback);

    /**
     * This constructor is convenient when you want to supply an initial set of properties.
     * @param properties Configuration properties for the communicator.
     * @param callback The callback object (must not be nil).
     */
    SessionFactoryHelper(const Ice::PropertiesPtr& properties, const SessionCallbackPtr& callback);

    ~SessionFactoryHelper();

    /**
     * Blocks until all background threads are terminated.
     */
    void destroy();

    /**
     * Sets the object identity of the Glacier2 router.
     * @param identity The router identity.
     */
    void setRouterIdentity(const Ice::Identity& identity);

    /**
     * Obtains the object identity of the Glacier2 router.
     * @return The router identity.
     */
    Ice::Identity getRouterIdentity() const;

    /**
     * Sets the host name of the Glacier2 router.
     * @param host The router host name.
     */
    void setRouterHost(const std::string& host);

    /**
     * Obtains the host name of the Glacier2 router.
     * @return The router host name.
     */
    std::string getRouterHost() const;

    /// \cond INTERNAL
    ICE_DEPRECATED_API("is deprecated, use SessionFactoryHelper::setProtocol instead")
    void setSecure(bool);
    ICE_DEPRECATED_API("is deprecated, use SessionFactoryHelper::getProtocol instead")
    bool getSecure() const;
    /// \endcond

    /**
     * Sets the Ice protocol used for communications with the Glacier2 router.
     * @param protocol The protocol name (e.g., "tcp").
     */
    void setProtocol(const std::string& protocol);

    /**
     * Obtains the Ice protocol used for communications with the Glacier2 router.
     * @return The protocol name.
     */
    std::string getProtocol() const;

    /**
     * Sets the timeout in milliseconds for the connection to the Glacier2 router.
     * @param timeout The timeout in milliseconds.
     */
    void setTimeout(int timeout);

    /**
     * Obtains the timeout in milliseconds for the connection to the Glacier2 router.
     * @return The timeout in milliseconds.
     */
    int getTimeout() const;

    /**
     * Sets the port on which the Glacier2 router is listening.
     * @param port The router port.
     */
    void setPort(int port);

    /**
     * Obtains the port on which the Glacier2 router is listening.
     * @return The router port.
     */
    int getPort() const;

    /**
     * Returns a copy of the InitializationData object that will be used during communicator initialization.
     * @return The communicator initialization data.
     */
    Ice::InitializationData getInitializationData() const;

    /**
     * Sets the request context to be used when creating a session.
     * @param context The request context.
     */
    void setConnectContext(const std::map<std::string, std::string>& context);

    /**
     * Determines whether the session helper automatically creates an object adapter for callback servants.
     * @param b True if the helper should create an object adapter, false otherwise.
     */
    void setUseCallbacks(bool b);

    /**
     * Determines whether the session helper automatically creates an object adapter for callback servants.
     * @return True if the helper will create an object adapter, false otherwise.
     */
    bool getUseCallbacks() const;

    /**
     * Initializes a communicator, creates a Glacier2 session using SSL credentials, and returns a new
     * SessionHelper object.
     */
    SessionHelperPtr connect();

    /**
     * Initializes a communicator, creates a Glacier2 session using the given username and password,
     * and returns a new SessionHelper object.
     */
    SessionHelperPtr connect(const std::string& username,  const std::string& password);

private:

    void addThread(const SessionHelper*, const IceUtil::ThreadPtr&);

    Ice::InitializationData createInitData();
    std::string getRouterFinderStr();
    int getPortInternal() const;
    std::string createProxyStr(const Ice::Identity& ident);
    void setDefaultProperties();

    IceUtil::Mutex _mutex;
    std::string _routerHost;
    Ice::Identity _identity;
    std::string _protocol;
    int _port;
    int _timeout;
    Ice::InitializationData _initData;
    SessionCallbackPtr _callback;
    std::map<std::string, std::string> _context;
    bool _useCallbacks;
    std::map<const SessionHelper*, IceUtil::ThreadPtr> _threads;
};
ICE_DEFINE_PTR(SessionFactoryHelperPtr, SessionFactoryHelper);

}

#endif
