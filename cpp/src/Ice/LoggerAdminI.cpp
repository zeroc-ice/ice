// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/LoggerAdminI.h>
#include <Ice/RemoteLogger.h>
#include <Ice/Properties.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/Communicator.h>
#include <Ice/Connection.h>
#include <Ice/LocalException.h>
#include <Ice/LoggerUtil.h>

#include <set>

using namespace std;

namespace
{

class LoggerAdminI : public Ice::LoggerAdmin
{
public:

    LoggerAdminI(const string&, const Ice::PropertiesPtr&);
    
    virtual void attachRemoteLogger(const Ice::RemoteLoggerPrx&, const Ice::LogMessageTypeSeq&, 
                                    const Ice::StringSeq&, Ice::Int, const Ice::Current&);
    
    virtual void detachRemoteLogger(const Ice::RemoteLoggerPrx&, const Ice::Current&);
    
    virtual Ice::LogMessageSeq getLog(const Ice::LogMessageTypeSeq&, 
                                      const Ice::StringSeq&, Ice::Int, string&,
                                      const Ice::Current&);
    
    vector<Ice::RemoteLoggerPrx> log(const Ice::LogMessage&);
    
    void deadRemoteLogger(const Ice::RemoteLoggerPrx&,
                          const Ice::LoggerPtr&, 
                          const Ice::LocalException&, const string&);

    void run(const Ice::LoggerPtr&);
    void destroy();

    const string& getFacetName() const
    {
        return _facetName;
    }

    const int getTraceLevel() const
    {
        return _traceLevel;
    }

private:
   
    bool removeRemoteLogger(const Ice::RemoteLoggerPrx&);
    
    void remoteCallCompleted(const Ice::AsyncResultPtr&);

    const string _facetName;

    IceUtil::Monitor<IceUtil::Mutex> _monitor;
    list<Ice::LogMessage> _queue;
    int _logCount; // non-trace messages
    const int _maxLogCount;
    int _traceCount;
    const int _maxTraceCount;
    const int _traceLevel;
    IceUtil::Time _pingPeriod;
    
    list<Ice::LogMessage>::iterator _oldestTrace;
    list<Ice::LogMessage>::iterator _oldestLog;

    struct ObjectIdentityCompare
    {
        bool operator()(const Ice::RemoteLoggerPrx& lhs, const Ice::RemoteLoggerPrx& rhs)
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
        Filters(const Ice::LogMessageTypeSeq& m, const Ice::StringSeq& c) :
            messageTypes(m.begin(), m.end()),
            traceCategories(c.begin(), c.end())
        {
        }
        
        const set<Ice::LogMessageType> messageTypes;
        const set<string> traceCategories;
    };

    typedef map<Ice::RemoteLoggerPrx, Filters, ObjectIdentityCompare> RemoteLoggerMap; 

    struct GetRemoteLoggerMapKey
    {
        RemoteLoggerMap::key_type 
        operator()(const RemoteLoggerMap::value_type& val)
        {
            return val.first;
        }
    };
    
    RemoteLoggerMap _remoteLoggerMap;

    const Ice::CallbackPtr _remoteCallCompleted;

    bool _destroyed;
    bool _pingThreadStarted;
    IceUtil::ThreadControl _pingThreadControl;
};
typedef IceUtil::Handle<LoggerAdminI> LoggerAdminIPtr;


class PingThread : public IceUtil::Thread
{
public:
    PingThread(const LoggerAdminIPtr&, const Ice::LoggerPtr&);

    virtual void run();

private:

    LoggerAdminIPtr _loggerAdmin;
    Ice::LoggerPtr _logger;
};


class Job : public IceUtil::Shared
{
public:
    
    Job(const vector<Ice::RemoteLoggerPrx>& r, const Ice::LogMessage& l) :
        remoteLoggers(r),
        logMessage(l)
    {
    }
    
    const vector<Ice::RemoteLoggerPrx> remoteLoggers;
    const Ice::LogMessage logMessage;   
};
typedef IceUtil::Handle<Job> JobPtr;


class LoggerAdminLoggerI : public Ice::LoggerAdminLogger
{
public:
    
