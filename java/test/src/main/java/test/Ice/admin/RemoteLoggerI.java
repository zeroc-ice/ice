// Copyright (c) ZeroC, Inc.

package test.Ice.admin;

import com.zeroc.Ice.Current;
import com.zeroc.Ice.LogMessage;
import com.zeroc.Ice.LogMessageType;
import com.zeroc.Ice.RemoteLogger;

import java.util.ArrayDeque;
import java.util.Deque;

class RemoteLoggerI implements RemoteLogger {
    @Override
    public synchronized void init(
            String prefix, LogMessage[] logMessages, Current current) {
        _prefix = prefix;
        for (int i = 0; i < logMessages.length; i++) {
            _initMessages.add(logMessages[i]);
        }
        _receivedCalls++;
        notifyAll();
    }

    @Override
    public synchronized void log(
            LogMessage logMessage, Current current) {
        _logMessages.add(logMessage);
        _receivedCalls++;
        notifyAll();
    }

    synchronized void checkNextInit(
            String prefix, LogMessageType type, String message, String category) {
        test(_prefix.equals(prefix));
        test(!_initMessages.isEmpty());
        LogMessage logMessage = _initMessages.pop();
        test(logMessage.type == type);
        test(logMessage.message.equals(message));
        test(logMessage.traceCategory.equals(category));
    }

    synchronized void checkNextLog(
            LogMessageType type, String message, String category) {
        test(!_logMessages.isEmpty());
        LogMessage logMessage = _logMessages.pop();
        test(logMessage.type == type);
        test(logMessage.message.equals(message));
        test(logMessage.traceCategory.equals(category));
    }

    synchronized void wait(int calls) {
        _receivedCalls -= calls;

        while (_receivedCalls < 0) {
            try {
                wait();
            } catch (InterruptedException ex) {
                break;
            }
        }
    }

    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    private int _receivedCalls;
    private String _prefix;
    private final Deque<LogMessage> _initMessages =
            new ArrayDeque<LogMessage>();
    private final Deque<LogMessage> _logMessages =
            new ArrayDeque<LogMessage>();
}
