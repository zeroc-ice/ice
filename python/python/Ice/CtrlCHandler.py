# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#
import threading
import signal

#
# Its not possible to block in a python signal handler since this
# blocks the main thread from doing further work. As such we queue the
# signal with a worker thread which then "dispatches" the signal to
# the registered callback object.
#
# Note the interface is the same as the C++ CtrlCHandler
# implementation, however, the implementation is different.
#


class CtrlCHandler(threading.Thread):
    # Class variable referring to the one and only handler for use
    # from the signal handling callback.
    _self = None

    def __init__(self):
        threading.Thread.__init__(self)

        if CtrlCHandler._self is not None:
            raise RuntimeError(
                "Only a single instance of a CtrlCHandler can be instantiated."
            )
        CtrlCHandler._self = self

        # State variables. These are not class static variables.
        self._condVar = threading.Condition()
        self._queue = []
        self._done = False
        self._callback = None

        #
        # Setup and install signal handlers
        #
        if "SIGHUP" in signal.__dict__:
            signal.signal(signal.SIGHUP, CtrlCHandler.signalHandler)
        if "SIGBREAK" in signal.__dict__:
            signal.signal(signal.SIGBREAK, CtrlCHandler.signalHandler)
        signal.signal(signal.SIGINT, CtrlCHandler.signalHandler)
        signal.signal(signal.SIGTERM, CtrlCHandler.signalHandler)

        # Start the thread once everything else is done.
        self.start()

    # Dequeue and dispatch signals.
    def run(self):
        while True:
            self._condVar.acquire()
            while len(self._queue) == 0 and not self._done:
                self._condVar.wait()
            if self._done:
                self._condVar.release()
                break
            sig, callback = self._queue.pop()
            self._condVar.release()
            if callback:
                callback(sig)

    # Destroy the object. Wait for the thread to terminate and cleanup
    # the internal state.
    def destroy(self):
        self._condVar.acquire()
        self._done = True
        self._condVar.notify()
        self._condVar.release()

        # Wait for the thread to terminate
        self.join()
        #
        # Cleanup any state set by the CtrlCHandler.
        #
        if "SIGHUP" in signal.__dict__:
            signal.signal(signal.SIGHUP, signal.SIG_DFL)
        if "SIGBREAK" in signal.__dict__:
            signal.signal(signal.SIGBREAK, signal.SIG_DFL)
        signal.signal(signal.SIGINT, signal.SIG_DFL)
        signal.signal(signal.SIGTERM, signal.SIG_DFL)
        CtrlCHandler._self = None

    def setCallback(self, callback):
        self._condVar.acquire()
        self._callback = callback
        self._condVar.release()

    def getCallback(self):
        self._condVar.acquire()
        callback = self._callback
        self._condVar.release()
        return callback

    # Private. Only called by the signal handling mechanism.
    def signalHandler(self, sig, frame):
        self._self._condVar.acquire()
        #
        # The signal AND the current callback are queued together.
        #
        self._self._queue.append([sig, self._self._callback])
        self._self._condVar.notify()
        self._self._condVar.release()

    signalHandler = classmethod(signalHandler)