    LoggerAdminLoggerI(const std::string&, const Ice::PropertiesPtr&, const Ice::LoggerPtr&);

    virtual void print(const std::string&);
    virtual void trace(const std::string&, const std::string&);
    virtual void warning(const std::string&);
    virtual void error(const std::string&);
    virtual std::string getPrefix();
    virtual Ice::LoggerPtr cloneWithPrefix(const std::string&);

    virtual void addAdminFacet(const Ice::CommunicatorPtr&);
    virtual void destroy();
    
    void run();
    
private:

    void log(const Ice::LogMessage&);
    
    const Ice::LoggerPtr _localLogger;
    const LoggerAdminIPtr _loggerAdmin;
     
    IceUtil::Monitor<IceUtil::Mutex> _monitor;

    bool _destroyed;
    bool _sendLogThreadStarted;
    IceUtil::ThreadControl _sendLogThreadControl;
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
// Filter out messages from in/out logMessages list
//
void 
filterLogMessages(Ice::LogMessageSeq& logMessages, const set<Ice::LogMessageType>& messageTypes,
                  const set<string>& traceCategories, Ice::Int messageMax)
{
    if(messageMax == 0)
    {
        logMessages.clear();
    }

    //
    // Filter only if one of the 3 filters is set; messageMax < 0 means "give me all"
    // that match the other filters, if any.
    //
    else if(!messageTypes.empty() || !traceCategories.empty() || messageMax > 0)
    {
        int count = 0;
        Ice::LogMessageSeq::reverse_iterator p = logMessages.rbegin();
        while(p != logMessages.rend())
        {
            bool keepIt = false;
            if(messageTypes.empty() || messageTypes.count(p->type) != 0)
            {
                if(p->type != Ice::TraceMessage || traceCategories.empty() || 
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
                p = Ice::LogMessageSeq::reverse_iterator(logMessages.erase(p.base()));
            }
        }
    }
    // else, don't need any filtering
}



//
// LoggerAdminI
//

LoggerAdminI::LoggerAdminI(const string& name, const Ice::PropertiesPtr& props) :
    _facetName(name),
    _logCount(0),
    _maxLogCount(props->getPropertyAsIntWithDefault("Ice.Admin.Logger.KeepLogs", 100)),
    _traceCount(0),
    _maxTraceCount(props->getPropertyAsIntWithDefault("Ice.Admin.Logger.KeepTraces", 100)),
    _traceLevel(props->getPropertyAsInt("Ice.Trace.Admin.Logger")),
    _remoteCallCompleted(Ice::newCallback(this, &LoggerAdminI::remoteCallCompleted)),
    _destroyed(false),
    _pingThreadStarted(false)
{
    int pingPeriod = props->getPropertyAsIntWithDefault("Ice.Admin.Logger.PingPeriod", 60);
    if(pingPeriod < 10)
    {
        pingPeriod = 10;
    }
    _pingPeriod = IceUtil::Time::seconds(pingPeriod);

    _oldestLog = _queue.end();
    _oldestTrace = _queue.end();
}

void
LoggerAdminI::attachRemoteLogger(const Ice::RemoteLoggerPrx& prx, 
                                 const Ice::LogMessageTypeSeq& messageTypes, 
                                 const Ice::StringSeq& categories,
                                 Ice::Int messageMax, 
                                 const Ice::Current& current)
{
    if(prx == 0)
    {
        return; // can't send this null RemoteLogger anything!
    }

    Ice::RemoteLoggerPrx remoteLogger = prx->ice_oneway();
    Filters filters(messageTypes, categories);

    Ice::LogMessageSeq initLogMessages;
    Ice::LoggerPtr logger = current.adapter->getCommunicator()->getLogger();

    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);
 
        if(_remoteLoggerMap.insert(make_pair(remoteLogger, filters)).second == false)
        {
            if(_traceLevel > 0)
            {
                Ice::Trace trace(current.adapter->getCommunicator()->getLogger(), _facetName);
                trace << "rejecting '" << remoteLogger << "' with RemoteLoggerAlreadyAttachedException";
            }

            throw Ice::RemoteLoggerAlreadyAttachedException();
        }

        if(_pingThreadStarted == false)
        {
            IceUtil::ThreadPtr t = new PingThread(this, logger);
            _pingThreadControl = t->start();
            _pingThreadStarted = true;
        }

        if(messageMax != 0)
        {
            initLogMessages = _queue; // copy
        }
    }
    
