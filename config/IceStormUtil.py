#!/usr/bin/env python

# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import TestUtil
import os, sys

global testdir
global toplevel

origIceBoxEndpoints = ' --IceBox.ServiceManager.Endpoints="default -p %d" --Ice.Default.Locator='

# Turn off the dispatch and connection warnings -- they are expected
# when using a replicated IceStorm.
origIceStormService = ' --IceBox.Service.IceStorm=IceStormService,' + TestUtil.getIceSoVersion() + ':createIceStorm' + \
                  ' --IceStorm.TopicManager.Endpoints="default -p %d"' + \
                  ' --IceBox.PrintServicesReady=IceStorm' + \
                  ' --IceBox.InheritProperties=1' + \
                  ' --Ice.Warn.Dispatch=0 --Ice.Warn.Connections=0' + \
                  ' --Ice.ServerIdleTime=0'
origIceStormProxy = '%s/TopicManager:default -p %d'
origIceStormReference = ' --IceStormAdmin.TopicManager.Default="%s"'

def printOutput(pipe):
    try:
        while True:
            line = pipe.readline()
            if not line:
                break
            print line,
    except IOError:
        pass

def captureOutput(pipe):
    out = ""
    try:
        while True:
            line = pipe.readline()
            if not line:
                break
            out = out + line
    except IOError:
        pass
    return out

class IceStormUtil(object):
    def __init__(self, toplevel, testdir):
        self.toplevel = toplevel
        self.testdir = testdir
        self.iceBox = TestUtil.getIceBox(testdir)
        self.iceBoxAdmin = os.path.join(TestUtil.getCppBinDir(), "iceboxadmin")
        self.iceStormAdmin = os.path.join(TestUtil.getCppBinDir(), "icestormadmin")

    def runIceBoxAdmin(self, endpts, command):
        clientCfg = TestUtil.DriverConfig("client")
        pipe = TestUtil.startClient(self.iceBoxAdmin, endpts + " " + command)
        out = captureOutput(pipe)
        status = TestUtil.closePipe(pipe)
        if status:
            print "non-zero status! %d" % status
            print out
            TestUtil.killServers()
            sys.exit(1)
        return out

    def admin(self, cmd, **args):
        return self.adminWithRef(self.iceStormReference, cmd, **args)

    def adminWithRef(self, ref, cmd, terminateOnError=True):
        pipe = TestUtil.startClient(self.iceStormAdmin, ref + r' -e "%s"' % cmd)
        out = captureOutput(pipe)
        status = TestUtil.closePipe(pipe)
        if terminateOnError and status:
            print "failed!"
            TestUtil.killServers()
            sys.exit(1)
        return status, out.strip()

    def reference(self):
        return self.iceStormReference
    def proxy(self):
        return self.iceStormProxy

class Replicated(IceStormUtil):
    def __init__(self, toplevel, testdir,
                 additional = None,
                 replicatedPublisher = True,
                 dbDir = "db",
                 instanceName="IceStorm", port = 12010):
        IceStormUtil.__init__(self, toplevel, testdir)
        self.pipes = []
        self.nendpoints = [] # Node endpoints
        self.instanceName = instanceName
        self.ibendpoints = [] # IceBox endpoints
        self.isendpoints = [] # TopicManager endpoints
        self.ipendpoints = [] # Publisher endpoints
        for replica in range(0, 3):
            self.nendpoints.append(port + 100 + replica * 10)
            self.ibendpoints.append(port + replica * 10)
            self.isendpoints.append(port + (replica * 10)+1)
            self.ipendpoints.append(port + (replica * 10)+2)
        replicaProperties = ""
        sep =''
        replicaTopicManagerEndpoints = ''
        replicaPublishEndpoints = ''
        for replica in range(0, 3):
            replicaProperties = replicaProperties + \
                ' --IceStorm.Nodes.%d="%s/node%d:default -p %d -t 1000"' % (
                replica, instanceName, replica, self.nendpoints[replica])
            replicaTopicManagerEndpoints = replicaTopicManagerEndpoints + "%sdefault -p %d" % (
                sep, self.isendpoints[replica])
            replicaPublishEndpoints = replicaPublishEndpoints + "%sdefault -p %d" % (sep, self.ipendpoints[replica])
            sep = ':'
        replicaProperties = replicaProperties + \
            ' --IceStorm.NodeId=%d --IceStorm.Node.Endpoints="default -p %d"' + \
            ' --IceStorm.ReplicatedTopicManagerEndpoints="' +\
            replicaTopicManagerEndpoints + '"'
        if replicatedPublisher:
            replicaProperties = replicaProperties + \
                ' --IceStorm.ReplicatedPublishEndpoints="' + replicaPublishEndpoints + '"'
        self.iceBoxEndpoints = []
        self.iceStormEndpoints = []
        self.replicaProperties = []
        self.dbHome= []
        self.iceStormDBEnv= []
        self.pipes = []
        for replica in range(0, 3):
            self.iceBoxEndpoints.append(origIceBoxEndpoints % self.ibendpoints[replica])
            service = origIceStormService % self.isendpoints[replica]
            service = service + ' --IceStorm.Publish.Endpoints="default -p %d:udp -p %d"' % (
                self.ipendpoints[replica], self.ipendpoints[replica])
            if instanceName:
                service = service + ' --IceStorm.InstanceName=%s ' % instanceName
            #service = service + ' --IceStorm.Trace.Election=1'
            #service = service + ' --IceStorm.Trace.Replication=1'
            #service = service + ' --IceStorm.Trace.Subscriber=1'
            #service = service + ' --IceStorm.Trace.Topic=1'
            #service = service + ' --Ice.Trace.Network=3'
            #service = service + ' --Ice.Trace.Protocol=1'
            if additional:
                service = service + " " + additional
            self.iceStormEndpoints.append(service)
            self.replicaProperties.append(replicaProperties % (replica, self.nendpoints[replica]))
            dbHome = os.path.join(self.testdir, "%d.%s" % (replica, dbDir))
            self.dbHome.append(dbHome)
            TestUtil.cleanDbDir(dbHome)
            self.iceStormDBEnv.append(" --Freeze.DbEnv.IceStorm.DbHome=%s" % dbHome)
            self.pipes.append(None)

        topicReplicaProxy = '%s/TopicManager:%s' % (instanceName, replicaTopicManagerEndpoints)
        self.iceStormProxy = topicReplicaProxy
        self.iceStormReference = ' --IceStormAdmin.TopicManager.Default="%s"' % topicReplicaProxy

    def adminForReplica(self, replica, cmd, **args):
        ep = self.isendpoints[replica]
        proxy = origIceStormProxy % (self.instanceName, self.isendpoints[replica])
        ref = origIceStormReference % proxy
        return self.adminWithRef(ref, cmd, **args)

    def clean(self):
        for replica in range(0, 3):
            TestUtil.cleanDbDir(self.dbHome[replica])

    def start(self, echo = True, **args):
        if echo:
            print "starting icestorm replicas...",
            sys.stdout.flush()
        # Start replicas.
        for replica in range(0, 3):
            if echo:
                print replica,
                sys.stdout.flush()
            self.startReplica(replica, echo=False, **args)
        if echo:
            print "ok"

    def startReplica(self, replica, echo = True, additionalOptions = "", createThread = True):
        if echo:
            print "starting icestorm replica %d..." % replica,
            sys.stdout.flush()

        pipe = TestUtil.startServer(self.iceBox,
                                    self.iceBoxEndpoints[replica] +
                                    self.iceStormEndpoints[replica] +
                                    self.replicaProperties[replica] +
                                    self.iceStormDBEnv[replica] +
                                    additionalOptions)
        self.pipes.append(pipe)
        TestUtil.getServerPid(pipe)
        TestUtil.waitServiceReady(pipe, "IceStorm")
        self.pipes[replica] = pipe
        if echo:
            print "ok"

    def stop(self):
        for replica in range(0, 3):
            self.stopReplica(replica)

    def stopReplica(self, replica):
        if self.pipes[replica]:
            self.runIceBoxAdmin(self.iceBoxEndpoints[replica], "shutdown")
            if TestUtil.specificServerStatus(self.pipes[replica]):
                print "failed!"
                TestUtil.killServers()
                sys.exit(1)
            self.pipes[replica] = None

    def reference(self, replica=-1):
        if replica == -1:
            return self.iceStormReference
        ep = self.isendpoints[replica]
        proxy = origIceStormProxy % (self.instanceName, self.isendpoints[replica])
        return origIceStormReference % proxy

