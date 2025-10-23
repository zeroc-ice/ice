// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.util.ArrayDeque;
import java.util.Calendar;
import java.util.Deque;
import java.util.List;

final class LoggerAdminLoggerI implements LoggerAdminLogger, Runnable {
    @Override
    public void print(String message) {
        _localLogger.print(message);
        if (!_detached) {
            LogMessage logMessage = new LogMessage(LogMessageType.PrintMessage, now(), "", message);
            log(logMessage);
        }
    }

    @Override
    public void trace(String category, String message) {
        _localLogger.trace(category, message);
        if (!_detached) {
            LogMessage logMessage = new LogMessage(LogMessageType.TraceMessage, now(), category, message);
            log(logMessage);
        }
    }

    @Override
    public void warning(String message) {
        _localLogger.warning(message);
        if (!_detached) {
            LogMessage logMessage = new LogMessage(LogMessageType.WarningMessage, now(), "", message);
            log(logMessage);
        }
    }

    @Override
    public void error(String message) {
        _localLogger.error(message);
        if (!_detached) {
            LogMessage logMessage = new LogMessage(LogMessageType.ErrorMessage, now(), "", message);
            log(logMessage);
        }
    }

    @Override
    public String getPrefix() {
        return _localLogger.getPrefix();
    }

    @Override
    public Logger cloneWithPrefix(String prefix) {
        return _localLogger.cloneWithPrefix(prefix);
    }

    @Override
    public Object getFacet() {
        return _loggerAdmin;
    }

    @Override
    public void detach() {
        Thread thread = null;
        synchronized (this) {
            if (_sendLogThread != null) {
                thread = _sendLogThread;
                _sendLogThread = null;
                _detached = true;
                notifyAll();
            }
        }

        if (thread != null) {
            try {
                thread.join();
            } catch (InterruptedException e) {
                synchronized (this) {
                    _sendLogThread = thread;
                }
                throw new OperationInterruptedException(e);
            }
        }

        _loggerAdmin.destroy();
    }

    @Override
    public void close() throws Exception {
        _localLogger.close();
    }

    @Override
    public void run() {
        if (_loggerAdmin.getTraceLevel() > 1) {
            _localLogger.trace(_traceCategory, "send log thread started");
        }

        for (; ; ) {
            Job job = null;
            synchronized (this) {
                while (!_detached && _jobQueue.isEmpty()) {
                    try {
                        wait();
                    } catch (InterruptedException e) {
                        // Ignored, this should never occur
                    }
                }

                if (_detached) {
                    break;
                }

                assert (!_jobQueue.isEmpty());
                job = _jobQueue.removeFirst();
            }

            for (RemoteLoggerPrx p : job.remoteLoggers) {
                if (_loggerAdmin.getTraceLevel() > 1) {
                    _localLogger.trace(_traceCategory, "sending log message to `" + p.toString() + "'");
                }

                try {
                    // p is a proxy associated with the _sendLogCommunicator
                    p.logAsync(job.logMessage)
                        .whenComplete(
                            (Void v, Throwable ex) -> {
                                if (ex != null) {
                                    if (ex instanceof CommunicatorDestroyedException) {
                                        // Expected if there are outstanding calls during communicator destruction.
                                    } else if (ex instanceof LocalException) {
                                        _loggerAdmin.deadRemoteLogger(p, _localLogger, (LocalException) ex, "log");
                                    } else {
                                        _loggerAdmin.deadRemoteLogger(p, _localLogger, new UnknownException(ex), "log");
                                    }
                                } else {
                                    if (_loggerAdmin.getTraceLevel() > 1) {
                                        String msg = "log on `" + p.toString() + "' completed successfully";
                                        _localLogger.trace(_traceCategory, msg);
                                    }
                                }
                            });
                } catch (LocalException ex) {
                    _loggerAdmin.deadRemoteLogger(p, _localLogger, ex, "log");
                }
            }
        }

        if (_loggerAdmin.getTraceLevel() > 1) {
            _localLogger.trace(_traceCategory, "send log thread completed");
        }
    }

    LoggerAdminLoggerI(Properties props, Logger localLogger) {
        if (localLogger instanceof LoggerAdminLoggerI) {
            _localLogger = ((LoggerAdminLoggerI) localLogger).getLocalLogger();
        } else {
            _localLogger = localLogger;
        }

        _loggerAdmin = new LoggerAdminI(props, this);
    }

    Logger getLocalLogger() {
        return _localLogger;
    }

    void log(LogMessage logMessage) {
        List<RemoteLoggerPrx> remoteLoggers = _loggerAdmin.log(logMessage);

        if (remoteLoggers != null) {
            assert (!remoteLoggers.isEmpty());

            synchronized (this) {
                if (_sendLogThread == null) {
                    _sendLogThread = new Thread(this, "Ice.SendLogThread");
                    _sendLogThread.start();
                }

                _jobQueue.addLast(new Job(remoteLoggers, logMessage));
                notifyAll();
            }
        }
    }

    private static long now() {
        return Calendar.getInstance().getTimeInMillis() * 1000;
    }

    private static class Job {
        Job(List<RemoteLoggerPrx> r, LogMessage l) {
            remoteLoggers = r;
            logMessage = l;
        }

        final List<RemoteLoggerPrx> remoteLoggers;
        final LogMessage logMessage;
    }

    private final Logger _localLogger;
    private final LoggerAdminI _loggerAdmin;
    private volatile boolean _detached;
    private Thread _sendLogThread;
    private final Deque<Job> _jobQueue = new ArrayDeque<>();

    private static final String _traceCategory = "Admin.Logger";
}