    if(_traceLevel > 0)
    {
        Ice::Trace trace(logger, _facetName);
        trace << "sending init to '" << remoteLogger->ice_twoway() << "'";
    }

    filterLogMessages(initLogMessages, filters.messageTypes, filters.traceCategories, messageMax);

    try
    {
        remoteLogger->ice_twoway()->begin_init(logger->getPrefix(), initLogMessages, _remoteCallCompleted, logger);
    }
    catch(const Ice::LocalException& ex)
    {
        deadRemoteLogger(remoteLogger, logger, ex, "init");
        throw;
    }
}
    
void
LoggerAdminI::detachRemoteLogger(const Ice::RemoteLoggerPrx& remoteLogger, 
                                 const Ice::Current& current)
{
    if(remoteLogger == 0)
    {
        throw Ice::RemoteLoggerNotAttachedException();
    }

    bool found = removeRemoteLogger(remoteLogger);

    if(!found)
    {
        throw Ice::RemoteLoggerNotAttachedException();
    }

    if(_traceLevel > 0)
    {
        Ice::Trace trace(current.adapter->getCommunicator()->getLogger(), _facetName);
        trace << "detached '" << remoteLogger << "'";
    }
}

Ice::LogMessageSeq 
LoggerAdminI::getLog(const Ice::LogMessageTypeSeq& messageTypes, 
                     const Ice::StringSeq& categories, 
                     Ice::Int messageMax, string& prefix, const Ice::Current& current)
{
    Ice::LogMessageSeq logMessages;
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);
        
        if(messageMax != 0)
        {
            logMessages = _queue;
        }
    }
   
    Ice::LoggerPtr logger = current.adapter->getCommunicator()->getLogger();
    prefix = logger->getPrefix();
    
    Filters filters(messageTypes, categories);
    filterLogMessages(logMessages, filters.messageTypes, filters.traceCategories, messageMax);
    return logMessages;
}

vector<Ice::RemoteLoggerPrx>
LoggerAdminI::log(const Ice::LogMessage& logMessage)
{
    vector<Ice::RemoteLoggerPrx> remoteLoggers;

    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);

    //
    // Put message in _queue
    //
    if((logMessage.type != Ice::TraceMessage && _maxLogCount > 0) || 
       (logMessage.type == Ice::TraceMessage && _maxTraceCount > 0)) 
    {
        list<Ice::LogMessage>::iterator p = _queue.insert(_queue.end(), logMessage);
        
        if(logMessage.type != Ice::TraceMessage)
        {
            assert(_maxLogCount > 0);
            if(_logCount == _maxLogCount)
            {
                //
                // Need to remove the oldest log from the queue
                //
                assert(_oldestLog != _queue.end());
                _oldestLog = _queue.erase(_oldestLog);
                while(_oldestLog != _queue.end() && _oldestLog->type == Ice::TraceMessage)
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
                while(_oldestTrace != _queue.end() && _oldestTrace->type != Ice::TraceMessage)
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
                if(logMessage.type != Ice::TraceMessage || filters.traceCategories.empty() ||
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
LoggerAdminI::deadRemoteLogger(const Ice::RemoteLoggerPrx& remoteLogger,
                               const Ice::LoggerPtr& logger,
                               const Ice::LocalException& ex,
                               const string& operation)
{
    if(removeRemoteLogger(remoteLogger))
    {
        if(_traceLevel > 0)
        {
            Ice::Trace trace(logger, _facetName);
            trace << operation << " on '" << remoteLogger << "' raised '" 
                  << ex << "'; remote logger removed automatically";
        }
    }
}

void
LoggerAdminI::destroy()
{
    IceUtil::ThreadControl pingThreadControl;
    bool joinThread = false;
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);
        
        if(_pingThreadStarted)
        {
            joinThread = true;
            pingThreadControl = _pingThreadControl;
            _pingThreadStarted = false;
            _destroyed = true;
            _monitor.notifyAll();
        }
    }

    if(joinThread)
    {
        pingThreadControl.join();
    }
}

void
LoggerAdminI::run(const Ice::LoggerPtr& logger)
{
    if(_traceLevel > 1)
    {
        Ice::Trace trace(logger, _facetName);
        trace << "Ping thread started";
    }

    for(;;)
    {
        vector<Ice::RemoteLoggerPrx> remoteLoggers;
        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);
            if(_destroyed)
            {
                break; // for(;;)
            }

            if(_monitor.timedWait(_pingPeriod) == false && !_destroyed)
            {
                transform(_remoteLoggerMap.begin(), _remoteLoggerMap.end(), back_inserter(remoteLoggers), 
                          GetRemoteLoggerMapKey());
            }
        }
        
        for(vector<Ice::RemoteLoggerPrx>::const_iterator p = remoteLoggers.begin(); p != remoteLoggers.end(); ++p)
        {
            Ice::RemoteLoggerPrx remoteLogger = (*p)->ice_twoway();

            if(_traceLevel > 1)
            {
                Ice::Trace trace(logger, _facetName);
                trace << "sending ice_ping to '" << remoteLogger << "'";
            }
            
            try
            {
                remoteLogger->begin_ice_ping(_remoteCallCompleted, logger);
            }
            catch(const Ice::LocalException& ex)
            {
                deadRemoteLogger(remoteLogger, logger, ex, "ice_ping");
            }
        }
    }        
    
    if(_traceLevel > 1)
    {
        Ice::Trace trace(logger, _facetName);
        trace << "Ping thread completed";
    }
}