class NonReplicated(IceStormUtil):
    def __init__(self, toplevel, testdir, transient, \
                 additional = None, dbDir = "db", instanceName=None, port = 12010):
        IceStormUtil.__init__(self, toplevel, testdir)
        iceBoxPort = port
        iceStormPort = port + 1
        publisherPort = port + 2
        self.iceBoxEndpoints = origIceBoxEndpoints % (iceBoxPort)
        self.iceStormService = origIceStormService % (iceStormPort)
        self.dbDir = dbDir

        if instanceName:
              self.iceStormService = self.iceStormService + ' --IceStorm.InstanceName=%s ' % instanceName
        else:
            instanceName = "IceStorm"

        if publisherPort:
            self.iceStormService = self.iceStormService + ' --IceStorm.Publish.Endpoints="default -p %d:udp -p %d"' % (
                publisherPort, publisherPort)
        else:
            self.iceStormService = self.iceStormService + ' --IceStorm.Publish.Endpoints="default:udp"'
        self.transient = transient
        if self.transient:
            self.iceStormService = self.iceStormService + " --IceStorm.Transient=1"
        if additional:
            self.iceStormService = self.iceStormService + " " + additional
        self.iceStormProxy = origIceStormProxy % (instanceName, iceStormPort)
        self.iceStormReference = origIceStormReference % self.iceStormProxy

        self.dbHome = os.path.join(self.testdir, self.dbDir)
        TestUtil.cleanDbDir(self.dbHome)
        self.iceStormDBEnv=" --Freeze.DbEnv.IceStorm.DbHome=" + self.dbHome

    def clean(self):
        TestUtil.cleanDbDir(self.dbHome)

    def start(self, echo = True, additionalOptions = "", createThread = True):
        if echo:
            if self.transient:
                print "starting transient icestorm service...",
            else:
                print "starting icestorm service...",
            sys.stdout.flush()

        self.pipe = TestUtil.startServer(self.iceBox,
                                         self.iceBoxEndpoints +
                                         self.iceStormService +
                                         self.iceStormDBEnv +
                                         additionalOptions)
        TestUtil.getServerPid(self.pipe)
        TestUtil.waitServiceReady(self.pipe, "IceStorm", createThread)
        if echo:
            print "ok"
        return self.pipe

    def stop(self):
        self.runIceBoxAdmin(self.iceBoxEndpoints, "shutdown")
        if TestUtil.specificServerStatus(self.pipe):
            print "failed!"
            TestUtil.killServers()
            sys.exit(1)

def init(toplevel, testdir, type, **args):
    if type == "replicated":
        return Replicated(toplevel, testdir, **args)
    if type == "transient":
        return NonReplicated(toplevel, testdir, True, **args)
    return NonReplicated(toplevel, testdir, False, **args)

