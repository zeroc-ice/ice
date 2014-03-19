#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, threading, subprocess

path = [ ".", "..", "../..", "../../..", "../../../.." ]
head = os.path.dirname(sys.argv[0])
if len(head) > 0:
    path = [os.path.join(head, p) for p in path]
path = [os.path.abspath(p) for p in path if os.path.exists(os.path.join(p, "scripts", "TestUtil.py")) ]
if len(path) == 0:
    raise RuntimeError("can't find toplevel directory!")
sys.path.append(os.path.join(path[0], "scripts"))
import TestUtil

#
# Run the script with the environment correctly set.
#
if os.environ.get("RUNNING_TEST_CONTROLLER_WITH_ENV", "") == "":
    env = TestUtil.getTestEnv("cpp", os.getcwd())
    env["RUNNING_TEST_CONTROLLER_WITH_ENV"] = "yes"
    sys.exit(os.spawnve(os.P_WAIT, sys.executable, [sys.executable, "run.py"], env))

import Ice, Expect
Ice.loadSlice("\"" + os.path.join(TestUtil.toplevel, "js", "test", "Common", "Controller.ice") + "\"")
import Test

class ServerI(Test.Server):
    def __init__(self, name, process):
        self.name = name
        self.process = process

    def waitTestSuccess(self, current):
        if not self.process:
            return
        sys.stdout.write("waiting for " + self.name + " to terminate... ")
        sys.stdout.flush()
        self.process.waitTestSuccess()
        self.process = None
        try:
            current.adapter.remove(current.id)
        except:
            pass
        print("ok")

    def terminate(self, current):
        if not self.process:
            return
        sys.stdout.write("terminating " + self.name + "... ")
        sys.stdout.flush()
        self.process.terminate()
        self.process = None
        try:
            current.adapter.remove(current.id)
        except:
            pass
        print("ok")

class ControllerI(Test.Controller):
    def __init__(self):
        self.currentServer = None

    def runServer(self, lang, name, protocol, host, current):

        # If server is still running, terminate it
        if self.currentServer:
            try:
                self.currentServer.terminate()
            except:
                pass
            self.currentServer = None

        pwd = os.getcwd()
        try:
            # Change current directory to the test directory
            serverdir = os.path.join(TestUtil.toplevel, lang, "test", name)
            os.chdir(serverdir)

            serverDesc = os.path.join(lang, "test", name)
            lang = TestUtil.getDefaultMapping()
            server = os.path.join(serverdir, TestUtil.getDefaultServerFile())
            serverenv = TestUtil.getTestEnv(lang, serverdir)
            
            sys.stdout.write("starting " + serverDesc + "... ")
            sys.stdout.flush()
            serverCfg = TestUtil.DriverConfig("server")
            serverCfg.protocol = protocol
            serverCfg.host = host
            server = TestUtil.getCommandLine(server, serverCfg)
            serverProc = TestUtil.spawnServer(server, env = serverenv, lang=serverCfg.lang, mx=serverCfg.mx)
            print("ok")
        finally:
            os.chdir(pwd)
        
        self.currentServer = Test.ServerPrx.uncheckedCast(current.adapter.addWithUUID(ServerI(serverDesc, serverProc)))
        return self.currentServer

class Reader(threading.Thread):
    def __init__(self, p):
        self.p = p
        threading.Thread.__init__(self)

    def run(self):
        while True:
            line = self.p.stdout.readline()
            if self.p.poll() is not None and not line:
                #process terminated
                return self.p.poll()
            if type(line) != str:
                line = line.decode()
            sys.stdout.write(line)
            sys.stdout.flush()
            
class Server(Ice.Application):
    def run(self, args):
        jsDir = os.path.join(TestUtil.toplevel, "js")
        httpServer = subprocess.Popen("node \"" + os.path.join(jsDir, "bin", "HttpServer.js") + "\"", shell = True,
                                      stdin = subprocess.PIPE, stdout = subprocess.PIPE, stderr = subprocess.STDOUT, 
                                      bufsize = 0)
        #
        # Wait for the HttpServer to start
        #
        while True:
            line = httpServer.stdout.readline()
            if httpServer.poll() is not None and not line:
                #process terminated
                return httpServer.poll()
            if type(line) != str:
                line = line.decode()
            sys.stdout.write(line)
            sys.stdout.flush()
            if line.find("listening on ports 8080 (http) and 9090 (https)...") != -1:
                break

        reader = Reader(httpServer)
        reader.start()
        
        adapter = self.communicator().createObjectAdapter("ControllerAdapter")
        adapter.add(ControllerI(), self.communicator().stringToIdentity("controller"))
        adapter.activate()
        self.communicator().waitForShutdown()

        if httpServer.poll() is None:
            httpServer.terminate()
        
        reader.join()
        return 0
        
app = Server()
initData = Ice.InitializationData()
initData.properties = Ice.createProperties();
initData.properties.setProperty("Ice.Plugin.IceSSL", "IceSSL:createIceSSL")
initData.properties.setProperty("IceSSL.DefaultDir", os.path.join(TestUtil.toplevel, "certs"));
initData.properties.setProperty("IceSSL.CertAuthFile", "cacert.pem");
initData.properties.setProperty("IceSSL.CertFile", "s_rsa1024_pub.pem");
initData.properties.setProperty("IceSSL.KeyFile", "s_rsa1024_priv.pem");
initData.properties.setProperty("IceSSL.VerifyPeer", "0");
initData.properties.setProperty("Ice.Plugin.IceWS", "IceWS:createIceWS")
initData.properties.setProperty("Ice.ThreadPool.Server.SizeMax", "10")
initData.properties.setProperty("ControllerAdapter.Endpoints", "ws -p 12009:wss -p 12008")
sys.exit(app.main(sys.argv, initData=initData))
