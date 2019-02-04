#!/usr/bin/env python
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

import sys, Ice, time

class Client(Ice.Application):
    def interruptCallback(self, sig):
        print("handling signal " + str(sig))

    # SIGINT interrupts time.sleep so a custom method is needed to
    # sleep for a given interval.
    def sleep(self, interval):
        start = time.time()
        while True:
            sleepTime = (start + interval) - time.time()
            if sleepTime <= 0:
                break
            time.sleep(sleepTime)

    def run(self, args):
        self.ignoreInterrupt()
        print("Ignore CTRL+C and the like for 5 seconds (try it!)")
        self.sleep(5)

        self.callbackOnInterrupt()

        self.holdInterrupt()
        print("Hold CTRL+C and the like for 5 seconds (try it!)")
        self.sleep(5)

        self.releaseInterrupt()
        print("Release CTRL+C (any held signals should be released)")
        self.sleep(5)

        self.holdInterrupt()
        print("Hold CTRL+C and the like for 5 seconds (try it!)")
        self.sleep(5)

        self.callbackOnInterrupt()
        print("Release CTRL+C (any held signals should be released)")
        self.sleep(5)

        self.shutdownOnInterrupt()
        print("Test shutdown on destroy. Press CTRL+C to shutdown & terminate")
        self.communicator().waitForShutdown()

        print("ok")
        return False

app = Client()
sys.exit(app.main(sys.argv))
