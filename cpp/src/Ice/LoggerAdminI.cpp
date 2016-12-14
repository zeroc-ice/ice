// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/LoggerAdminI.h>
#include <Ice/Initialize.h>
#include <Ice/Communicator.h>
#include <Ice/RemoteLogger.h>
#include <Ice/Properties.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/Connection.h>
#include <Ice/LocalException.h>
#include <Ice/LoggerUtil.h>

#include <set>

using namespace Ice;
using namespace std;

namespace
{

const char* traceCategory = "Admin.Logger";

class LoggerAdminI : public Ice::LoggerAdmin
#ifdef ICE_CPP11_MAPPING
                   , public std::enable_shared_from_this<LoggerAdminI>
#endif
{
public:

    LoggerAdminI(const PropertiesPtr&);

#ifdef ICE_CPP11_MAPPING
    virtual void attachRemoteLogger(shared_ptr<RemoteLoggerPrx>, LogMessageTypeSeq,
                                    StringSeq, Int, const Current&);

    virtual bool detachRemoteLogger(shared_ptr<RemoteLoggerPrx>, const Current&);

    virtual LogMessageSeq getLog(LogMessageTypeSeq, StringSeq, Int, string&, const Current&);
#else
    virtual void attachRemoteLogger(const RemoteLoggerPrx&, const LogMessageTypeSeq&,
                                    const StringSeq&, Int, const Current&);

    virtual bool detachRemoteLogger(const RemoteLoggerPrx&, const Current&);

    virtual LogMessageSeq getLog(const LogMessageTypeSeq&, const StringSeq&, Int, string&, const Current&);
#endif

    void destroy();

    vector<RemoteLoggerPrxPtr> log(const LogMessage&);

    void deadRemoteLogger(const RemoteLoggerPrxPtr&, const LoggerPtr&, const LocalException&, const string&);

    int getTraceLevel() const
    {
        return _traceLevel;
    }

private:

    bool removeRemoteLogger(const RemoteLoggerPrxPtr&);

#ifndef ICE_CPP11_MAPPING // C++98 mapping begin_init callback
    void initCompleted(const AsyncResultPtr&);
#endif

    IceUtil::Mutex _mutex;
    list<LogMessage> _queue;
    int _logCount; // non-trace messages
    const int _maxLogCount;
    int _traceCount;
    const int _maxTraceCount;
    const int _traceLevel;

    list<LogMessage>::iterator _oldestTrace;
    list<LogMessage>::iterator _oldestLog;

    struct ObjectIdentityCompare
    {
        bool operator()(const RemoteLoggerPrxPtr& lhs, const RemoteLoggerPrxPtr& rhs) const
        {
            //
            // Caller should make sure that proxies are never null
            //
            assert(lhs != 0 && rhs != 0);

            return lhs->ice_getIdentity() < rhs->ice_getIdentity();
        }
    };

    struct Filters
    {
        Filters(const LogMessageTypeSeq& m, const StringSeq& c) :
            messageTypes(m.begin(), m.end()),
            traceCategories(c.begin(), c.end())
        {
        }

        const set<LogMessageType> messageTypes;
        const set<string> traceCategories;
    };

    typedef map<RemoteLoggerPrxPtr, Filters, ObjectIdentityCompare> RemoteLoggerMap;

    struct GetRemoteLoggerMapKey
    {
        RemoteLoggerMap::key_type
        operator()(const RemoteLoggerMap::value_type& val)
        {
            return val.first;
        }
    };

    RemoteLoggerMap _remoteLoggerMap;
    CommunicatorPtr _sendLogCommunicator;
    bool _destroyed;
};
ICE_DEFINE_PTR(LoggerAdminIPtr, LoggerAdminI);


class Job : public IceUtil::Shared
{
public:

    Job(const vector<RemoteLoggerPrxPtr>& r, const LogMessage& l) :
        remoteLoggers(r),
        logMessage(l)
    {
    }

