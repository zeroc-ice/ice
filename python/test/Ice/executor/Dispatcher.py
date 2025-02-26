#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import threading


def test(b):
    if not b:
        raise RuntimeError("test assertion failed")


class Dispatcher:
    def __init__(self):
        self._calls = []
        self._terminated = False
        self._cond = threading.Condition()
        self._thread = threading.Thread(target=self.run)
        self._thread.start()
        Dispatcher._instance = self

    def dispatch(self, call, con):
        with self._cond:
            self._calls.append(call)
            if len(self._calls) == 1:
                self._cond.notify()

    def run(self):
        while True:
            call = None
            with self._cond:
                while not self._terminated and len(self._calls) == 0:
                    self._cond.wait()
                if len(self._calls) > 0:
                    call = self._calls.pop(0)
                elif self._terminated:
                    # Terminate only once all calls are dispatched.
                    return

            if call:
                try:
                    call()
                except Exception:
                    # Exceptions should never propagate here.
                    test(False)

    @staticmethod
    def terminate():
        with Dispatcher._instance._cond:
            Dispatcher._instance._terminated = True
            Dispatcher._instance._cond.notify()

        Dispatcher._instance._thread.join()
        Dispatcher._instance = None

    @staticmethod
    def isDispatcherThread():
        return threading.current_thread() == Dispatcher._instance._thread

    @staticmethod
    def instance():
        return Dispatcher._instance
