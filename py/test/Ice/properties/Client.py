#!/usr/bin/env python
# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************


import os, sys, traceback
import Ice

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')
    
class Client(Ice.Application):
    def run(self, args):
        properties = self.communicator().getProperties()
        test(properties.getProperty("Ice.Trace.Network") == "1")
        test(properties.getProperty("Ice.Trace.Protocol") == "1")
        test(properties.getProperty("Config.Path") == "./config/中国_client.config")
        test(properties.getProperty("Ice.ProgramName") == "PropertiesClient")
        test(self.appName() == properties.getProperty("Ice.ProgramName"))


print "testing load properties from UTF-8 path... ",
id = Ice.InitializationData()
id.properties = Ice.createProperties()
id.properties.load("./config/中国_client.config")
test(id.properties.getProperty("Ice.Trace.Network") == "1")
test(id.properties.getProperty("Ice.Trace.Protocol") == "1")
test(id.properties.getProperty("Config.Path") == "./config/中国_client.config")
test(id.properties.getProperty("Ice.ProgramName") == "PropertiesClient")
print "ok"
print "testing load properties from UTF-8 path using Ice::Application... ",
c  = Client()
c.main(sys.argv, "./config/中国_client.config")
print "ok"

sys.exit(0)