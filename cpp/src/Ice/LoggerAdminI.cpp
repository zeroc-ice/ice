// Copyright (c) ZeroC, Inc.

#include "LoggerAdminI.h"
#include "Ice/Communicator.h"
#include "Ice/Initialize.h"
#include "Ice/LocalExceptions.h"
#include "Ice/LoggerUtil.h"
#include "Ice/ObjectAdapter.h"
#include "Ice/Properties.h"
#include "Ice/ProxyFunctions.h"
#include "Ice/RemoteLogger.h"

#include <atomic>
#include <deque>
#include <set>

using namespace Ice;
using namespace std;

namespace
{
    const char* traceCategory = "Admin.Logger";

    class LoggerAdminI : public Ice::LoggerAdmin, public std::enable_shared_from_this<LoggerAdminI>
    {
    public:
        LoggerAdminI(const PropertiesPtr&);

        void
        attachRemoteLogger(optional<RemoteLoggerPrx>, LogMessageTypeSeq, StringSeq, int32_t, const Current&) override;

        bool detachRemoteLogger(optional<RemoteLoggerPrx>, const Current&) override;

        LogMessageSeq getLog(LogMessageTypeSeq, StringSeq, int32_t, string&, const Current&) override;

        void destroy();

        vector<RemoteLoggerPrx> log(const LogMessage&);

        void deadRemoteLogger(const RemoteLoggerPrx&, const LoggerPtr&, exception_ptr, std::string_view);

        [[nodiscard]] int getTraceLevel() const { return _traceLevel; }

    private:
        bool removeRemoteLogger(const RemoteLoggerPrx&);

        std::mutex _mutex;
        list<LogMessage> _queue;
        int _logCount{0}; // non-trace messages
        const int _maxLogCount;
        int _traceCount{0};
        const int _maxTraceCount;
        const int _traceLevel;

        list<LogMessage>::iterator _oldestTrace;
        list<LogMessage>::iterator _oldestLog;

        struct ObjectIdentityCompare
        {
            bool operator()(const RemoteLoggerPrx& lhs, const RemoteLoggerPrx& rhs) const
            {
                return lhs->ice_getIdentity() < rhs->ice_getIdentity();
            }
        };

        struct Filters
        {
            Filters(const LogMessageTypeSeq& m, const StringSeq& c)
                : messageTypes(m.begin(), m.end()),
                  traceCategories(c.begin(), c.end())
            {
            }

            const set<LogMessageType> messageTypes;
            const set<string> traceCategories;
        };

        using RemoteLoggerMap = map<RemoteLoggerPrx, Filters, ObjectIdentityCompare>;

        struct GetRemoteLoggerMapKey
        {
            RemoteLoggerMap::key_type operator()(const RemoteLoggerMap::value_type& val) { return val.first; }
        };

        RemoteLoggerMap _remoteLoggerMap;
        CommunicatorPtr _sendLogCommunicator;
        bool _destroyed{false};
    };
    using LoggerAdminIPtr = std::shared_ptr<LoggerAdminI>;

    class Job
    {
    public:
        Job(const vector<RemoteLoggerPrx>& r, LogMessage l) : remoteLoggers(r), logMessage(std::move(l)) {}

        const vector<RemoteLoggerPrx> remoteLoggers;
        const LogMessage logMessage;
    };
    using JobPtr = std::shared_ptr<Job>;

    class LoggerAdminLoggerI final : public IceInternal::LoggerAdminLogger,
                                     public std::enable_shared_from_this<LoggerAdminLoggerI>
    {
    public:
        LoggerAdminLoggerI(const PropertiesPtr&, const LoggerPtr&);

        void print(const std::string&) final;
        void trace(const std::string&, const std::string&) final;
        void warning(const std::string&) final;
        void error(const std::string&) final;
        std::string getPrefix() final;
        LoggerPtr cloneWithPrefix(std::string) final;

        [[nodiscard]] ObjectPtr getFacet() const override;
        void detach() final;

        [[nodiscard]] const LoggerPtr& getLocalLogger() const { return _localLogger; }

        void run();

    private:
        void log(const LogMessage&);

        const LoggerPtr _localLogger;
        const LoggerAdminIPtr _loggerAdmin;

        std::mutex _mutex;
        std::condition_variable _conditionVariable;

        std::atomic<bool> _detached{false};
        std::thread _sendLogThread;
        std::deque<JobPtr> _jobQueue;
    };
    using LoggerAdminLoggerIPtr = std::shared_ptr<LoggerAdminLoggerI>;

