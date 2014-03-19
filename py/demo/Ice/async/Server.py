#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, os, traceback, threading, Ice

Ice.loadSlice('Hello.ice')
import Demo

class CallbackEntry(object):
    def __init__(self, cb, delay):
        self.cb = cb
        self.delay = delay

class WorkQueue(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
        self._callbacks = []
        self._done = False
        self._cond = threading.Condition()

    def run(self):
        self._cond.acquire()

        try:
            while not self._done:
                if len(self._callbacks) == 0:
                    self._cond.wait()

                if not len(self._callbacks) == 0:
                    self._cond.wait(self._callbacks[0].delay / 1000.0)

                    if not self._done:
                        print("Belated Hello World!")
                        self._callbacks[0].cb.ice_response()
                        del self._callbacks[0]

            for i in range(0, len(self._callbacks)):
                self._callbacks[i].cb.ice_exception(Demo.RequestCanceledException())
        finally:
            self._cond.release()

    def add(self, cb, delay):
        self._cond.acquire()

        try:
            if not self._done:
                entry = CallbackEntry(cb, delay)
                if len(self._callbacks) == 0:
                    self._cond.notify()
                self._callbacks.append(entry)
            else:
               cb.ice_exception(Demo.RequestCanceledException())
        finally:
            self._cond.release()

    def destroy(self):
        self._cond.acquire()

        try:
            self._done = True
            self._cond.notify()
        finally:
            self._cond.release()

class HelloI(Demo.Hello):
    def __init__(self, workQueue):
        self._workQueue = workQueue

    def sayHello_async(self, cb, delay, current=None):
        if delay == 0:
            print("Hello World!")
            cb.ice_response()
        else:
            self._workQueue.add(cb, delay)

    def shutdown(self, current=None):
        self._workQueue.destroy()
        current.adapter.getCommunicator().shutdown();

class Server(Ice.Application):
    def run(self, args):
        if len(args) > 1:
            print(self.appName() + ": too many arguments")
            return 1

        self.callbackOnInterrupt()

        adapter = self.communicator().createObjectAdapter("Hello")
        self._workQueue = WorkQueue()
        adapter.add(HelloI(self._workQueue), self.communicator().stringToIdentity("hello"))
        
        self._workQueue.start()
        adapter.activate()
        
        self.communicator().waitForShutdown()
        self._workQueue.join()
        return 0

    def interruptCallback(self, sig):
        self._workQueue.destroy()
        self._communicator.shutdown()

app = Server()
sys.exit(app.main(sys.argv, "config.server"))