    const vector<RemoteLoggerPrxPtr> remoteLoggers;
    const LogMessage logMessage;
};
typedef IceUtil::Handle<Job> JobPtr;


class LoggerAdminLoggerI : public IceInternal::LoggerAdminLogger
#ifdef ICE_CPP11_MAPPING
                         , public std::enable_shared_from_this<LoggerAdminLoggerI>
#endif

{
public:

    LoggerAdminLoggerI(const PropertiesPtr&, const LoggerPtr&);

    virtual void print(const std::string&);
    virtual void trace(const std::string&, const std::string&);
    virtual void warning(const std::string&);
    virtual void error(const std::string&);
    virtual std::string getPrefix();
    virtual LoggerPtr cloneWithPrefix(const std::string&);

    virtual ObjectPtr getFacet() const;

    virtual void destroy();

    const LoggerPtr& getLocalLogger() const
    {
        return _localLogger;
    }

    void run();

private:

    void log(const LogMessage&);
#ifndef ICE_CPP11_MAPPING // C++98 mapping begin_log callback
    void logCompleted(const AsyncResultPtr&);
#endif

    LoggerPtr _localLogger;
    const LoggerAdminIPtr _loggerAdmin;

    IceUtil::Monitor<IceUtil::Mutex> _monitor;

    bool _destroyed;
    IceUtil::ThreadPtr _sendLogThread;
    std::deque<JobPtr> _jobQueue;
};
ICE_DEFINE_PTR(LoggerAdminLoggerIPtr, LoggerAdminLoggerI);


class SendLogThread : public IceUtil::Thread
{
public:

    SendLogThread(const LoggerAdminLoggerIPtr&);

    virtual void run();

private:

