// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef ICE_APPLICATION_H
#define ICE_APPLICATION_H

#include <Ice/Initialize.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/Cond.h>
#include <IceUtil/CtrlCHandler.h>

namespace Ice
{

using IceUtil::CtrlCHandler;
using IceUtil::CtrlCHandlerCallback;

#ifdef ICE_CPP11_MAPPING
/**
 * Determines how the Application class handles signals.
 */
enum class SignalPolicy : unsigned char
#else
enum SignalPolicy
#endif
{
    /** Enables signal handling. */
    HandleSignals,
    /** Disables signal handling, meaning signals retain their default behavior. */
    NoSignalHandling
};

/**
 * Singleton helper class that simplifies Ice initialization, finalization and signal handling.
 * An application uses this class by writing a subclass and implementing the run method.
 * \headerfile Ice/Ice.h
 */
class ICE_API Application
{
public:

    /**
     * The constructor configures the signal handling behavior.
     * @param policy Specifies whether to handle signals. If not specified, the default behavior
     * is to handle signals.
     */
    Application(SignalPolicy policy = ICE_ENUM(SignalPolicy, HandleSignals));

#ifdef ICE_CPP11_MAPPING
    /// \cond IGNORE
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    /// \endcond
#endif

    virtual ~Application();

    /**
     * Call this main() from the global main(). main()
     * initializes the Communicator, calls run() and destroys the
     * the Communicator upon return from run(). It handles all
     * exceptions properly, i.e., error message are printed if
     * exceptions propagate to main(), and the Communicator is always
     * destroyed, regardless of exceptions.
     * @param argc Specifies the number of arguments in argv.
     * @param argv The command-line arguments.
     * @param initData Configuration data for the new Communicator.
     * @param version Indicates the Ice version with which the application is compatible. If not
     * specified, the version of the Ice installation is used.
     * @return The application's exit status.
     */
    int main(int argc, const char* const argv[], const InitializationData& initData = InitializationData(),
             int version = ICE_INT_VERSION);

    /**
     * Call this main() from the global main(). main()
     * initializes the Communicator, calls run() and destroys the
     * the Communicator upon return from run(). It handles all
     * exceptions properly, i.e., error message are printed if
     * exceptions propagate to main(), and the Communicator is always
     * destroyed, regardless of exceptions.
     * @param argc Specifies the number of arguments in argv.
     * @param argv The command-line arguments.
     * @param configFile The name of an Ice configuration file.
     * @param version Indicates the Ice version with which the application is compatible. If not
     * specified, the version of the Ice installation is used.
     * @return The application's exit status.
     */
    int main(int argc, const char* const argv[], ICE_CONFIG_FILE_STRING configFile, int version = ICE_INT_VERSION);

#ifdef _WIN32
    /**
     * Call this main() from the global main(). main()
     * initializes the Communicator, calls run() and destroys the
     * the Communicator upon return from run(). It handles all
     * exceptions properly, i.e., error message are printed if
     * exceptions propagate to main(), and the Communicator is always
     * destroyed, regardless of exceptions.
     * @param argc Specifies the number of arguments in argv.
     * @param argv The command-line arguments.
     * @param initData Configuration data for the new Communicator.
     * @param version Indicates the Ice version with which the application is compatible. If not
     * specified, the version of the Ice installation is used.
     * @return The application's exit status.
     */
    int main(int argc, const wchar_t* const argv[], const InitializationData& initData = InitializationData(),
             int version = ICE_INT_VERSION);

    /**
     * Call this main() from the global main(). main()
     * initializes the Communicator, calls run() and destroys the
     * the Communicator upon return from run(). It handles all
     * exceptions properly, i.e., error message are printed if
     * exceptions propagate to main(), and the Communicator is always
     * destroyed, regardless of exceptions.
     * @param argc Specifies the number of arguments in argv.
     * @param argv The command-line arguments.
     * @param configFile The name of an Ice configuration file.
     * @param version Indicates the Ice version with which the application is compatible. If not
     * specified, the version of the Ice installation is used.
     * @return The application's exit status.
     */
    int main(int argc, const wchar_t* const argv[], ICE_CONFIG_FILE_STRING configFile, int version = ICE_INT_VERSION);
#endif

    /**
     * Call this main() from the global main(). main()
     * initializes the Communicator, calls run() and destroys the
     * the Communicator upon return from run(). It handles all
     * exceptions properly, i.e., error message are printed if
     * exceptions propagate to main(), and the Communicator is always
     * destroyed, regardless of exceptions.
     * @param args The command-line arguments.
     * @param initData Configuration data for the new Communicator.
     * @param version Indicates the Ice version with which the application is compatible. If not
     * specified, the version of the Ice installation is used.
     * @return The application's exit status.
     */
    int main(const StringSeq& args, const InitializationData& initData = InitializationData(),
             int version = ICE_INT_VERSION);