bool
LoggerAdminI::removeRemoteLogger(const Ice::RemoteLoggerPrx& remoteLogger)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);
    return _remoteLoggerMap.erase(remoteLogger) > 0;
}

void
LoggerAdminI::remoteCallCompleted(const Ice::AsyncResultPtr& r)
{
    try 
    {
        r->throwLocalException();

        if(_traceLevel > 1)
        {
            Ice::LoggerPtr logger = Ice::LoggerPtr::dynamicCast(r->getCookie());
            Ice::Trace trace(logger, _facetName);
            trace << r->getOperation() << " on '" << r->getProxy() << "' completed successfully";
        }
    }
    catch(const Ice::LocalException& ex)
    {
        deadRemoteLogger(Ice::RemoteLoggerPrx::uncheckedCast(r->getProxy()),
                        Ice::LoggerPtr::dynamicCast(r->getCookie()),
                        ex, r->getOperation());
    }
}

//
// PingThread
//
PingThread::PingThread(
    const LoggerAdminIPtr& loggerAdmin, const Ice::LoggerPtr& logger) :
    IceUtil::Thread("Ice.RemoteLoggerPingThread"),
    _loggerAdmin(loggerAdmin),
    _logger(logger)
{
}

void
PingThread::run()
{
    _loggerAdmin->run(_logger);
}


//
// LoggerAdminLoggerI
//

LoggerAdminLoggerI::LoggerAdminLoggerI(const string& facetName, 
                                       const Ice::PropertiesPtr& props, 
                                       const Ice::LoggerPtr& localLogger) :
    _localLogger(localLogger),
    _loggerAdmin(new LoggerAdminI(facetName, props)),
    _destroyed(false),
    _sendLogThreadStarted(false)
{
    //
    // There is currently no way to have a null local logger
    //
    assert(_localLogger != 0);
}

void
LoggerAdminLoggerI::print(const string& message)
{
   Ice::LogMessage logMessage = {Ice::PrintMessage, 
                                 IceUtil::Time::now().toMicroSeconds(),
                                 "",
                                 message};
   
    _localLogger->print(message);
    log(logMessage);
}

void
LoggerAdminLoggerI::trace(const string& category, const string& message)
{
    Ice::LogMessage logMessage = {Ice::TraceMessage, 
                                  IceUtil::Time::now().toMicroSeconds(),
                                  category,
                                  message};
    
    _localLogger->trace(category, message);
    log(logMessage);
}