    LoggerAdminLoggerIPtr _logger;
};


//
// Helper functions
//

//
// Filter out messages from in/out logMessages list
//
void
filterLogMessages(LogMessageSeq& logMessages, const set<LogMessageType>& messageTypes,
                  const set<string>& traceCategories, Int messageMax)
{
    assert(!logMessages.empty() && messageMax != 0);

    //
    // Filter only if one of the 3 filters is set; messageMax < 0 means "give me all"
    // that match the other filters, if any.
    //
    if(!messageTypes.empty() || !traceCategories.empty() || messageMax > 0)
    {
        int count = 0;
        LogMessageSeq::reverse_iterator p = logMessages.rbegin();
        while(p != logMessages.rend())
        {
            bool keepIt = false;
            if(messageTypes.empty() || messageTypes.count(p->type) != 0)
            {
                if(p->type != ICE_ENUM(LogMessageType, TraceMessage) || traceCategories.empty() ||
                   traceCategories.count(p->traceCategory) != 0)
                {
                    keepIt = true;
                }
            }

            if(keepIt)
            {
                ++p;
                ++count;
                if(messageMax > 0 && count >= messageMax)
                {
                    //
                    // p.base() points to p "+1"; note that this invalidates p.
                    //
                    logMessages.erase(logMessages.begin(), p.base());
                    break; // while
                }
            }
            else
            {
                ++p;
                //
                // p.base() points to p "+1"; the erase invalidates p so we
                // need to rebuild it
                //
                p = LogMessageSeq::reverse_iterator(logMessages.erase(p.base()));
            }
        }
    }
    // else, don't need any filtering
}

//
// Change this proxy's communicator, while keeping its invocation timeout
//
RemoteLoggerPrxPtr
changeCommunicator(const RemoteLoggerPrxPtr& prx, const CommunicatorPtr& communicator)
{
    if(prx == 0)
    {
        return 0;
    }

    RemoteLoggerPrxPtr result = ICE_UNCHECKED_CAST(RemoteLoggerPrx, communicator->stringToProxy(prx->ice_toString()));

    return result->ice_invocationTimeout(prx->ice_getInvocationTimeout());
}

//
// Copies a set of properties
//
void
copyProperties(const string& prefix, const PropertiesPtr& from, const PropertiesPtr& to)
{
    PropertyDict dict = from->getPropertiesForPrefix(prefix);
    for(PropertyDict::const_iterator p = dict.begin(); p != dict.end(); ++p)
    {
        to->setProperty(p->first, p->second);
    }
}

//
// Create communicator used to send logs
//
CommunicatorPtr
createSendLogCommunicator(const CommunicatorPtr& communicator, const LoggerPtr& logger)
{
    InitializationData initData;
    initData.logger = logger;
    initData.properties = createProperties();

    PropertiesPtr mainProps = communicator->getProperties();

    copyProperties("Ice.Default.Locator", mainProps, initData.properties);
    copyProperties("Ice.Plugin.IceSSL", mainProps, initData.properties);
    copyProperties("IceSSL.", mainProps, initData.properties);

    StringSeq extraProps = mainProps->getPropertyAsList("Ice.Admin.Logger.Properties");

    if(!extraProps.empty())
    {
        for(vector<string>::iterator p = extraProps.begin(); p != extraProps.end(); ++p)
        {
            if(p->find("--") != 0)
            {
                *p = "--" + *p;
            }
        }
        initData.properties->parseCommandLineOptions("", extraProps);
    }

    return initialize(initData);
}

//
// LoggerAdminI
//

LoggerAdminI::LoggerAdminI(const PropertiesPtr& props) :
    _logCount(0),
    _maxLogCount(props->getPropertyAsIntWithDefault("Ice.Admin.Logger.KeepLogs", 100)),
    _traceCount(0),
    _maxTraceCount(props->getPropertyAsIntWithDefault("Ice.Admin.Logger.KeepTraces", 100)),
    _traceLevel(props->getPropertyAsInt("Ice.Trace.Admin.Logger")),
    _destroyed(false)
{
    _oldestLog = _queue.end();
    _oldestTrace = _queue.end();
}

void
#ifdef ICE_CPP11_MAPPING
LoggerAdminI::attachRemoteLogger(shared_ptr<RemoteLoggerPrx> prx,
                                 LogMessageTypeSeq messageTypes,
                                 StringSeq categories,
                                 Int messageMax,
                                 const Current& current)
#else
LoggerAdminI::attachRemoteLogger(const RemoteLoggerPrx& prx,
                                 const LogMessageTypeSeq& messageTypes,
                                 const StringSeq& categories,
                                 Int messageMax,
                                 const Current& current)
#endif
{
    if(!prx)
    {
        return; // can't send this null RemoteLogger anything!
    }

    //
    // In C++, LoggerAdminI does not keep a "logger" data member to avoid a hard-to-break circular
    // reference, so we retrieve the logger from Current
    //

    LoggerAdminLoggerIPtr logger = ICE_DYNAMIC_CAST(LoggerAdminLoggerI, current.adapter->getCommunicator()->getLogger());
    if(!logger)
    {
        // Our logger is not installed - must be a bug
        assert(0);
        return;
    }

    RemoteLoggerPrxPtr remoteLogger = prx->ice_twoway();

    Filters filters(messageTypes, categories);
    LogMessageSeq initLogMessages;
    {
        IceUtil::Mutex::Lock lock(_mutex);

        if(!_sendLogCommunicator)
        {
            if(_destroyed)
            {
                throw Ice::ObjectNotExistException(__FILE__, __LINE__);
            }

            _sendLogCommunicator =
                createSendLogCommunicator(current.adapter->getCommunicator(), logger->getLocalLogger());
        }

        if(!_remoteLoggerMap.insert(make_pair(changeCommunicator(remoteLogger, _sendLogCommunicator), filters)).second)
        {
            if(_traceLevel > 0)
            {
                Trace trace(logger, traceCategory);
                trace << "rejecting `" << remoteLogger << "' with RemoteLoggerAlreadyAttachedException";
            }

            throw RemoteLoggerAlreadyAttachedException();
        }

        if(messageMax != 0)
        {
            initLogMessages = _queue; // copy
        }
    }

    if(_traceLevel > 0)
    {
        Trace trace(logger, traceCategory);
        trace << "attached `" << remoteLogger << "'";
    }

    if(!initLogMessages.empty())
    {
        filterLogMessages(initLogMessages, filters.messageTypes, filters.traceCategories, messageMax);
    }

#ifdef ICE_CPP11_MAPPING
    try
    {
        auto self = shared_from_this();
        remoteLogger->initAsync(logger->getPrefix(), initLogMessages,
            [self, logger, remoteLogger]()
            {
                if(self->_traceLevel > 1)
                {
                    Trace trace(logger, traceCategory);
                    trace << "init on `" << remoteLogger << "' completed successfully";
                }
            },
            [self, logger, remoteLogger](exception_ptr e)
            {
                try
                {
                    rethrow_exception(e);
                }
                catch(const Ice::LocalException& e)
                {
                    self->deadRemoteLogger(remoteLogger, logger, e, "init");
                }
            });
    }
    catch(const LocalException& ex)
    {
        deadRemoteLogger(remoteLogger, logger, ex, "init");
        throw;
    }
#else
    CallbackPtr initCompletedCb = newCallback(this, &LoggerAdminI::initCompleted);
    try
    {
        remoteLogger->begin_init(logger->getPrefix(), initLogMessages, initCompletedCb, logger);
    }
    catch(const LocalException& ex)
    {
        deadRemoteLogger(remoteLogger, logger, ex, "init");
        throw;
    }
#endif
}

bool
#ifdef ICE_CPP11_MAPPING
LoggerAdminI::detachRemoteLogger(shared_ptr<RemoteLoggerPrx> remoteLogger, const Current& current)
#else
LoggerAdminI::detachRemoteLogger(const RemoteLoggerPrx& remoteLogger, const Current& current)
#endif
{
    if(remoteLogger == 0)
    {
        return false;
    }

    //
    // No need to convert the proxy as we only use its identity
    //
    bool found = removeRemoteLogger(remoteLogger);

    if(_traceLevel > 0)
    {
        Trace trace(current.adapter->getCommunicator()->getLogger(), traceCategory);
        if(found)
        {
            trace << "detached `" << remoteLogger << "'";
        }
        else
        {
            trace << "cannot detach `" << remoteLogger << "': not found";
        }
    }

    return found;
}

LogMessageSeq
#ifdef ICE_CPP11_MAPPING
LoggerAdminI::getLog(LogMessageTypeSeq messageTypes, StringSeq categories,
                     Int messageMax, string& prefix, const Current& current)
#else
LoggerAdminI::getLog(const LogMessageTypeSeq& messageTypes, const StringSeq& categories,
                     Int messageMax, string& prefix, const Current& current)
#endif
{
    LogMessageSeq logMessages;
    {
        IceUtil::Mutex::Lock lock(_mutex);

        if(messageMax != 0)
        {
            logMessages = _queue;
        }
    }

    LoggerPtr logger = current.adapter->getCommunicator()->getLogger();
    prefix = logger->getPrefix();

    if(!logMessages.empty())
    {
        Filters filters(messageTypes, categories);
        filterLogMessages(logMessages, filters.messageTypes, filters.traceCategories, messageMax);
    }

    return logMessages;
}

void
LoggerAdminI::destroy()
{
    CommunicatorPtr sendLogCommunicator;
    {
        IceUtil::Mutex::Lock lock(_mutex);
        if(!_destroyed)
        {
            _destroyed = true;
            sendLogCommunicator = _sendLogCommunicator;
            _sendLogCommunicator = 0;
        }
    }

    //
    // Destroy outside lock to avoid deadlock when there are outstanding two-way log calls sent to
    // remote logggers
    //
    if(sendLogCommunicator)
    {
        sendLogCommunicator->destroy();
    }
}

vector<RemoteLoggerPrxPtr>
LoggerAdminI::log(const LogMessage& logMessage)
{
    vector<RemoteLoggerPrxPtr> remoteLoggers;

    IceUtil::Mutex::Lock lock(_mutex);

    //
    // Put message in _queue
    //
    if((logMessage.type != ICE_ENUM(LogMessageType, TraceMessage) && _maxLogCount > 0) ||
       (logMessage.type == ICE_ENUM(LogMessageType, TraceMessage) && _maxTraceCount > 0))
    {
        list<LogMessage>::iterator p = _queue.insert(_queue.end(), logMessage);

        if(logMessage.type != ICE_ENUM(LogMessageType, TraceMessage))
        {
            assert(_maxLogCount > 0);
            if(_logCount == _maxLogCount)
            {
                //
                // Need to remove the oldest log from the queue
                //
                assert(_oldestLog != _queue.end());
                _oldestLog = _queue.erase(_oldestLog);
                while(_oldestLog != _queue.end() && _oldestLog->type == ICE_ENUM(LogMessageType, TraceMessage))
                {
                    _oldestLog++;
                }
                assert(_oldestLog != _queue.end());
            }
            else
            {
                assert(_logCount < _maxLogCount);
                _logCount++;
                if(_oldestLog == _queue.end())
                {
                    _oldestLog = p;
                }
            }
        }
        else
        {
            assert(_maxTraceCount > 0);
            if(_traceCount == _maxTraceCount)
            {
                //
                // Need to remove the oldest trace from the queue
                //
                assert(_oldestTrace != _queue.end());
                _oldestTrace = _queue.erase(_oldestTrace);
                while(_oldestTrace != _queue.end() && _oldestTrace->type != ICE_ENUM(LogMessageType, TraceMessage))
                {
                    _oldestTrace++;
                }
                assert(_oldestTrace != _queue.end());
            }
            else
            {
                assert(_traceCount < _maxTraceCount);
                _traceCount++;
                if(_oldestTrace == _queue.end())
                {
                    _oldestTrace = p;
                }
            }
        }

        //
        // Queue updated, now find which remote loggers want this message
        //
        for(RemoteLoggerMap::const_iterator p = _remoteLoggerMap.begin(); p != _remoteLoggerMap.end(); ++p)
        {
            const Filters& filters = p->second;

            if(filters.messageTypes.empty() || filters.messageTypes.count(logMessage.type) != 0)
            {
                if(logMessage.type != ICE_ENUM(LogMessageType, TraceMessage) || filters.traceCategories.empty() ||
                   filters.traceCategories.count(logMessage.traceCategory) != 0)
                {
                    remoteLoggers.push_back(p->first);
                }
            }
        }
    }
    return remoteLoggers;
}

void
LoggerAdminI::deadRemoteLogger(const RemoteLoggerPrxPtr& remoteLogger,
                               const LoggerPtr& logger,
                               const LocalException& ex,
                               const string& operation)
{
    //
    // No need to convert remoteLogger as we only use its identity
    //
    if(removeRemoteLogger(remoteLogger))
    {
        if(_traceLevel > 0)
        {
            Trace trace(logger, traceCategory);
            trace << "detached `" << remoteLogger << "' because " << operation << " raised:\n" << ex;
        }
    }
}

bool
LoggerAdminI::removeRemoteLogger(const RemoteLoggerPrxPtr& remoteLogger)
{
    IceUtil::Mutex::Lock lock(_mutex);
    return _remoteLoggerMap.erase(remoteLogger) > 0;
}

#ifndef ICE_CPP11_MAPPING
//
// begin_init callback method for C++98 mapping
//
void
LoggerAdminI::initCompleted(const AsyncResultPtr& r)
{
    RemoteLoggerPrxPtr remoteLogger = ICE_UNCHECKED_CAST(RemoteLoggerPrx, r->getProxy());

    try
    {
        remoteLogger->end_init(r);

        if(_traceLevel > 1)
        {
            LoggerPtr logger = ICE_DYNAMIC_CAST(Logger, r->getCookie());
            Trace trace(logger, traceCategory);
            trace << r->getOperation() << " on `" << remoteLogger << "' completed successfully";
        }
    }
    catch(const LocalException& ex)
    {
        deadRemoteLogger(remoteLogger, ICE_DYNAMIC_CAST(Logger, r->getCookie()), ex, r->getOperation());
    }
}
#endif

//
// LoggerAdminLoggerI
//

LoggerAdminLoggerI::LoggerAdminLoggerI(const PropertiesPtr& props,
                                       const LoggerPtr& localLogger) :
    _loggerAdmin(new LoggerAdminI(props)),
    _destroyed(false)
{
    //
    // There is currently no way to have a null local logger
    //
    assert(localLogger != 0);

    LoggerAdminLoggerI* wrapper = dynamic_cast<LoggerAdminLoggerI*>(localLogger.get());
    if(wrapper)
    {
        // use the underlying local logger
       _localLogger = wrapper->getLocalLogger();
    }
    else
    {
        _localLogger = localLogger;
    }
}

void
LoggerAdminLoggerI::print(const string& message)
{
   LogMessage logMessage = { ICE_ENUM(LogMessageType, PrintMessage), IceUtil::Time::now().toMicroSeconds(), "", message };

    _localLogger->print(message);
    log(logMessage);
}

void
LoggerAdminLoggerI::trace(const string& category, const string& message)
{
    LogMessage logMessage = { ICE_ENUM(LogMessageType, TraceMessage), IceUtil::Time::now().toMicroSeconds(), category, message };

    _localLogger->trace(category, message);
    log(logMessage);
}

void
LoggerAdminLoggerI::warning(const string& message)
{
    LogMessage logMessage = { ICE_ENUM(LogMessageType, WarningMessage), IceUtil::Time::now().toMicroSeconds(), "", message };

    _localLogger->warning(message);
    log(logMessage);
}

void
LoggerAdminLoggerI::error(const string& message)
{
    LogMessage logMessage = { ICE_ENUM(LogMessageType, ErrorMessage), IceUtil::Time::now().toMicroSeconds(), "", message };

    _localLogger->error(message);
    log(logMessage);
}

string
LoggerAdminLoggerI::getPrefix()
{
    return _localLogger->getPrefix();
}

LoggerPtr
LoggerAdminLoggerI::cloneWithPrefix(const string& prefix)
{
    return _localLogger->cloneWithPrefix(prefix);
}

ObjectPtr
LoggerAdminLoggerI::getFacet() const
{
    return _loggerAdmin;
}

void
LoggerAdminLoggerI::log(const LogMessage& logMessage)
{
    const vector<RemoteLoggerPrxPtr> remoteLoggers = _loggerAdmin->log(logMessage);

    if(!remoteLoggers.empty())
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);

