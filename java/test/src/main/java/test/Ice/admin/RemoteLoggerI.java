// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.admin;

class RemoteLoggerI implements com.zeroc.Ice.RemoteLogger
{
    @Override
    public synchronized void init(String prefix, com.zeroc.Ice.LogMessage[] logMessages, com.zeroc.Ice.Current current)
    {
        test(prefix.equals(_expectedPrefix));
        test(java.util.Arrays.equals(logMessages, _expectedInitMessages));
        _receivedCalls++;
        notifyAll();
    }

    @Override
    public synchronized void log(com.zeroc.Ice.LogMessage logMessage, com.zeroc.Ice.Current current)
    {
        com.zeroc.Ice.LogMessage front = _expectedLogMessages.pollFirst();
        test(front.type == logMessage.type && front.message.equals(logMessage.message) &&
             front.traceCategory.equals(logMessage.traceCategory));

        _receivedCalls++;
        notifyAll();
    }

    synchronized void checkNextInit(String prefix, com.zeroc.Ice.LogMessage[] logMessages)
    {
        _expectedPrefix = prefix;
        _expectedInitMessages = logMessages;
    }

    synchronized void checkNextLog(com.zeroc.Ice.LogMessageType messageType, String message, String category)
    {
        com.zeroc.Ice.LogMessage logMessage = new com.zeroc.Ice.LogMessage(messageType, 0, category, message);
        _expectedLogMessages.addLast(logMessage);
    }

    synchronized void wait(int calls)
    {
        _receivedCalls -= calls;

        while(_receivedCalls < 0)
        {
            try
            {
                wait();
            }
            catch(InterruptedException ex)
            {
                break;
            }
        }
    }

    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    private int _receivedCalls;
    private String _expectedPrefix;
    private com.zeroc.Ice.LogMessage[] _expectedInitMessages;
    private java.util.Deque<com.zeroc.Ice.LogMessage> _expectedLogMessages =
        new java.util.ArrayDeque<com.zeroc.Ice.LogMessage>();
}
