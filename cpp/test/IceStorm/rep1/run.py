#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, time, re

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.append(os.path.join(toplevel, "config"))
import TestUtil
TestUtil.processCmdLine()

name = os.path.join("IceStorm", "rep1")
testdir = os.path.dirname(os.path.abspath(__file__))
publisher = os.path.join(testdir, "publisher")
subscriber = os.path.join(testdir, "subscriber")
subscriber2 = os.path.join(testdir, "sub")

def printOutput(pipe):
    try:
        while True:
            line = pipe.readline()
            if not line:
                break
            print line,
            sys.stdout.flush()
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

def runsub(opt, ref, arg = "", echo=False):
    qos = ""
    if opt == "twoway":
        qos = " --twoway"
    if opt == "ordered":
        qos = " --ordered"
    pipe = TestUtil.startServer(subscriber, ref + arg + qos)
    TestUtil.getServerPid(pipe)
    TestUtil.getAdapterReady(pipe, True)
    return pipe

def runpub(ref, arg = "", echo=False):
    return TestUtil.startClient(publisher, ref + arg)

def runtest(opt, ref, subopt="", pubopt=""):
    subscriberPipe = runsub(opt, ref, subopt)
    publisherPipe = runpub(ref, pubopt)
    printOutput(publisherPipe)
    publisherStatus = TestUtil.closePipe(publisherPipe)
    subscriberStatus = TestUtil.specificServerStatus(subscriberPipe, 30)
    if subscriberStatus or publisherStatus:
        print "FAILED!"
        while True:
            import time
            time.sleep(1000)
        TestUtil.killServers()
        sys.exit(1)

def runsub2(replica = -1, cmd = "", terminateOnError=True):
    pipe = TestUtil.startServer(subscriber2, icestorm.reference(replica) + ' --id foo' + cmd)
    TestUtil.getServerPid(pipe)
    #TestUtil.getAdapterReady(pipe, True)
    out = captureOutput(pipe)
    status = TestUtil.closePipe(pipe)
    if terminateOnError and status:
        print "status: %d out: '%s'" % (status, out)
        TestUtil.killServers()
        sys.exit(1)
    return status, out.strip()

def rununsub2(replica = -1, terminateOnError=True):
    if replica == -1:
        runsub2(replica, " --unsub", terminateOnError)
    # Else we first subscribe to this replica, then unsub. We
    # shouldn't get an AlreadySubscribedException.
    status, out = runsub2(replica, terminateOnError=terminateOnError)
    if status:
        return status, out
    return runsub2(replica, " --unsub", terminateOnError=terminateOnError)

import IceStormUtil

icestorm = IceStormUtil.init(toplevel, testdir, "replicated", replicatedPublisher=True, additional =
                             ' --IceStorm.Election.MasterTimeout=2' +
                             ' --IceStorm.Election.ElectionTimeout=2' +
                             ' --IceStorm.Election.ResponseTimeout=2')
icestorm.start()

print "testing topic creation across replicas...",
sys.stdout.flush()
icestorm.admin("create single")

for replica in range(0, 3):
    status, out = icestorm.adminForReplica(replica, "create single", terminateOnError=False)
    if out != "error: topic `single' exists":
        TestUtil.killServers()
        sys.exit(1)
print "ok"

print "testing topic destruction across replicas...",
sys.stdout.flush()
icestorm.admin("destroy single")

for replica in range(0, 3):
    status, out = icestorm.adminForReplica(replica, "destroy single", terminateOnError=False)
    if out != "error: couldn't find topic `single'":
        TestUtil.killServers()
        sys.exit(1)
print "ok"

print "testing topic creation without replica...",
sys.stdout.flush()

icestorm.stopReplica(0)
icestorm.admin("create single")

for replica in range(1, 3):
    status, out = icestorm.adminForReplica(replica, "create single", terminateOnError=False)
    if out != "error: topic `single' exists":
        TestUtil.killServers()
        sys.exit(1)

status, out = icestorm.adminForReplica(0, "create single", terminateOnError=False)
if not re.search("ConnectionRefused", out):
    TestUtil.killServers()
    sys.exit(1)

icestorm.startReplica(0, echo=False)

status, out = icestorm.adminForReplica(0, "create single", terminateOnError=False)
if out != "error: topic `single' exists":
    TestUtil.killServers()
    sys.exit(1)
print "ok"

icestorm.admin("destroy single")

print "testing topic creation without master...",
sys.stdout.flush()
icestorm.stopReplica(2)

icestorm.admin("create single")

for replica in range(0, 2):
    status, out = icestorm.adminForReplica(replica, "create single", terminateOnError=False)
    if out != "error: topic `single' exists":
        TestUtil.killServers()
        sys.exit(1)

status, out = icestorm.adminForReplica(2, "create single", terminateOnError=False)
if not re.search("ConnectionRefused", out):
    TestUtil.killServers()
    sys.exit(1)

icestorm.startReplica(2, echo=False)

status, out = icestorm.adminForReplica(2, "create single", terminateOnError=False)
if out != "error: topic `single' exists":
    TestUtil.killServers()
    sys.exit(1)
print "ok"

# All replicas are running

print "testing topic destruction without replica...",
sys.stdout.flush()
icestorm.stopReplica(0)

icestorm.admin("destroy single")