    /**
     * Call this main() from the global main(). main()
     * initializes the Communicator, calls run() and destroys the
     * the Communicator upon return from run(). It handles all
     * exceptions properly, i.e., error message are printed if
     * exceptions propagate to main(), and the Communicator is always
     * destroyed, regardless of exceptions.
     * @param args The command-line arguments.
     * @param configFile The name of an Ice configuration file.
     * @param version Indicates the Ice version with which the application is compatible. If not
     * specified, the version of the Ice installation is used.
     * @return The application's exit status.
     */
    int main(const StringSeq& args, ICE_CONFIG_FILE_STRING configFile, int version = ICE_INT_VERSION);

    /**
     * run is given a copy of the remaining argc/argv arguments,
     * after the communicator initialization in the caller (main)
     * has removed all Ice-related arguments.
     * @param argc Specifies the number of arguments in argv.
     * @param argv The command-line arguments.
     * @return The application's exit status.
     */
    virtual int run(int argc, char* argv[]) = 0;

    /**
     * Override this method to provide a custom application interrupt
     * hook. You must call callbackOnInterrupt for this method to
     * be called. Note that the interruptCallback can be called
     * concurrently with any other thread (including main) in your
     * application and thus must take appropriate concurrency
     * precautions.
     * @param signal The signal identifier.
     */
    virtual void interruptCallback(int signal);

    /**
     * Obtains the application name, i.e., argv[0].
     * @return The application's name.
     */
    static const char* appName();

    /**
     * Obtains the application's Communicator instance.
     * One limitation of this class is that there can only be one
     * Application instance, with one global Communicator, accessible
     * with this communicator() operation. This limitation is due to
     * how the signal handling functions below operate. If you require
     * multiple Communicators, then you cannot use this Application
     * framework class.
     * @return The application's communicator.
     */
    static CommunicatorPtr communicator();

    /**
     * Configures the application to destroy the communicator when one of the
     * monitored signals is raised. This is the default behavior.
     */
    static void destroyOnInterrupt();

    /**
     * Configures the application to shut down the communicator when one of the
     * monitored signals is raised.
     */
    static void shutdownOnInterrupt();

    /**
     * Configures the application to ignore signals.
     */
    static void ignoreInterrupt();

    /**
     * Configures the application to invoke interruptCallback when a signal occurs,
     * thereby giving the subclass responsibility for handling the signal.
     */
    static void callbackOnInterrupt();

    /**
     * Configures the application to ignore (but remember) a signal.
     * A stored signal (if any) can be handled later by calling releaseInterrupt.
     */
    static void holdInterrupt();

    /**
     * Processes a stored signal (if any) using the current signal handling configuration.
     */
    static void releaseInterrupt();

    /**
     * Indicates whether a signal handler was triggered.
     * This can be used once Communicator::waitForShutdown() returns to
     * test whether the shutdown was due to an interrupt (returns true)
     * or because Communicator::shutdown() was called (returns false).
     * @return True if a signal handler was triggered, false otherwise.
     */
    static bool interrupted();

protected:

    /**
     * Helper function that implements the application logic.
     */
    virtual int doMain(int, char*[], const InitializationData&, Int);

    /**
     * Used to synchronize the main thread and the CtrlCHandler thread.
     */
    static IceUtil::Mutex _mutex;

    /**
     * Used to synchronize the main thread and the CtrlCHandler thread.
     */
    static IceUtil::Cond _condVar;

    /**
     * True if a signal handling callback is currently executing.
     * Can change while run() and communicator->destroy() are running!
     */
    static bool _callbackInProgress;

    /**
     * True if the communicator has been destroyed.
     * Can change while run() and communicator->destroy() are running!
     */
    static bool _destroyed;

    /**
     * True if an interrupt signal was received.
     * Can change while run() and communicator->destroy() are running!
     */
    static bool _interrupted;

    /**
     * The application's name.
     * Immutable during run() and until communicator->destroy() has returned.
     * Before and after run(), and once communicator->destroy() has returned,
     * we assume that only the main thread and CtrlCHandler threads are running.
     */
    static std::string _appName;

    /**
     * The application's communicator.
     * Immutable during run() and until communicator->destroy() has returned.
     * Before and after run(), and once communicator->destroy() has returned,
     * we assume that only the main thread and CtrlCHandler threads are running.
     */
    static CommunicatorPtr _communicator;

    /**
     * The signal-handling policy specified at construction.
     * Immutable during run() and until communicator->destroy() has returned.
     * Before and after run(), and once communicator->destroy() has returned,
     * we assume that only the main thread and CtrlCHandler threads are running.
     */
    static SignalPolicy _signalPolicy;

    /**
     * The singleton instance.
     * Immutable during run() and until communicator->destroy() has returned.
     * Before and after run(), and once communicator->destroy() has returned,
     * we assume that only the main thread and CtrlCHandler threads are running.
     */
    static Application* _application;

private:

    static void holdInterruptCallback(int);
    static void destroyOnInterruptCallback(int);
    static void shutdownOnInterruptCallback(int);
    static void callbackOnInterruptCallback(int);

#ifndef ICE_CPP11_MAPPING
    //
    // Not defined, make Application non-copyable
    //
    Application(const Application&);
    Application& operator=(const Application&);
#endif

};

}

#endif
