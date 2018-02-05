// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef GLACIER2_APPLICATION_H
#define GLACIER2_APPLICATION_H

#include <Ice/Application.h>

#include <Glacier2/Session.h>
#include <Glacier2/Router.h>

namespace Glacier2
{

/**
 *
 * This exception is raised if the session should be restarted.
 * \headerfile Glacier2/Glacier2.h
 *
 **/
class GLACIER2_API RestartSessionException : public IceUtil::ExceptionHelper<RestartSessionException>
{
public:

    virtual std::string ice_id() const;
#ifndef ICE_CPP11_MAPPING
    virtual RestartSessionException* ice_clone() const;
#endif
};

/**
 * An extension of Ice::Application that makes it easy to write
 * Glacier2 applications.
 *
 * Applications must create a derived class that implements the
 * createSession and runWithSession methods.
 *
 * The base class invokes createSession to create a new
 * Glacier2 session and then invokes runWithSession in
 * which the subclass performs its application logic. The base class
 * automatically destroys the session when runWithSession returns.
 *
 * If runWithSession calls restart or raises any of
 * the exceptions Ice::ConnectionRefusedException,
 * Ice::ConnectionLostException, Ice::UnknownLocalException,
 * Ice::RequestFailedException, or Ice::TimeoutException, the base
 * class destroys the current session and restarts the application
 * with another call to createSession followed by runWithSession.
 *
 * The application can optionally override the sessionDestroyed
 * callback method if it needs to take action when connectivity with
 * the Glacier2 router is lost.
 *
 * A program can contain only one instance of this class.
 * \headerfile Glacier2/Glacier2.h
 **/
class GLACIER2_API Application : public Ice::Application
{
public:

    /**
     * Initializes an instance that calls Ice::Communicator::shutdown if
     * a signal is received.
     **/
    Application(Ice::SignalPolicy = Ice::ICE_ENUM(SignalPolicy,HandleSignals));

#ifdef ICE_CPP11_MAPPING
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
#endif

    /**
     * Creates a new Glacier2 session. A call to
     * <code>createSession</code> always precedes a call to
     * <code>runWithSession</code>. If <code>Ice::LocalException</code>
     * is thrown from this method, the application is terminated.
     * @return The Glacier2 session.
     **/
    virtual Glacier2::SessionPrxPtr createSession() = 0;

    /**
     * Called once the communicator has been initialized and the Glacier2 session
     * has been established. A derived class must implement <code>runWithSession</code>,
     * which is the application's starting method.
     *
     * @param argc The number of elements in argv.
     *
     * @param argv The argument vector for the application. <code>Application</code>
     * scans the argument vector passed to <code>main</code> for options that are
     * specific to the Ice run time and removes them; therefore, the vector passed
     * to <code>run</code> is free from Ice-related options and contains only options
     * and arguments that are application-specific.
     *
     * @return The <code>runWithSession</code> method should return zero for successful
     * termination, and non-zero otherwise. <code>Application.main</code> returns the
     * value returned by <code>runWithSession</code>.
     **/
    virtual int runWithSession(int argc, char* argv[]) = 0;

    /**
     * Called when the session refresh thread detects that the session has been
     * destroyed. A subclass can override this method to take action after the
     * loss of connectivity with the Glacier2 router. This method is called
     * according to the Ice invocation dipsatch rules (in other words, it
     * uses the same rules as an servant upcall or AMI callback).
     **/
    virtual void sessionDestroyed();

    /**
     * Called to restart the application's Glacier2 session. This
     * method never returns. The exception produce an application restart
     * when called from the Application main thread.
     *
     * @throws RestartSessionException This exception is always thrown.
     **/
    static void restart();

    /**
     * Returns the Glacier2 router proxy
     * @return The router proxy.
     **/
    static Glacier2::RouterPrxPtr router();

    /**
     * Returns the Glacier2 session proxy
     * @return The session proxy.
     **/
    static Glacier2::SessionPrxPtr session();

    /**
     * Returns the category to be used in the identities of all of the client's
     * callback objects. Clients must use this category for the router to
     * forward callback requests to the intended client.
     * @return The category.
     * @throws SessionNotExistException No session exists.
     **/
    static std::string categoryForClient();

    /**
     * Create a new Ice identity for callback objects with the given
     * identity name field.
     * @return The identity.
     **/
    static Ice::Identity createCallbackIdentity(const std::string&);

    /**
     * Adds a servant to the callback object adapter's Active Servant Map with a UUID.
     * @param servant The servant to add.
     * @return The proxy for the servant.
     **/
    static Ice::ObjectPrxPtr addWithUUID(const Ice::ObjectPtr& servant);

    /**
     * Creates an object adapter for callback objects.
     * @return The object adapter.
     */
    static Ice::ObjectAdapterPtr objectAdapter();

protected:

    /**
     * Helper function that implements the application logic.
     */
    virtual int doMain(int argc, char* argv[], const Ice::InitializationData& initData, int version);

private:

    bool doMain(Ice::StringSeq&, const Ice::InitializationData&, int&, int);

    //
    // Run should not be overridden for Glacier2::Application. Instead
    // runWithSession should be used.
    //
    int run(int, char*[])
    {
        // This shouldn't be called.
        assert(false);
        return 0;
    }

    static Ice::ObjectAdapterPtr _adapter;
    static Glacier2::RouterPrxPtr _router;
    static Glacier2::SessionPrxPtr _session;
    static std::string _category;
};

}

#endif
