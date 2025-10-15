// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.io.IOException;
import java.nio.channels.CancelledKeyException;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

final class Selector {
    static final class TimeoutException extends java.lang.Exception {
        private static final long serialVersionUID = 7885765825975312023L;
    }

    Selector(Instance instance) {
        _instance = instance;
        _selecting = false;
        _interrupted = false;

        try {
            _selector = java.nio.channels.Selector.open();
        } catch (IOException ex) {
            throw new SyscallException(ex);
        }

        //
        // The Selector holds a Set representing the selected keys. The
        // Set reference doesn't change, so we obtain it once here.
        //
        _keys = _selector.selectedKeys();
    }

    void destroy() {
        if (_selector != null) {
            try {
                _selector.close();
            } catch (IOException ex) {}
            _selector = null;
        }
    }

    void initialize(EventHandler handler) {
        if (handler.fd() != null) {
            updateImpl(handler);
        }
    }

    void update(EventHandler handler, int remove, int add) {
        int previous = handler._registered;
        handler._registered = handler._registered & ~remove;
        handler._registered = handler._registered | add;
        if (previous == handler._registered) {
            return;
        }

        if (handler.fd() != null) {
            updateImpl(handler);
        }

        checkReady(handler);
    }

    void enable(EventHandler handler, int status) {
        if ((handler._disabled & status) == 0) {
            return;
        }
        handler._disabled = handler._disabled & ~status;

        if (handler._key != null && (handler._registered & status) != 0) {
            updateImpl(handler); // If registered with the selector, update the registration.
        }
        checkReady(handler);
    }

    void disable(EventHandler handler, int status) {
        if ((handler._disabled & status) != 0) {
            return;
        }
        handler._disabled = handler._disabled | status;

        if (handler._key != null && (handler._registered & status) != 0) {
            updateImpl(handler); // If registered with the selector, update the registration.
        }
        checkReady(handler);
    }

    boolean finish(EventHandler handler, boolean closeNow) {
        handler._registered = 0;
        if (handler._key != null) {
            handler._key.cancel();
            handler._key = null;
        }
        _changes.remove(handler);
        checkReady(handler);
        return closeNow;
    }

    void ready(EventHandler handler, int status, boolean value) {
        if (((handler._ready & status) != 0) == value) {
            return; // Nothing to do if ready state already correctly set.
        }

        if (value) {
            handler._ready |= status;
        } else {
            handler._ready &= ~status;
        }
        checkReady(handler);
    }

    void startSelect() {
        if (!_changes.isEmpty()) {
            updateSelector();
        }
        _selecting = true;

        //
        // If there are ready handlers, don't block in select, just do a non-blocking
        // select to retrieve new ready handlers from the Java selector.
        //
        _selectNow = !_readyHandlers.isEmpty();
    }

    void finishSelect(List<EventHandlerOpPair> handlers) {
        assert (handlers.isEmpty());

        // If key set is empty and we weren't woken up.
        if (_keys.isEmpty() && _readyHandlers.isEmpty() && !_interrupted) {
            // This is necessary to prevent a busy loop in case of a spurious wake-up which
            // sometime occurs in the client thread pool when the communicator is destroyed.
            // If there are too many successive spurious wake-ups, we log an error.
            try {
                Thread.sleep(1);
            } catch (InterruptedException ex) {
                // Eat the InterruptedException (as we do in ThreadPool.promoteFollower).
            }

            if (++_spuriousWakeUp > 100) {
                _spuriousWakeUp = 0;
                _instance.initializationData().logger.warning("spurious selector wake up");
            }
            return;
        }
        _interrupted = false;
        _spuriousWakeUp = 0;

        for (SelectionKey key : _keys) {
            EventHandler handler = (EventHandler) key.attachment();
            try {
                // Use the intersection of readyOps and interestOps because we only want to
                // report the operations in which the handler is still interested.
                final int op = fromJavaOps(key.readyOps() & key.interestOps());

                // Handler will be added by the loop below.
                if (!_readyHandlers.contains(handler)) {
                    handlers.add(new EventHandlerOpPair(handler, op));
                }
            } catch (CancelledKeyException ex) {
                assert (handler._registered == 0);
            }
        }

        for (EventHandler handler : _readyHandlers) {
            int op = handler._ready & ~handler._disabled & handler._registered;
            if (handler._key != null && _keys.contains(handler._key)) {
                op |= fromJavaOps(handler._key.readyOps() & handler._key.interestOps());
            }
            if (op > 0) {
                handlers.add(new EventHandlerOpPair(handler, op));
            }
        }

        _keys.clear();
        _selecting = false;
    }

    void select(long timeout) throws TimeoutException {
        try {
            if (_selectNow) {
                _selector.selectNow();
            } else if (timeout > 0) {
                if (_selector.select(timeout * 1000) == 0 && !_interrupted) {
                    throw new TimeoutException();
                }
            } else {
                _selector.select();
            }
        } catch (IOException ex) {
            _instance.initializationData().logger.error("selector failed:\n" + Ex.toString(ex));
            Runtime.getRuntime().halt(1);
        }
    }

    private void updateImpl(EventHandler handler) {
        _changes.add(handler);
        wakeup();
    }

    private void updateSelector() {
        for (EventHandler handler : _changes) {
            int status = handler._registered & ~handler._disabled;
            int ops = toJavaOps(handler, status);
            if (handler._key == null) {
                if (handler._registered != 0) {
                    try {
                        handler._key = handler.fd().register(_selector, ops, handler);
                    } catch (ClosedChannelException ex) {
                        assert false;
                    }
                }
            } else {
                handler._key.interestOps(ops);
            }
        }
        _changes.clear();
    }

    private void checkReady(EventHandler handler) {
        if ((handler._ready & ~handler._disabled & handler._registered) != 0) {
            _readyHandlers.add(handler);
            wakeup();
        } else {
            _readyHandlers.remove(handler);
        }
    }

    private void wakeup() {
        if (_selecting && !_interrupted) {
            _selector.wakeup();
            _interrupted = true;
        }
    }

    private int toJavaOps(EventHandler handler, int o) {
        int op = 0;
        if ((o & SocketOperation.Read) != 0) {
            if ((handler.fd().validOps() & SelectionKey.OP_READ) != 0) {
                op |= SelectionKey.OP_READ;
            } else {
                op |= SelectionKey.OP_ACCEPT;
            }
        }
        if ((o & SocketOperation.Write) != 0) {
            op |= SelectionKey.OP_WRITE;
        }
        if ((o & SocketOperation.Connect) != 0) {
            op |= SelectionKey.OP_CONNECT;
        }
        return op;
    }

    private int fromJavaOps(int o) {
        int op = 0;
        if ((o
            & (SelectionKey.OP_READ
            | SelectionKey.OP_ACCEPT))
            != 0) {
            op |= SocketOperation.Read;
        }
        if ((o & SelectionKey.OP_WRITE) != 0) {
            op |= SocketOperation.Write;
        }
        if ((o & SelectionKey.OP_CONNECT) != 0) {
            op |= SocketOperation.Connect;
        }
        return op;
    }

    private final Instance _instance;

    private java.nio.channels.Selector _selector;
    private final Set<SelectionKey> _keys;
    private final HashSet<EventHandler> _changes = new HashSet<>();
    private final HashSet<EventHandler> _readyHandlers = new HashSet<>();
    private boolean _selecting;
    private boolean _selectNow;
    private boolean _interrupted;
    private int _spuriousWakeUp;
}
