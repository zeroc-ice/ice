// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.ListIterator;
import java.util.Map;
import java.util.Set;

final class LoggerAdminI implements LoggerAdmin {
    @Override
    public void attachRemoteLogger(
            RemoteLoggerPrx prx,
            LogMessageType[] messageTypes,
            String[] categories,
            int messageMax,
            Current current)
        throws RemoteLoggerAlreadyAttachedException {
        if (prx == null) {
            return; // can't send this null RemoteLogger anything!
        }

        RemoteLoggerPrx remoteLogger = RemoteLoggerPrx.uncheckedCast(prx.ice_twoway());

        Filters filters = new Filters(messageTypes, categories);
        List<LogMessage> initLogMessages = null;

        synchronized (this) {
            if (_sendLogCommunicator == null) {
                if (_destroyed) {
                    throw new ObjectNotExistException();
                }

                _sendLogCommunicator =
                    createSendLogCommunicator(current.adapter.getCommunicator(), _logger.getLocalLogger());
            }

            Identity remoteLoggerId = remoteLogger.ice_getIdentity();

            if (_remoteLoggerMap.containsKey(remoteLoggerId)) {
                if (_traceLevel > 0) {
                    String m = "rejecting `" + remoteLogger.toString() + "' with RemoteLoggerAlreadyAttachedException";
                    _logger.trace(_traceCategory, m);
                }

                throw new RemoteLoggerAlreadyAttachedException();
            }

            _remoteLoggerMap.put(
                remoteLoggerId,
                new RemoteLoggerData(changeCommunicator(remoteLogger, _sendLogCommunicator), filters));

            if (messageMax != 0) {
                initLogMessages = new LinkedList<>(_queue); // copy
            } else {
                initLogMessages = new LinkedList<>();
            }
        }

        if (_traceLevel > 0) {
            _logger.trace(_traceCategory, "attached `" + remoteLogger.toString() + "'");
        }

        if (!initLogMessages.isEmpty()) {
            filterLogMessages(initLogMessages, filters.messageTypes, filters.traceCategories, messageMax);
        }

        try {
            remoteLogger
                .initAsync(_logger.getPrefix(), initLogMessages.toArray(new LogMessage[0]))
                .whenComplete(
                    (Void v, Throwable ex) -> {
                        if (ex != null) {
                            if (ex instanceof LocalException) {
                                deadRemoteLogger(remoteLogger, _logger, (LocalException) ex, "init");
                            } else {
                                deadRemoteLogger(remoteLogger, _logger, new UnknownException(ex), "init");
                            }
                        } else {
                            if (_traceLevel > 1) {
                                String msg = "init on `" + remoteLogger.toString() + "' completed successfully";
                                _logger.trace(_traceCategory, msg);
                            }
                        }
                    });
        } catch (LocalException ex) {
            deadRemoteLogger(remoteLogger, _logger, ex, "init");
            throw ex;
        }
    }

    @Override
    public boolean detachRemoteLogger(RemoteLoggerPrx remoteLogger, Current current) {
        if (remoteLogger == null) {
            return false;
        }

        // No need to convert the proxy as we only use its identity
        boolean found = removeRemoteLogger(remoteLogger);

        if (_traceLevel > 0) {
            if (found) {
                _logger.trace(_traceCategory, "detached `" + remoteLogger.toString() + "'");
            } else {
                _logger.trace(
                    _traceCategory,
                    "cannot detach `" + remoteLogger.toString() + "': not found");
            }
        }

        return found;
    }

    @Override
    public LoggerAdmin.GetLogResult getLog(
            LogMessageType[] messageTypes, String[] categories, int messageMax, Current current) {
        LoggerAdmin.GetLogResult r = new LoggerAdmin.GetLogResult();

        List<LogMessage> logMessages = null;
        synchronized (this) {
            if (messageMax != 0) {
                logMessages = new LinkedList<>(_queue);
            } else {
                logMessages = new LinkedList<>();
            }
        }

        r.prefix = _logger.getPrefix();

        if (!logMessages.isEmpty()) {
            Filters filters = new Filters(messageTypes, categories);
            filterLogMessages(logMessages, filters.messageTypes, filters.traceCategories, messageMax);
        }
        r.returnValue = logMessages.toArray(new LogMessage[0]);
        return r;
    }