for replica in range(1, 3):
    status, out = icestorm.adminForReplica(replica, "destroy single", terminateOnError=False)
    if out != "error: couldn't find topic `single'":
        TestUtil.killServers()
        sys.exit(1)

status, out = icestorm.adminForReplica(0, "destroy single", terminateOnError=False)
if not re.search("ConnectionRefused", out):
    TestUtil.killServers()
    sys.exit(1)

icestorm.startReplica(0, echo=False)

status, out = icestorm.adminForReplica(0, "destroy single", terminateOnError=False)
if out != "error: couldn't find topic `single'":
    print out
    TestUtil.killServers()
    sys.exit(1)
print "ok"

print "testing topic destruction without master...",
sys.stdout.flush()

icestorm.admin("create single")
icestorm.stopReplica(2)

icestorm.admin("destroy single")

for replica in range(0, 2):
    status, out = icestorm.adminForReplica(replica, "destroy single", terminateOnError=False)
    if out != "error: couldn't find topic `single'":
        TestUtil.killServers()
        sys.exit(1)

status, out = icestorm.adminForReplica(2, "destroy single", terminateOnError=False)
if not re.search("ConnectionRefused", out):
    TestUtil.killServers()
    sys.exit(1)

icestorm.startReplica(2, echo=False)

status, out = icestorm.adminForReplica(2, "destroy single", terminateOnError=False)
if out != "error: couldn't find topic `single'":
    TestUtil.killServers()
    sys.exit(1)
print "ok"

# Now test subscription/unsubscription on all replicas.

icestorm.admin("create single")

print "testing subscription across replicas...",
sys.stdout.flush()
runsub2()

for replica in range(0, 3):
    status, out = runsub2(replica, terminateOnError=False)
    if out != "IceStorm::AlreadySubscribed":
        print out
        TestUtil.killServers()
        sys.exit(1)
print "ok"

print "testing unsubscription across replicas...",
sys.stdout.flush()
rununsub2()

for replica in range(0, 3):
    rununsub2(replica, terminateOnError=False)
print "ok"

print "testing subscription without master...",
sys.stdout.flush()
icestorm.stopReplica(2)

runsub2()

for replica in range(0, 2):
    status, out = runsub2(replica, terminateOnError=False)
    if out != "IceStorm::AlreadySubscribed":
        TestUtil.killServers()
        sys.exit(1)

status, out = runsub2(2, terminateOnError=False)
if not re.search("ConnectionRefused", out):
    TestUtil.killServers()
    sys.exit(1)

icestorm.startReplica(2, echo=False)

status, out = runsub2(2, terminateOnError=False)
if out != "IceStorm::AlreadySubscribed":
    TestUtil.killServers()
    sys.exit(1)
print "ok"

print "testing unsubscription without master...",
sys.stdout.flush()
icestorm.stopReplica(2)

rununsub2()

for replica in range(0, 2):
    rununsub2(replica, terminateOnError=False)

status, out = rununsub2(2, terminateOnError=False)
if not re.search("ConnectionRefused", out):
    TestUtil.killServers()
    sys.exit(1)

icestorm.startReplica(2, echo=False)

rununsub2(2)
print "ok"

print "testing subscription without replica...",
sys.stdout.flush()
icestorm.stopReplica(0)

runsub2()

for replica in range(1, 3):
    status, out = runsub2(replica, terminateOnError=False)
    if out != "IceStorm::AlreadySubscribed":
        TestUtil.killServers()
        sys.exit(1)

status, out = runsub2(0, terminateOnError=False)
if not re.search("ConnectionRefused", out):
    TestUtil.killServers()
    sys.exit(1)

icestorm.startReplica(0, echo=False)

status, out = runsub2(0, terminateOnError=False)
if out != "IceStorm::AlreadySubscribed":
    TestUtil.killServers()
    sys.exit(1)
print "ok"

print "testing unsubscription without replica...",
sys.stdout.flush()
icestorm.stopReplica(0)

rununsub2()

for replica in range(1, 3):
    rununsub2(replica, terminateOnError=False)

status, out = rununsub2(0, terminateOnError=False)
if not re.search("ConnectionRefused", out):
    TestUtil.killServers()
    sys.exit(1)

icestorm.startReplica(0, echo=False)

rununsub2(0)
print "ok"

# All replicas are running

print "running twoway subscription test...",
sys.stdout.flush()
runtest("twoway", icestorm.reference())
print "ok"

print "running ordered subscription test...",
sys.stdout.flush()
runtest("ordered", icestorm.reference())
print "ok"

icestorm.stopReplica(2)

print "running twoway, ordered subscription test without master...",
sys.stdout.flush()
runtest("twoway", icestorm.reference())
runtest("ordered", icestorm.reference())
print "ok"

icestorm.startReplica(2, echo = False)
icestorm.stopReplica(0)

print "running twoway, ordered subscription test without replica...",
sys.stdout.flush()
runtest("twoway", icestorm.reference())
runtest("ordered", icestorm.reference())
print "ok"

icestorm.startReplica(0, echo = False)

print "running cycle publishing test...",
sys.stdout.flush()
runtest("twoway", icestorm.reference(), pubopt=" --cycle")
print "ok"

print "stopping replicas...",
sys.stdout.flush()
icestorm.stop()
print "ok"

if TestUtil.serverStatus():
    TestUtil.killServers()
    sys.exit(1)

sys.exit(0)