void
LoggerAdminLoggerI::warning(const string& message)
{
    Ice::LogMessage logMessage = {Ice::WarningMessage, 
                                  IceUtil::Time::now().toMicroSeconds(),
                                  "",
                                  message};

    
    _localLogger->warning(message);
    log(logMessage);
}

void
LoggerAdminLoggerI::error(const string& message)
{
    Ice::LogMessage logMessage = {Ice::ErrorMessage, 
                                   IceUtil::Time::now().toMicroSeconds(),
                                  "",
                                  message};
    
    
    _localLogger->error(message);
    log(logMessage);
}

string
LoggerAdminLoggerI::getPrefix()
{
    return _localLogger->getPrefix();
}

Ice::LoggerPtr
LoggerAdminLoggerI::cloneWithPrefix(const string& prefix)
{
    return _localLogger->cloneWithPrefix(prefix);
}

void
LoggerAdminLoggerI::addAdminFacet(const Ice::CommunicatorPtr& communicator)
{
    communicator->addAdminFacet(_loggerAdmin, _loggerAdmin->getFacetName());
}

void 
LoggerAdminLoggerI::log(const Ice::LogMessage& logMessage)
{

    //
    // We don't want to proceed if the SendLog thread is calling log.
    //
    bool proceed = false;
    {
        IceUtil::ThreadControl currentThread;

        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);
        proceed = !_destroyed &&
            (!_sendLogThreadStarted || (_sendLogThreadStarted && (currentThread != _sendLogThreadControl)));
    }

    if(proceed)
    {
        const vector<Ice::RemoteLoggerPrx> remoteLoggers = _loggerAdmin->log(logMessage); 
    
        if(!remoteLoggers.empty())
        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);
            
            if(!_sendLogThreadStarted)
            {
                IceUtil::ThreadPtr t = new SendLogThread(this);
                _sendLogThreadControl = t->start();
                _sendLogThreadStarted = true;
            }
            _jobQueue.push_back(new Job(remoteLoggers, logMessage));
            _monitor.notifyAll();
        }
    }
}

void
LoggerAdminLoggerI::destroy()
{
    _loggerAdmin->destroy();

    IceUtil::ThreadControl sendLogThreadControl;
    bool joinThread = false;
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);
        
        if(_sendLogThreadStarted)
        {
            joinThread = true;
            sendLogThreadControl = _sendLogThreadControl;
            _sendLogThreadStarted = false;
            _destroyed = true;
            _monitor.notifyAll();
        }
    }

    if(joinThread)
    {
        sendLogThreadControl.join();
    }
}

void
LoggerAdminLoggerI::run()
{
    if(_loggerAdmin->getTraceLevel() > 1)
    {
        Ice::Trace trace(_localLogger, _loggerAdmin->getFacetName());
        trace << "Send log thread started";
    }

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
        if(!_jobQueue.empty())
        {
            JobPtr job = _jobQueue.front();
            _jobQueue.pop_front();
            lock.release();
            
            for(vector<Ice::RemoteLoggerPrx>::const_iterator p = job->remoteLoggers.begin();
                p != job->remoteLoggers.end(); ++p)
            {
                if(_loggerAdmin->getTraceLevel() > 1)
                {
                    Ice::Trace trace(_localLogger, _loggerAdmin->getFacetName());
                    trace << "sending log message to '" << *p << "'";
                }

                try
                {
                    // We use a synchronous oneway call here
                    // using twoway could generate additional log when receiving the response
                    // from the client thread-pool thread; likewise, AMI could generate
                    // extra logs from the client thread pool thread
                    
                    (*p)->log(job->logMessage);
                }
                catch(const Ice::LocalException& ex)
                {
                    _loggerAdmin->deadRemoteLogger(*p, _localLogger, ex, "log");
                }
            }  
        }
    }

    if(_loggerAdmin->getTraceLevel() > 1)
    {
        Ice::Trace trace(_localLogger, _loggerAdmin->getFacetName());
        trace << "Send log thread completed";
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

Ice::LoggerAdminLoggerPtr 
createLoggerAdminLogger(const std::string& facetName, 
                        const Ice::PropertiesPtr& props, 
                        const Ice::LoggerPtr& localLogger)
{
    return new LoggerAdminLoggerI(facetName, props, localLogger);
}

}
