// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

class EventHandlerOpPair {
    EventHandlerOpPair(EventHandler handler, int op) {
        this.handler = handler;
        this.op = op;
    }

    EventHandler handler;
    int op;
}
