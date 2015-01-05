#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, threading, subprocess, getopt

path = [ ".", "..", "../..", "../../..", "../../../.." ]
head = os.path.dirname(sys.argv[0])
if len(head) > 0:
    path = [os.path.join(head, p) for p in path]
path = [os.path.abspath(p) for p in path if os.path.exists(os.path.join(p, "scripts", "TestUtil.py")) ]
if len(path) == 0:
    raise RuntimeError("can't find toplevel directory!")
sys.path.append(os.path.join(path[0], "scripts"))

#
# On OS X, provide an option to allow removing the trust settings
#
if sys.platform == "darwin":
    try:
        opts, args = getopt.getopt(sys.argv[1:], "", ["clean"])
        if ("--clean", "") in opts:
            serverCert = os.path.join(path[0], "certs", "s_rsa1024_pub.pem")
            if os.system("security verify-cert -c " + serverCert + " >& /dev/null") == 0:
                sys.stdout.write("removing trust settings for the HTTP server certificate... ")
                sys.stdout.flush()
                if os.system("security remove-trusted-cert " + serverCert) != 0:
                    print("error: couldn't remove trust settings for the HTTP server certificate")
                print("ok")
            else:
                print("trust settings already removed")
            sys.exit(0)
    except getopt.GetoptError:
        pass

#
# Run the script with the environment correctly set.
#
if os.environ.get("RUNNING_TEST_CONTROLLER_WITH_ENV", "") == "":
    os.environ["TESTCONTROLLER"] = "yes" # Trick to prevent TestUtil.py from printing out diagnostic information
    import TestUtil
    del os.environ["TESTCONTROLLER"]

    #
    # Get environment to run Ice for Python script
    #
    env = TestUtil.getTestEnv("py", os.getcwd())
    env["RUNNING_TEST_CONTROLLER_WITH_ENV"] = "yes"
    try:

        #
        # The HttpServer relies on the ICE_HOME environment variable
        # to find the Ice for JavaScript installation so we make sure
        # here that ICE_HOME is set if we are running against a binary
        # distribution (i.e.: TestUtil.iceHome is not None)
        #
        if TestUtil.iceHome:
            env["ICE_HOME"] = TestUtil.iceHome

        if TestUtil.printenv:
            TestUtil.dumpenv(env, "py")
        args = [sys.executable, "run.py"]
        for a in sys.argv[1:]:
            args.append(a)
        sys.exit(os.spawnve(os.P_WAIT, sys.executable, args, env))
    except KeyboardInterrupt:
        sys.exit(0)
    finally:
        if sys.platform == "darwin":
            print("")
            print("To remove the certificate trust settings, run: `" + sys.argv[0] + " --clean'")
    sys.exit(1)

import TestUtil, Ice, Expect
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
        # Stop the WatchDog thread since we may not run any more tests
        # for an extended period of time
        TestUtil.stopWatchDog()
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
        # Stop the WatchDog thread since we may not run any more tests
        # for an extended period of time
        TestUtil.stopWatchDog()
        print("ok")

class ControllerI(Test.Controller):
    def __init__(self):
        self.currentServer = None

    def runServer(self, lang, name, protocol, host, options, current):

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
            testDir = 'test' if lang != 'java' else 'test/src/main/java/test'
            serverdir = os.path.join(TestUtil.toplevel, lang, testDir, name)
            os.chdir(serverdir)
            serverDesc = os.path.join(lang, 'test', name)
            lang = TestUtil.getDefaultMapping()
            server = TestUtil.getDefaultServerFile()
            if lang != "java":
                server = os.path.join(serverdir, server)
            serverenv = TestUtil.getTestEnv(lang, serverdir)

            sys.stdout.write("starting " + serverDesc + "... ")
            sys.stdout.flush()
            serverCfg = TestUtil.DriverConfig("server")
            serverCfg.protocol = protocol
            serverCfg.host = host
            server = TestUtil.getCommandLine(server, serverCfg, options)
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
        nodeCmd = TestUtil.getNodeCommand()
        httpServer = subprocess.Popen(nodeCmd + " \"" + os.path.join(jsDir, "bin", "HttpServer.js") + "\"", 
                                      shell = True, 
                                      stdin = subprocess.PIPE, 
                                      stdout = subprocess.PIPE, 
                                      stderr = None,
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
            line = line.strip("\n")
            if len(line) > 0:
                print(line)
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
initData.properties.setProperty("IceSSL.Keychain", "test.keychain");
initData.properties.setProperty("IceSSL.KeychainPassword", "password");
initData.properties.setProperty("IceSSL.VerifyPeer", "0");
initData.properties.setProperty("Ice.ThreadPool.Server.SizeMax", "10")
initData.properties.setProperty("ControllerAdapter.Endpoints", "ws -p 12009:wss -p 12008")

if TestUtil.isDarwin():
    #
    # On OS X, we set the trust settings on the certificate to prevent
    # the Web browsers from prompting the user about the unstrusted
    # certificate. Some browsers such as Chrome don't provide the
    # option to set this trust settings.
    #
    serverCert = os.path.join(TestUtil.toplevel, "certs", "s_rsa1024_pub.pem")
    if os.system("security verify-cert -c " + serverCert + " >& /dev/null") != 0:
        sys.stdout.write("adding trust settings for the HTTP server certificate... ")
        sys.stdout.flush()
        if os.system("security add-trusted-cert -r trustAsRoot " + serverCert) != 0:
            print("error: couldn't add trust settings for the HTTP server certificate")
        print("ok")

sys.exit(app.main(sys.argv, initData=initData))