        if(!_sendLogThread)
        {
            _sendLogThread = new SendLogThread(ICE_SHARED_FROM_THIS);
            _sendLogThread->start();
        }

        _jobQueue.push_back(new Job(remoteLoggers, logMessage));
        _monitor.notifyAll();
    }
}

void
LoggerAdminLoggerI::destroy()
{
    IceUtil::ThreadControl sendLogThreadControl;
    bool joinThread = false;
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);

        if(_sendLogThread)
        {
            joinThread = true;
            sendLogThreadControl = _sendLogThread->getThreadControl();
            _sendLogThread = 0;
            _destroyed = true;
            _monitor.notifyAll();
        }
    }

    if(joinThread)
    {
        sendLogThreadControl.join();
    }

     // destroy sendLogCommunicator
    _loggerAdmin->destroy();
}

void
LoggerAdminLoggerI::run()
{
    if(_loggerAdmin->getTraceLevel() > 1)
    {
        Trace trace(_localLogger, traceCategory);
        trace << "send log thread started";
    }

#ifndef ICE_CPP11_MAPPING
    CallbackPtr logCompletedCb = newCallback(this, &LoggerAdminLoggerI::logCompleted);
#endif

    for(;;)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);
        while(!_destroyed && _jobQueue.empty())
        {
            _monitor.wait();
        }
        if(_destroyed)
        {
            break; // for(;;)
        }

        assert(!_jobQueue.empty());
        JobPtr job = _jobQueue.front();
        _jobQueue.pop_front();
        lock.release();

        for(vector<RemoteLoggerPrxPtr>::const_iterator p = job->remoteLoggers.begin(); p != job->remoteLoggers.end(); ++p)
        {
            if(_loggerAdmin->getTraceLevel() > 1)
            {
                Trace trace(_localLogger, traceCategory);
                trace << "sending log message to `" << *p << "'";
            }

            try
            {
#ifdef ICE_CPP11_MAPPING
                RemoteLoggerPrxPtr remoteLogger = *p;
                auto self = shared_from_this();
                remoteLogger->logAsync(job->logMessage,
                    [self, remoteLogger]()
                    {
                        if(self->_loggerAdmin->getTraceLevel() > 1)
                        {
                            Trace trace(self->_localLogger, traceCategory);
                            trace << "log on `" << remoteLogger << "' completed successfully";
                        }
                    },
                    [self, remoteLogger](exception_ptr e)
                    {
                        try
                        {
                            rethrow_exception(e);
                        }
                        catch(const CommunicatorDestroyedException&)
                        {
                            // expected if there are outstanding calls during communicator destruction
                        }
                        catch(const LocalException& ex)
                        {
                            self->_loggerAdmin->deadRemoteLogger(remoteLogger, self->_localLogger, ex, "log");
                        }
                    });
#else
                //
                // *p is a proxy associated with the _sendLogCommunicator
                //
                (*p)->begin_log(job->logMessage, logCompletedCb);
#endif
            }
            catch(const LocalException& ex)
            {
                _loggerAdmin->deadRemoteLogger(*p, _localLogger, ex, "log");
            }
        }
    }

    if(_loggerAdmin->getTraceLevel() > 1)
    {
        Trace trace(_localLogger, traceCategory);
        trace << "send log thread completed";
    }
}

