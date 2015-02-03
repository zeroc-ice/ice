#!/usr/bin/env python
# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, subprocess, time, atexit, re

path = [ ".", "..", "../..", "../../..", "../../../.." ]
head = os.path.dirname(sys.argv[0])
if len(head) > 0:
    path = [os.path.join(head, p) for p in path]
path = [os.path.abspath(p) for p in path if os.path.exists(os.path.join(p, "scripts", "TestUtil.py")) ]

if len(path) == 0:
    raise RuntimeError("can't find toplevel directory!")
sys.path.append(os.path.join(path[0], "scripts"))
import TestUtil


configDir = os.path.join(TestUtil.getIceDir("cpp"), "config")

class ServiceController:

    def runCommand(self, command, expectedStatus = 0):

        p = subprocess.Popen(command,
                             shell = False, 
                             stdin = subprocess.PIPE, 
                             stdout = subprocess.PIPE,
                             stderr = subprocess.STDOUT,
                             env = self._env,
                             bufsize = 0)
        lines = []
        line = ""
        while(True):
            c = p.stdout.read(1)
            if not c:
                if p.wait() == expectedStatus:
                    break
                else:
                    print("failed! exit status %s" % p.wait())
                    sys.exit(1)

            if type(c) != str:
                c = c.decode()

            line += c
            if line == "Press any key to continue . . .":
                p.stdin.write("\n".encode("UTF-8"))

            if c == '\n':
                lines.append(line.rstrip())
		line = ""
        return "\n".join(lines)

    def installService(self, service):
        pass

    def startService(self, service):
        pass

    def stopService(self, service):
        pass

    def queryService(self, service):
        pass

    def uninstallService(self, service):
        pass

    def isRunning(self, service):
        pass

class WindowsServiceController(ServiceController):

    def __init__(self):
        self._iceserviceinstall = os.path.join(TestUtil.getIceDir("cpp"), "bin", "iceserviceinstall")
        self._notrunning = re.compile("STATE\s*:\s*1\s*STOPED")
        self._startPending = re.compile("STATE\s*:\s*2\s*START_PENDING")
        self._stopPending = re.compile("STATE\s*:\s*3\s*STOP_PENDING")
        self._running = re.compile("STATE\s*:\s*4\s*RUNNING")
        self._env = TestUtil.getTestEnv("cpp", os.path.abspath(os.getcwd()))

    def installService(self, service):
        sys.stdout.write("Installing %s service... " % service.name())
        sys.stdout.flush()
        config = os.path.join(configDir, ("%s.cfg" % service.name()))
        self.runCommand([self._iceserviceinstall, "-n", service.name(), config])
        print("ok")

    def startService(self, service):
        sys.stdout.write("Starting %s service... " % service.name())
        sys.stdout.flush()
        self.runCommand(["sc", "start", service.serviceName()])
        print("ok")

    def stopService(self, service):
        sys.stdout.write("Stoping %s service... " % service.name())
        sys.stdout.flush()
        self.runCommand(["sc", "stop", service.serviceName()])
        print("ok")

    def queryService(self, service):
        return self.runCommand(["sc", "query", service.serviceName()])

    def uninstallService(self, service, quiet = False):
        if not quiet:
            sys.stdout.write("Uninstalling %s service... " % service.name())
            sys.stdout.flush()
        
        config = os.path.join(configDir, ("%s.cfg" % service.name()))
        while(True):
            out = self.queryService(service)
            if self._startPending.search(out) or self._stopPending.search(out):
                time.sleep(1)
                continue
            out = self.runCommand([self._iceserviceinstall, "-n", "-u", service.name(), config])
            break
        if not quiet:
            print("ok")

    def isRunning(self, service):
        return self._running.search(self.queryService(service))
    
    def isStopped(self, service):
        return self._running.search(self.queryService(service))


class UbuntuServiceController(ServiceController):

    def __init__(self):
        self._running = re.compile("is running")
        self._notrunning = re.compile("is not running")
                
        self._env = TestUtil.getTestEnv("cpp", os.path.abspath(os.getcwd()))
        self._env["INIT_VERBOSE"] = "yes"
        
    def startService(self, service):
        sys.stdout.write("Starting %s service... " % service.name())
        sys.stdout.flush()
        
        pattern = re.compile("\* Starting %s\s.*[ OK ]" % service.name())
        if not pattern.search(self.runCommand(["/etc/init.d/%s" % service.name(), "start"])):
            print("failed!")
            sys.exit(1)
        print("ok")
        
    def queryService(self, service, expectedStatus = 0):
        return self.runCommand(["/etc/init.d/%s" % service.name(), "status"], expectedStatus)
        
    def stopService(self, service):
        sys.stdout.write("Stoping %s service... " % service.name())
        sys.stdout.flush()
        pattern = re.compile("\* Stopping %s\s.*[ OK ]" % service.name())
        if not pattern.search(self.runCommand(["/etc/init.d/%s" % service.name(), "stop"])):
            print("failed!")
            sys.exit(1)
        print("ok")
        
    
    def isRunning(self, service):
        return self._running.search(self.queryService(service))
    
    def isStopped(self, service):
        return self._notrunning.search(self.queryService(service, expectedStatus = 3))

