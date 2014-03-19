#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, traceback, Ice, threading

Ice.loadSlice("Contact.ice")
import Demo

class ContactDBI(Demo.ContactDB):
    def __init__(self):
        self._contacts = {}

    def addContact(self, name, type, number, dialGroup, current = None):
        contact = Demo.Contact()
        contact.name = name
        if type != Ice.Unset:
            contact.type = type
        if number != Ice.Unset:
            contact.number = number
        if dialGroup != Ice.Unset:
            contact.dialGroup = dialGroup
        self._contacts[name] = contact

    def updateContact(self, name, type, number, dialGroup, current = None):
        if name in self._contacts:
            contact = self._contacts[name]
            if type != Ice.Unset:
                contact.type = type
            if number != Ice.Unset:
                contact.number = number
            if dialGroup != Ice.Unset:
                contact.dialGroup = dialGroup

    def query(self, name, current = None):
        if name in self._contacts:
            return self._contacts[name]
        return None

    def queryNumber(self, name, current = None):
        if name in self._contacts:
            return self._contacts[name].number
        return Ice.Unset

    def queryDialgroup(self, name, current = None):
        if name in self._contacts:
            return self._contacts[name].dialGroup
        return Ice.Unset

    def shutdown(self, current = None):
        print("Shutting down...")
        current.adapter.getCommunicator().shutdown();

class ContactServer(Ice.Application):
    def run(self, args):
        if len(args) > 1:
            print(self.appName() + ": too many arguments")
            return 1

        adapter = self.communicator().createObjectAdapter("ContactDB")
        adapter.add(ContactDBI(), self.communicator().stringToIdentity("contactdb"))
        adapter.activate()
        self.communicator().waitForShutdown()
        return 0

sys.stdout.flush()
app = ContactServer()
sys.exit(app.main(sys.argv, "config.server"))
