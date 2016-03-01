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
{
public:

    LoggerAdminI(const PropertiesPtr&);
    
    virtual void attachRemoteLogger(const RemoteLoggerPrx&, const LogMessageTypeSeq&, 
                                    const StringSeq&, Int, const Current&);
    
    virtual bool detachRemoteLogger(const RemoteLoggerPrx&, const Current&);
    
    virtual LogMessageSeq getLog(const LogMessageTypeSeq&, const StringSeq&, Int, string&, const Current&);

    void destroy();

    vector<RemoteLoggerPrx> log(const LogMessage&);

    void deadRemoteLogger(const RemoteLoggerPrx&, const LoggerPtr&, const LocalException&, const string&);

    const int getTraceLevel() const
    {
        return _traceLevel;
    }

private:

    bool removeRemoteLogger(const RemoteLoggerPrx&);
    
    void initCompleted(const AsyncResultPtr&);

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
        bool operator()(const RemoteLoggerPrx& lhs, const RemoteLoggerPrx& rhs) const
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

    typedef map<RemoteLoggerPrx, Filters, ObjectIdentityCompare> RemoteLoggerMap;

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
typedef IceUtil::Handle<LoggerAdminI> LoggerAdminIPtr;


class Job : public IceUtil::Shared
{
public:

    Job(const vector<RemoteLoggerPrx>& r, const LogMessage& l) :
        remoteLoggers(r),
        logMessage(l)
    {
    }

    const vector<RemoteLoggerPrx> remoteLoggers;
    const LogMessage logMessage;
};
typedef IceUtil::Handle<Job> JobPtr;


class LoggerAdminLoggerI : public IceInternal::LoggerAdminLogger
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

    void logCompleted(const AsyncResultPtr&);
    
    LoggerPtr _localLogger;
    const LoggerAdminIPtr _loggerAdmin;

    IceUtil::Monitor<IceUtil::Mutex> _monitor;

    bool _destroyed;
    IceUtil::ThreadPtr _sendLogThread;
    std::deque<JobPtr> _jobQueue;   
};
typedef IceUtil::Handle<LoggerAdminLoggerI> LoggerAdminLoggerIPtr;


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
                if(p->type != TraceMessage || traceCategories.empty() ||
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
RemoteLoggerPrx
changeCommunicator(const RemoteLoggerPrx& prx, const CommunicatorPtr& communicator)
{
    if(prx == 0)
    {
        return 0;
    }

    RemoteLoggerPrx result = RemoteLoggerPrx::uncheckedCast(communicator->stringToProxy(prx->ice_toString()));

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
LoggerAdminI::attachRemoteLogger(const RemoteLoggerPrx& prx,
                                 const LogMessageTypeSeq& messageTypes,
                                 const StringSeq& categories,
                                 Int messageMax,
                                 const Current& current)
{
    if(!prx)
    {
        return; // can't send this null RemoteLogger anything!
    }
    
    //
    // In C++, LoggerAdminI does not keep a "logger" data member to avoid a hard-to-break circular
    // reference, so we retrieve the logger from Current
    //
    
    LoggerAdminLoggerIPtr logger = LoggerAdminLoggerIPtr::dynamicCast(current.adapter->getCommunicator()->getLogger());
    if(!logger)
    {
        // Our logger is not installed - must be a bug
        assert(0);
        return;
    }

    RemoteLoggerPrx remoteLogger = prx->ice_twoway();

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
}

bool
LoggerAdminI::detachRemoteLogger(const RemoteLoggerPrx& remoteLogger, const Current& current)
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
LoggerAdminI::getLog(const LogMessageTypeSeq& messageTypes, const StringSeq& categories,
                     Int messageMax, string& prefix, const Current& current)
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

vector<RemoteLoggerPrx>
LoggerAdminI::log(const LogMessage& logMessage)
{
    vector<RemoteLoggerPrx> remoteLoggers;

    IceUtil::Mutex::Lock lock(_mutex);

    //
    // Put message in _queue
    //
    if((logMessage.type != TraceMessage && _maxLogCount > 0) ||
       (logMessage.type == TraceMessage && _maxTraceCount > 0))
    {
        list<LogMessage>::iterator p = _queue.insert(_queue.end(), logMessage);

        if(logMessage.type != TraceMessage)
        {
            assert(_maxLogCount > 0);
            if(_logCount == _maxLogCount)
            {
                //
                // Need to remove the oldest log from the queue
                //
                assert(_oldestLog != _queue.end());
                _oldestLog = _queue.erase(_oldestLog);
                while(_oldestLog != _queue.end() && _oldestLog->type == TraceMessage)
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
                while(_oldestTrace != _queue.end() && _oldestTrace->type != TraceMessage)
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
                if(logMessage.type != TraceMessage || filters.traceCategories.empty() ||
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
LoggerAdminI::deadRemoteLogger(const RemoteLoggerPrx& remoteLogger,
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
LoggerAdminI::removeRemoteLogger(const RemoteLoggerPrx& remoteLogger)
{
    IceUtil::Mutex::Lock lock(_mutex);
    return _remoteLoggerMap.erase(remoteLogger) > 0;
}

void
LoggerAdminI::initCompleted(const AsyncResultPtr& r)
{
    RemoteLoggerPrx remoteLogger = RemoteLoggerPrx::uncheckedCast(r->getProxy());
    
try 
    {
        remoteLogger->end_init(r);

        if(_traceLevel > 1)
        {
            LoggerPtr logger = LoggerPtr::dynamicCast(r->getCookie());
            Trace trace(logger, traceCategory);
            trace << r->getOperation() << " on `" << remoteLogger << "' completed successfully";
        }
    }
    catch(const LocalException& ex)
    {
        deadRemoteLogger(remoteLogger, LoggerPtr::dynamicCast(r->getCookie()), ex, r->getOperation());
    }
}

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
   LogMessage logMessage = { PrintMessage, IceUtil::Time::now().toMicroSeconds(), "", message };

    _localLogger->print(message);
    log(logMessage);
}

void
LoggerAdminLoggerI::trace(const string& category, const string& message)
{
    LogMessage logMessage = { TraceMessage, IceUtil::Time::now().toMicroSeconds(), category, message };

    _localLogger->trace(category, message);
    log(logMessage);
}

void
LoggerAdminLoggerI::warning(const string& message)
{
    LogMessage logMessage = { WarningMessage, IceUtil::Time::now().toMicroSeconds(), "", message };

    _localLogger->warning(message);
    log(logMessage);
}

void
LoggerAdminLoggerI::error(const string& message)
{
    LogMessage logMessage = { ErrorMessage, IceUtil::Time::now().toMicroSeconds(), "", message };

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
    const vector<RemoteLoggerPrx> remoteLoggers = _loggerAdmin->log(logMessage);

    if(!remoteLoggers.empty())
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);

        if(!_sendLogThread)
        {
            _sendLogThread = new SendLogThread(this);
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

    CallbackPtr logCompletedCb = newCallback(this, &LoggerAdminLoggerI::logCompleted);

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

        for(vector<RemoteLoggerPrx>::const_iterator p = job->remoteLoggers.begin(); p != job->remoteLoggers.end(); ++p)
        {
            if(_loggerAdmin->getTraceLevel() > 1)
            {
                Trace trace(_localLogger, traceCategory);
                trace << "sending log message to `" << *p << "'";
            }

            try
            {
                //
                // *p is a proxy associated with the _sendLogCommunicator
                //
                (*p)->begin_log(job->logMessage, logCompletedCb);
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

void
LoggerAdminLoggerI::logCompleted(const AsyncResultPtr& r)
{
    RemoteLoggerPrx remoteLogger = RemoteLoggerPrx::uncheckedCast(r->getProxy());
    
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
    return new LoggerAdminLoggerI(props, localLogger);
}

}
