// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

class ProcessI implements com.zeroc.Ice.Process {
    public ProcessI(Communicator communicator) {
        _communicator = communicator;
    }

    @Override
    public void shutdown(Current current) {
        _communicator.shutdown();
    }

    @Override
    public void writeMessage(String message, int fd, Current current) {
        switch (fd) {
            case 1:
                {
                    System.out.println(message);
                    break;
                }
            case 2:
                {
                    System.err.println(message);
                    break;
                }
        }
    }

    private Communicator _communicator;
}