    LoggerAdminI(Properties props, LoggerAdminLoggerI logger) {
        _maxLogCount = props.getIcePropertyAsInt("Ice.Admin.Logger.KeepLogs");
        _maxTraceCount = props.getIcePropertyAsInt("Ice.Admin.Logger.KeepTraces");
        _traceLevel = props.getIcePropertyAsInt("Ice.Trace.Admin.Logger");
        _logger = logger;
    }

    void destroy() {
        Communicator sendLogCommunicator = null;

        synchronized (this) {
            if (!_destroyed) {
                _destroyed = true;
                sendLogCommunicator = _sendLogCommunicator;
                _sendLogCommunicator = null;
            }
        }

        // Destroy outside lock to avoid deadlock when there are outstanding two-way log calls sent to remote loggers
        if (sendLogCommunicator != null) {
            sendLogCommunicator.destroy();
        }
    }

    synchronized List<RemoteLoggerPrx> log(LogMessage logMessage) {
        List<RemoteLoggerPrx> remoteLoggers = null;

        // Put message in _queue
        if ((logMessage.type != LogMessageType.TraceMessage && _maxLogCount > 0)
            || (logMessage.type == LogMessageType.TraceMessage && _maxTraceCount > 0)) {
            _queue.add(logMessage); // add at the end

            if (logMessage.type != LogMessageType.TraceMessage) {
                assert (_maxLogCount > 0);
                if (_logCount == _maxLogCount) {
                    // Need to remove the oldest log from the queue
                    assert (_oldestLog != -1);
                    _queue.remove(_oldestLog);
                    int qs = _queue.size();

                    while (_oldestLog < qs && _queue.get(_oldestLog).type == LogMessageType.TraceMessage) {
                        _oldestLog++;
                    }
                    assert (_oldestLog < qs); // remember: we just added a log message at end
                } else {
                    assert (_logCount < _maxLogCount);
                    _logCount++;
                    if (_oldestLog == -1) {
                        _oldestLog = _queue.size() - 1;
                    }
                }
            } else {
                assert (_maxTraceCount > 0);
                if (_traceCount == _maxTraceCount) {
                    // Need to remove the oldest trace from the queue
                    assert (_oldestTrace != -1);
                    _queue.remove(_oldestTrace);
                    int qs = _queue.size();
                    while (_oldestTrace < qs && _queue.get(_oldestTrace).type != LogMessageType.TraceMessage) {
                        _oldestTrace++;
                    }
                    assert (_oldestTrace < qs); // remember: we just added a trace message at end
                } else {
                    assert (_traceCount < _maxTraceCount);
                    _traceCount++;
                    if (_oldestTrace == -1) {
                        _oldestTrace = _queue.size() - 1;
                    }
                }
            }

            // Queue updated, now find which remote loggers want this message
            for (RemoteLoggerData p : _remoteLoggerMap.values()) {
                Filters filters = p.filters;

                if (filters.messageTypes.isEmpty() || filters.messageTypes.contains(logMessage.type)) {
                    if (logMessage.type != LogMessageType.TraceMessage
                        || filters.traceCategories.isEmpty()
                        || filters.traceCategories.contains(logMessage.traceCategory)) {
                        if (remoteLoggers == null) {
                            remoteLoggers = new ArrayList<>();
                        }
                        remoteLoggers.add(p.remoteLogger);
                    }
                }
            }
        }

        return remoteLoggers;
    }

    void deadRemoteLogger(RemoteLoggerPrx remoteLogger, Logger logger, LocalException ex, String operation) {
        // No need to convert remoteLogger as we only use its identity
        if (removeRemoteLogger(remoteLogger)) {
            if (_traceLevel > 0) {
                logger.trace(
                    _traceCategory,
                    "detached `" + remoteLogger.toString() + "' because " + operation + " raised:\n" + ex.toString());
            }
        }
    }

    int getTraceLevel() {
        return _traceLevel;
    }

