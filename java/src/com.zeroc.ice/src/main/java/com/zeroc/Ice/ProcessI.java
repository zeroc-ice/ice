// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

class ProcessI implements Process {
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
            case 1 -> System.out.println(message);
            case 2 -> System.err.println(message);
        }
    }

    private final Communicator _communicator;
}
