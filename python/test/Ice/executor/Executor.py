#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import threading


def test(b):
    if not b:
        raise RuntimeError("test assertion failed")


class Executor:
    def __init__(self):
        self._calls = []
        self._terminated = False
        self._cond = threading.Condition()
        self._thread = threading.Thread(target=self.run)
        self._thread.start()
        Executor._instance = self

    def execute(self, call, con):
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
                    # Terminate only once all calls are executed.
                    return

            if call:
                try:
                    call()
                except Exception:
                    # Exceptions should never propagate here.
                    test(False)

    @staticmethod
    def terminate():
        with Executor._instance._cond:
            Executor._instance._terminated = True
            Executor._instance._cond.notify()

        Executor._instance._thread.join()
        Executor._instance = None

    @staticmethod
    def isExecutorThread():
        return threading.current_thread() == Executor._instance._thread

    @staticmethod
    def instance():
        return Executor._instance