    private synchronized boolean removeRemoteLogger(RemoteLoggerPrx remoteLogger) {
        return _remoteLoggerMap.remove(remoteLogger.ice_getIdentity()) != null;
    }

    private static void filterLogMessages(
            List<LogMessage> logMessages,
            Set<LogMessageType> messageTypes,
            Set<String> traceCategories,
            int messageMax) {
        assert (!logMessages.isEmpty() && messageMax != 0);

        // Filter only if one of the 3 filters is set;
        // messageMax < 0 means "give me all" that match the other filters, if any.
        if (!messageTypes.isEmpty() || !traceCategories.isEmpty() || messageMax > 0) {
            int count = 0;
            ListIterator<LogMessage> p = logMessages.listIterator(logMessages.size());
            while (p.hasPrevious()) {
                boolean keepIt = false;
                LogMessage msg = p.previous();
                if (messageTypes.isEmpty() || messageTypes.contains(msg.type)) {
                    if (msg.type != LogMessageType.TraceMessage
                        || traceCategories.isEmpty()
                        || traceCategories.contains(msg.traceCategory)) {
                        keepIt = true;
                    }
                }

                if (keepIt) {
                    ++count;
                    if (messageMax > 0 && count >= messageMax) {
                        if (p.hasPrevious()) {
                            int removeCount = p.previousIndex() + 1;
                            for (int i = 0; i < removeCount; i++) {
                                logMessages.remove(0);
                            }
                        }
                        break; // while
                    }
                } else {
                    p.remove();
                }
            }
        }
        // else, don't need any filtering
    }

    // Change this proxy's communicator, while keeping its invocation timeout
    private static RemoteLoggerPrx changeCommunicator(RemoteLoggerPrx prx, Communicator communicator) {
        if (prx == null) {
            return null;
        }

        return RemoteLoggerPrx.createProxy(
            communicator, prx.toString()).ice_invocationTimeout(prx.ice_getInvocationTimeout());
    }

    private static void copyProperties(String prefix, Properties from, Properties to) {
        for (Map.Entry<String, String> p : from.getPropertiesForPrefix(prefix).entrySet()) {
            to.setProperty(p.getKey(), p.getValue());
        }
    }

    private static Communicator createSendLogCommunicator(Communicator communicator, Logger logger) {
        InitializationData initData = new InitializationData();
        initData.logger = logger;
        initData.properties = new Properties();

        Properties mainProps = communicator.getProperties();

        copyProperties("Ice.Default.Locator", mainProps, initData.properties);
        copyProperties("IceSSL.", mainProps, initData.properties);

        String[] extraProps = mainProps.getIcePropertyAsList("Ice.Admin.Logger.Properties");

        if (extraProps.length > 0) {
            for (int i = 0; i < extraProps.length; i++) {
                String p = extraProps[i];
                if (!p.startsWith("--")) {
                    extraProps[i] = "--" + p;
                }
            }
            initData.properties.parseCommandLineOptions("", extraProps);
        }
        return Util.initialize(initData);
    }

    private final List<LogMessage> _queue = new LinkedList<>();
    private int _logCount; // non-trace messages
    private final int _maxLogCount;
    private int _traceCount;
    private final int _maxTraceCount;
    private final int _traceLevel;

    private int _oldestTrace = -1;
    private int _oldestLog = -1;

    private static class Filters {
        Filters(LogMessageType[] m, String[] c) {
            messageTypes = new HashSet<>(Arrays.asList(m));
            traceCategories = new HashSet<>(Arrays.asList(c));
        }

        final Set<LogMessageType> messageTypes;
        final Set<String> traceCategories;
    }

    private static class RemoteLoggerData {
        RemoteLoggerData(RemoteLoggerPrx prx, Filters f) {
            remoteLogger = prx;
            filters = f;
        }

        final RemoteLoggerPrx remoteLogger;
        final Filters filters;
    }

    private final Map<Identity, RemoteLoggerData> _remoteLoggerMap = new HashMap<>();

    private final LoggerAdminLoggerI _logger;
    private Communicator _sendLogCommunicator;
    private boolean _destroyed;
    private static final String _traceCategory = "Admin.Logger";
}