#ifndef ICE_CPP11_MAPPING
//
// begin_log callback for C++98 mapping
//
void
LoggerAdminLoggerI::logCompleted(const AsyncResultPtr& r)
{
    RemoteLoggerPrxPtr remoteLogger = ICE_UNCHECKED_CAST(RemoteLoggerPrx, r->getProxy());

    try
    {
        remoteLogger->end_log(r);

        if(_loggerAdmin->getTraceLevel() > 1)
        {
            Trace trace(_localLogger, traceCategory);
            trace << r->getOperation() << " on `" << remoteLogger << "' completed successfully";
        }
    }
    catch(const CommunicatorDestroyedException&)
    {
        // expected if there are outstanding calls during communicator destruction
    }
    catch(const LocalException& ex)
    {
        _loggerAdmin->deadRemoteLogger(remoteLogger, _localLogger, ex, r->getOperation());
    }
}
#endif

//
// SendLogThread
//

SendLogThread::SendLogThread(const LoggerAdminLoggerIPtr& logger) :
    IceUtil::Thread("Ice.SendLogThread"),
    _logger(logger)
{
}

void
SendLogThread::run()
{
    _logger->run();
}
}

//
// createLoggerAdminLogger
//

namespace IceInternal
{

LoggerAdminLoggerPtr
createLoggerAdminLogger(const PropertiesPtr& props,
                        const LoggerPtr& localLogger)
{
    return ICE_MAKE_SHARED(LoggerAdminLoggerI, props, localLogger);
}

}
