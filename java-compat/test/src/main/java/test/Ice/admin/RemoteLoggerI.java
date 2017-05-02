// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.admin;

class RemoteLoggerI extends Ice._RemoteLoggerDisp
{

    @Override
    public synchronized void init(String prefix, Ice.LogMessage[] logMessages, Ice.Current current)
    {
        test(prefix.equals(_expectedPrefix));
        test(java.util.Arrays.equals(logMessages, _expectedInitMessages));
        _receivedCalls++;
        notifyAll();
    }

    @Override
    public synchronized void log(Ice.LogMessage logMessage, Ice.Current current)
    {
        Ice.LogMessage front = _expectedLogMessages.pollFirst();
        test(front.type == logMessage.type && front.message.equals(logMessage.message) &&
             front.traceCategory.equals(logMessage.traceCategory));

        _receivedCalls++;
        notifyAll();
    }

    synchronized void checkNextInit(String prefix, Ice.LogMessage[] logMessages)
    {
        _expectedPrefix = prefix;
        _expectedInitMessages = logMessages;
    }

    synchronized void checkNextLog(Ice.LogMessageType messageType, String message, String category)
    {
        Ice.LogMessage logMessage = new Ice.LogMessage(messageType, 0, category, message);
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

    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    private int _receivedCalls;
    private String _expectedPrefix;
    private Ice.LogMessage[] _expectedInitMessages;
    private java.util.Deque<Ice.LogMessage> _expectedLogMessages = new java.util.ArrayDeque<Ice.LogMessage>();
}