    //
    // Helper functions
    //

    LoggerPtr unwrapLocalLogger(const LoggerPtr& localLogger)
    {
        // There is currently no way to have a null local logger
        assert(localLogger);

        auto wrapper = dynamic_pointer_cast<LoggerAdminLoggerI>(localLogger);
        if (wrapper)
        {
            // use the underlying local logger
            return wrapper->getLocalLogger();
        }
        else
        {
            return localLogger;
        }
    }

    //
    // Filter out messages from in/out logMessages list
    //
    void filterLogMessages(
        LogMessageSeq& logMessages,
        const set<LogMessageType>& messageTypes,
        const set<string>& traceCategories,
        int32_t messageMax)
    {
        assert(!logMessages.empty() && messageMax != 0);

        //
        // Filter only if one of the 3 filters is set; messageMax < 0 means "give me all"
        // that match the other filters, if any.
        //
        if (!messageTypes.empty() || !traceCategories.empty() || messageMax > 0)
        {
            int count = 0;
            auto p = logMessages.rbegin();
            while (p != logMessages.rend())
            {
                bool keepIt = false;
                if (messageTypes.empty() || messageTypes.count(p->type) != 0)
                {
                    if (p->type != LogMessageType::TraceMessage || traceCategories.empty() ||
                        traceCategories.count(p->traceCategory) != 0)
                    {
                        keepIt = true;
                    }
                }

                if (keepIt)
                {
                    ++p;
                    ++count;
                    if (messageMax > 0 && count >= messageMax)
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
    RemoteLoggerPrx changeCommunicator(const RemoteLoggerPrx& prx, const CommunicatorPtr& communicator)
    {
        RemoteLoggerPrx result{communicator, prx->ice_toString()};
        return result->ice_invocationTimeout(prx->ice_getInvocationTimeout());
    }

    //
    // Copies a set of properties
    //
    void copyProperties(const string& prefix, const PropertiesPtr& from, const PropertiesPtr& to)
    {
        PropertyDict dict = from->getPropertiesForPrefix(prefix);
        for (const auto& p : dict)
        {
            to->setProperty(p.first, p.second);
        }
    }

    //
    // Create communicator used to send logs
    //
    CommunicatorPtr createSendLogCommunicator(const CommunicatorPtr& communicator, const LoggerPtr& logger)
    {
        InitializationData initData;
        initData.logger = logger;
        initData.properties = createProperties();

        PropertiesPtr mainProps = communicator->getProperties();

        copyProperties("Ice.Default.Locator", mainProps, initData.properties);
        copyProperties("IceSSL.", mainProps, initData.properties);

        StringSeq extraProps = mainProps->getIcePropertyAsList("Ice.Admin.Logger.Properties");

        if (!extraProps.empty())
        {
            for (auto& extraProp : extraProps)
            {
                if (extraProp.find("--") != 0)
                {
                    extraProp = "--" + extraProp;
                }
            }
            initData.properties->parseCommandLineOptions("", extraProps);
        }

        return initialize(std::move(initData));
    }

    //
    // LoggerAdminI
    //

    LoggerAdminI::LoggerAdminI(const PropertiesPtr& props)
        : _maxLogCount(props->getIcePropertyAsInt("Ice.Admin.Logger.KeepLogs")),
          _maxTraceCount(props->getIcePropertyAsInt("Ice.Admin.Logger.KeepTraces")),
          _traceLevel(props->getIcePropertyAsInt("Ice.Trace.Admin.Logger"))

    {
        _oldestLog = _queue.end();
        _oldestTrace = _queue.end();
    }

    void LoggerAdminI::attachRemoteLogger(
        optional<RemoteLoggerPrx> prx,
        LogMessageTypeSeq messageTypes,
        StringSeq categories,
        int32_t messageMax,
        const Current& current)
    {
        if (!prx)
        {
            return; // can't send this null RemoteLogger anything!
        }

        //
        // In C++, LoggerAdminI does not keep a "logger" data member to avoid a hard-to-break circular
        // reference, so we retrieve the logger from Current
        //
        LoggerAdminLoggerIPtr logger =
            dynamic_pointer_cast<LoggerAdminLoggerI>(current.adapter->getCommunicator()->getLogger());
        assert(logger);

        RemoteLoggerPrx remoteLogger = prx->ice_twoway();

        Filters filters(messageTypes, categories);
        LogMessageSeq initLogMessages;
        {
            lock_guard lock(_mutex);

            if (!_sendLogCommunicator)
            {
                if (_destroyed)
                {
                    throw Ice::ObjectNotExistException{__FILE__, __LINE__};
                }

                _sendLogCommunicator =
                    createSendLogCommunicator(current.adapter->getCommunicator(), logger->getLocalLogger());
            }

            if (!_remoteLoggerMap.insert(make_pair(changeCommunicator(remoteLogger, _sendLogCommunicator), filters))
                     .second)
            {
                if (_traceLevel > 0)
                {
                    Trace trace(logger, traceCategory);
                    trace << "rejecting '" << remoteLogger << "' with RemoteLoggerAlreadyAttachedException";
                }

                throw RemoteLoggerAlreadyAttachedException();
            }

            if (messageMax != 0)
            {
                initLogMessages = _queue; // copy
            }
        }

        if (_traceLevel > 0)
        {
            Trace trace(logger, traceCategory);
            trace << "attached '" << remoteLogger << "'";
        }

        if (!initLogMessages.empty())
        {
            filterLogMessages(initLogMessages, filters.messageTypes, filters.traceCategories, messageMax);
        }

        try
        {
            auto self = shared_from_this();
            remoteLogger->initAsync(
                logger->getPrefix(),
                initLogMessages,
                [self, logger, remoteLogger]()
                {
                    if (self->_traceLevel > 1)
                    {
                        Trace trace(logger, traceCategory);
                        trace << "init on '" << remoteLogger << "' completed successfully";
                    }
                },
                [self, logger, remoteLogger](exception_ptr e)
                { self->deadRemoteLogger(remoteLogger, logger, e, "init"); });
        }
        catch (const LocalException&)
        {
            deadRemoteLogger(remoteLogger, logger, current_exception(), "init");
            throw;
        }
    }

    bool LoggerAdminI::detachRemoteLogger(std::optional<RemoteLoggerPrx> remoteLogger, const Current& current)
    {
        if (!remoteLogger)
        {
            return false;
        }

        //
        // No need to convert the proxy as we only use its identity
        //
        bool found = removeRemoteLogger(remoteLogger.value());

        if (_traceLevel > 0)
        {
            Trace trace(current.adapter->getCommunicator()->getLogger(), traceCategory);
            if (found)
            {
                trace << "detached '" << remoteLogger.value() << "'";
            }
            else
            {
                trace << "cannot detach '" << remoteLogger.value() << "': not found";
            }
        }

        return found;
    }

    LogMessageSeq LoggerAdminI::getLog(
        LogMessageTypeSeq messageTypes,
        StringSeq categories,
        int32_t messageMax,
        string& prefix,
        const Current& current)
    {
        LogMessageSeq logMessages;
        {
            lock_guard lock(_mutex);

            if (messageMax != 0)
            {
                logMessages = _queue;
            }
        }

        LoggerPtr logger = current.adapter->getCommunicator()->getLogger();
        prefix = logger->getPrefix();

        if (!logMessages.empty())
        {
            Filters filters(messageTypes, categories);
            filterLogMessages(logMessages, filters.messageTypes, filters.traceCategories, messageMax);
        }

        return logMessages;
    }

    void LoggerAdminI::destroy()
    {
        CommunicatorPtr sendLogCommunicator;
        {
            lock_guard lock(_mutex);
            if (!_destroyed)
            {
                _destroyed = true;
                sendLogCommunicator = _sendLogCommunicator;
                _sendLogCommunicator = nullptr;
            }
        }

        //
        // Destroy outside lock to avoid deadlock when there are outstanding two-way log calls sent to
        // remote loggers
        //
        if (sendLogCommunicator)
        {
            sendLogCommunicator->destroy();
        }
    }

    vector<RemoteLoggerPrx> LoggerAdminI::log(const LogMessage& logMessage)
    {
        vector<RemoteLoggerPrx> remoteLoggers;

        lock_guard lock(_mutex);

        //
        // Put message in _queue
        //
        if ((logMessage.type != LogMessageType::TraceMessage && _maxLogCount > 0) ||
            (logMessage.type == LogMessageType::TraceMessage && _maxTraceCount > 0))
        {
            auto p = _queue.insert(_queue.end(), logMessage);

            if (logMessage.type != LogMessageType::TraceMessage)
            {
                assert(_maxLogCount > 0);
                if (_logCount == _maxLogCount)
                {
                    //
                    // Need to remove the oldest log from the queue
                    //
                    assert(_oldestLog != _queue.end());
                    _oldestLog = _queue.erase(_oldestLog);
                    while (_oldestLog != _queue.end() && _oldestLog->type == LogMessageType::TraceMessage)
                    {
                        _oldestLog++;
                    }
                    assert(_oldestLog != _queue.end());
                }
                else
                {
                    assert(_logCount < _maxLogCount);
                    _logCount++;
                    if (_oldestLog == _queue.end())
                    {
                        _oldestLog = p;
                    }
                }
            }
            else
            {
                assert(_maxTraceCount > 0);
                if (_traceCount == _maxTraceCount)
                {
                    //
                    // Need to remove the oldest trace from the queue
                    //
                    assert(_oldestTrace != _queue.end());
                    _oldestTrace = _queue.erase(_oldestTrace);
                    while (_oldestTrace != _queue.end() && _oldestTrace->type != LogMessageType::TraceMessage)
                    {
                        _oldestTrace++;
                    }
                    assert(_oldestTrace != _queue.end());
                }
                else
                {
                    assert(_traceCount < _maxTraceCount);
                    _traceCount++;
                    if (_oldestTrace == _queue.end())
                    {
                        _oldestTrace = p;
                    }
                }
            }

            //
            // Queue updated, now find which remote loggers want this message
            //
            for (const auto& q : _remoteLoggerMap)
            {
                const Filters& filters = q.second;

                if (filters.messageTypes.empty() || filters.messageTypes.count(logMessage.type) != 0)
                {
                    if (logMessage.type != LogMessageType::TraceMessage || filters.traceCategories.empty() ||
                        filters.traceCategories.count(logMessage.traceCategory) != 0)
                    {
                        remoteLoggers.push_back(q.first);
                    }
                }
            }
        }
        return remoteLoggers;
    }

    void LoggerAdminI::deadRemoteLogger(
        const RemoteLoggerPrx& remoteLogger,
        const LoggerPtr& logger,
        std::exception_ptr ex,
        string_view operation)
    {
        //
        // No need to convert remoteLogger as we only use its identity
        //
        if (removeRemoteLogger(remoteLogger))
        {
            if (_traceLevel > 0)
            {
                try
                {
                    rethrow_exception(ex);
                }
                catch (const std::exception& e)
                {
                    Trace trace(logger, traceCategory);
                    trace << "detached '" << remoteLogger << "' because " << operation << " raised:\n" << e;
                }
            }
        }
    }

    bool LoggerAdminI::removeRemoteLogger(const RemoteLoggerPrx& remoteLogger)
    {
        lock_guard lock(_mutex);
        return _remoteLoggerMap.erase(remoteLogger) > 0;
    }

    //
    // LoggerAdminLoggerI
    //

    LoggerAdminLoggerI::LoggerAdminLoggerI(const PropertiesPtr& props, const LoggerPtr& localLogger)
        : _localLogger(unwrapLocalLogger(localLogger)),
          _loggerAdmin(new LoggerAdminI(props))
    {
    }

    void LoggerAdminLoggerI::print(const string& message)
    {
        _localLogger->print(message);

        if (!_detached.load())
        {
            LogMessage logMessage = {
                LogMessageType::PrintMessage,
                chrono::duration_cast<chrono::microseconds>(chrono::system_clock::now().time_since_epoch()).count(),
                "",
                message};
            log(logMessage);
        }
    }

    void LoggerAdminLoggerI::trace(const string& category, const string& message)
    {
        _localLogger->trace(category, message);

        if (!_detached.load())
        {
            LogMessage logMessage = {
                LogMessageType::TraceMessage,
                chrono::duration_cast<chrono::microseconds>(chrono::system_clock::now().time_since_epoch()).count(),
                category,
                message};
            log(logMessage);
        }
    }

    void LoggerAdminLoggerI::warning(const string& message)
    {
        _localLogger->warning(message);

        if (!_detached.load())
        {
            LogMessage logMessage = {
                LogMessageType::WarningMessage,
                chrono::duration_cast<chrono::microseconds>(chrono::system_clock::now().time_since_epoch()).count(),
                "",
                message};
            log(logMessage);
        }
    }

    void LoggerAdminLoggerI::error(const string& message)
    {
        _localLogger->error(message);

        if (!_detached.load())
        {
            LogMessage logMessage = {
                LogMessageType::ErrorMessage,
                chrono::duration_cast<chrono::microseconds>(chrono::system_clock::now().time_since_epoch()).count(),
                "",
                message};
            log(logMessage);
        }
    }

    string LoggerAdminLoggerI::getPrefix() { return _localLogger->getPrefix(); }

    LoggerPtr LoggerAdminLoggerI::cloneWithPrefix(string prefix)
    {
        return _localLogger->cloneWithPrefix(std::move(prefix));
    }

    ObjectPtr LoggerAdminLoggerI::getFacet() const { return _loggerAdmin; }

    void LoggerAdminLoggerI::log(const LogMessage& logMessage)
    {
        const vector<RemoteLoggerPrx> remoteLoggers = _loggerAdmin->log(logMessage);

        if (!remoteLoggers.empty())
        {
            lock_guard lock(_mutex);

            if (!_sendLogThread.joinable())
            {
                _sendLogThread = std::thread(&LoggerAdminLoggerI::run, this);
            }

            _jobQueue.push_back(make_shared<Job>(remoteLoggers, logMessage));
            _conditionVariable.notify_all();
        }
    }

    void LoggerAdminLoggerI::detach()
    {
        std::thread sendLogThread;
        {
            lock_guard lock(_mutex);

            if (_sendLogThread.joinable())
            {
                sendLogThread = std::move(_sendLogThread);
                _detached.store(true);
                _conditionVariable.notify_all();
            }
        }

        if (sendLogThread.joinable())
        {
            sendLogThread.join();
        }

        // destroy sendLogCommunicator
        _loggerAdmin->destroy();
    }

    void LoggerAdminLoggerI::run()
    {
        if (_loggerAdmin->getTraceLevel() > 1)
        {
            Trace trace(_localLogger, traceCategory);
            trace << "send log thread started";
        }

        for (;;)
        {
            unique_lock lock(_mutex);
            _conditionVariable.wait(lock, [this] { return _detached.load() || !_jobQueue.empty(); });

            if (_detached.load())
            {
                break;
            }

            assert(!_jobQueue.empty());
            JobPtr job = _jobQueue.front();
            _jobQueue.pop_front();
            lock.unlock();

            for (const auto& remoteLogger : job->remoteLoggers)
            {
                if (_loggerAdmin->getTraceLevel() > 1)
                {
                    Trace trace(_localLogger, traceCategory);
                    trace << "sending log message to '" << remoteLogger << "'";
                }

                try
                {
                    auto self = shared_from_this();
                    remoteLogger->logAsync(
                        job->logMessage,
                        [self, proxy = remoteLogger]()
                        {
                            if (self->_loggerAdmin->getTraceLevel() > 1)
                            {
                                Trace trace(self->_localLogger, traceCategory);
                                trace << "log on '" << proxy << "' completed successfully";
                            }
                        },
                        [self, proxy = remoteLogger](exception_ptr e)
                        {
                            try
                            {
                                rethrow_exception(e);
                            }
                            catch (const CommunicatorDestroyedException&)
                            {
                                // expected if there are outstanding calls during communicator destruction
                            }
                            catch (const LocalException&)
                            {
                                self->_loggerAdmin->deadRemoteLogger(proxy, self->_localLogger, e, "log");
                            }
                        });
                }
                catch (const LocalException&)
                {
                    _loggerAdmin->deadRemoteLogger(remoteLogger, _localLogger, current_exception(), "log");
                }
            }
        }

        if (_loggerAdmin->getTraceLevel() > 1)
        {
            Trace trace(_localLogger, traceCategory);
            trace << "send log thread completed";
        }
    }
}

namespace IceInternal
{
    LoggerAdminLogger::~LoggerAdminLogger() = default; // avoid weak vtable

    LoggerAdminLoggerPtr createLoggerAdminLogger(const PropertiesPtr& props, const LoggerPtr& localLogger)
    {
        return make_shared<LoggerAdminLoggerI>(props, localLogger);
    }
}