class Rhel6ServiceController(ServiceController):

    def __init__(self):
        self._running = re.compile("is running")
        self._notrunning = re.compile("is stopped")
                
        self._env = TestUtil.getTestEnv("cpp", os.path.abspath(os.getcwd()))
        
    def startService(self, service):
        sys.stdout.write("Starting %s service... " % service.name())
        sys.stdout.flush()
        
        pattern = re.compile("Starting %s:\s*[  OK  ]" % service.name())
        if not pattern.search(self.runCommand(["/etc/init.d/%s" % service.name(), "start"])):
            print("failed!")
            sys.exit(1)
        print("ok")
        
    def queryService(self, service, expectedStatus = 0):
        return self.runCommand(["/etc/init.d/%s" % service.name(), "status"], expectedStatus)
        
    def stopService(self, service):
        sys.stdout.write("Stoping %s service... " % service.name())
        sys.stdout.flush()
        pattern = re.compile("Shutting down %s:\s*[  OK  ]" % service.name())
        if not pattern.search(self.runCommand(["/etc/init.d/%s" % service.name(), "stop"])):
            print("failed!")
            sys.exit(1)
        print("ok")
    
    def isRunning(self, service):
        return self._running.search(self.queryService(service))
    
    def isStopped(self, service):
        return self._notrunning.search(self.queryService(service, expectedStatus = 3))
    
class SystemdServiceController(ServiceController):

    def __init__(self):
        self._env = TestUtil.getTestEnv("cpp", os.path.abspath(os.getcwd()))
        
    def installService(self, service):
        sys.stdout.write("Enabling %s service... " % service.name())
        sys.stdout.flush()
        self.runCommand(["systemctl", "enable", service.name()])
        print("ok")
        
    def startService(self, service):
        sys.stdout.write("Starting %s service... " % service.name())
        sys.stdout.flush()
        self.runCommand(["systemctl", "start", service.name()])
        print("ok")
        
    def queryService(self, service, expectedStatus = 0):
        return self.runCommand(["systemctl", "is-active", service.name()], expectedStatus)
        
    def stopService(self, service):
        sys.stdout.write("Stoping %s service... " % service.name())
        sys.stdout.flush()
        self.runCommand(["systemctl", "stop", service.name()])
        print("ok")
    
    def uninstallService(self, service):
        sys.stdout.write("Disable %s service... " % service.name())
        sys.stdout.flush()
        self.runCommand(["systemctl", "disable", service.name()])
        print("ok")
        
    def isRunning(self, service, expectedStatus = 0):
        return self.queryService(service, expectedStatus) == "active"
    
    def isStopped(self, service):
        return not self.isRunning(service, 3)

class SLES11ServiceController(ServiceController):

    def __init__(self):
        self._env = TestUtil.getTestEnv("cpp", os.path.abspath(os.getcwd()))
        
    def startService(self, service):
        sys.stdout.write("Starting %s service... " % service.name())
        sys.stdout.flush()
        
        pattern = re.compile("Starting %s.*done" % service.name())
        if not pattern.search(self.runCommand(["/etc/init.d/%s" % service.name(), "start"])):
            print("failed!")
            sys.exit(1)
        print("ok")
        
    def queryService(self, service, expectedStatus = 0):
        return self.runCommand(["/etc/init.d/%s" % service.name(), "status"], expectedStatus)
        
    def stopService(self, service):
        sys.stdout.write("Stoping %s service... " % service.name())
        sys.stdout.flush()
        pattern = re.compile("Stopping %s.*done" % service.name())
        if not pattern.search(self.runCommand(["/etc/init.d/%s" % service.name(), "stop"])):
            print("failed!")
            sys.exit(1)
        print("ok")
    
    def isRunning(self, service):
	pattern = re.compile("Checking for service %s.*running" % service.name())
        return pattern.search(self.queryService(service))
    
    def isStopped(self, service):
        pattern = re.compile("Checking for service %s.*unused" % service.name())
        return pattern.search(self.queryService(service, expectedStatus = 3))

class SystemService:

    def __init__(self, name, id):
        self._name = name
        self._id = id

    def name(self):
        return self._name

    def id(self):
        return self._id

    def serviceName(self):
        return "%s.%s" % (self._name, self._id)

    
def run(platform):

    services = [
        SystemService("glacier2router", "DemoGlacier2Router"),
        SystemService("icegridnode", "DemoIceGrid.node1"),
        SystemService("icegridregistry", "DemoIceGrid")
    ]

    if platform.isWindows():
        controller = WindowsServiceController()
    elif platform.isLinux():
        if platform.isUbuntu():
            controller = UbuntuServiceController()
        elif platform.isRhel(7):
            controller = SystemdServiceController()
        elif platform.isRhel():
            controller = Rhel6ServiceController()
	elif platform.isSles(12):
            controller = SystemdServiceController()
	elif platform.isSles():
	    controller = SLES11ServiceController()
    else:
        print("Service testing not supported with platform: %s" % sys.platform)
        return

    def cleanup():
        for service in services:
            try:
                controller.uninstallService(service)
            except:
                pass
    atexit.register(cleanup)
    
    for service in services:
        controller.installService(service)
        controller.startService(service)

        sys.stdout.write("Waiting for service to start...")
        sys.stdout.flush()
        while(not controller.isRunning(service)):
            #
            # Wait for the service to be ready
            #
            sys.stdout.write(".")
            sys.stdout.flush()
            time.sleep(1)
        print(" ok")
        controller.stopService(service)
        
        sys.stdout.write("Waiting for service to stop...")
        sys.stdout.flush()
        while(not controller.isStopped(service)):
            #
            # Wait for the service to be ready
            #
            sys.stdout.write(".")
            sys.stdout.flush()
            time.sleep(1)
        print(" ok")
