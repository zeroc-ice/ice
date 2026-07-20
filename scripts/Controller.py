#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import os
import subprocess
import sys
import uuid

from Util import (
    Darwin,
    Driver,
    IceProcess,
    Mapping,
    Result,
    parseOptions,
    platform,
    runTests,
    toplevel,
    traceback,
)


class ControllerDriver(Driver):
    class Current(Driver.Current):
        def __init__(self, driver, testsuite, testcase, cross, protocol, host, args):
            Driver.Current.__init__(self, driver, testsuite, Result(testsuite, driver.debug))
            self.testcase = testcase
            self.serverTestCase = self.testcase.getServerTestCase(cross)
            self.clientTestCase = self.testcase.getClientTestCase()
            self.cross = cross
            self.host = host
            self.args = args
            self.config.protocol = protocol

    @classmethod
    def getSupportedArgs(self):
        return (
            "",
            [
                "clean",
                "id=",
                "endpoints=",
                "uuid=",
                "bt-setup=",
                "bt-bond=",
                "bt-diagnostics",
                "bt-prepare",
                "bt-emulators",
                "bt-client=",
                "bt-server=",
                "bt-image=",
            ],
        )

    @classmethod
    def usage(self):
        print("")
        print("Controller driver options:")
        print("--id=<identity>       The identify of the controller object.")
        print("--endpoints=<endpts>  The endpoints to listen on.")
        print("--clean               Remove trust settings (macOS).")
        print("")
        print("Bluetooth harness options (all exit without starting a controller):")
        print("--bt-emulators        Create and boot the --bt-client/--bt-server emulators (--bt-image).")
        print("--bt-prepare          Prepare both emulators and bond them; prints the server address.")
        print("--bt-setup=<apk>      Prepare --device only (boot-wait, install the btbond privileged")
        print("                      helper, enable Bluetooth, grant permissions).")
        print("--bt-bond=<serial>    Bond --device to the given peer emulator over RFCOMM.")
        print("--bt-diagnostics      Dump adb state (controller pid, forwards, logcat) for --device.")
        print("--bt-client=<serial>  Emulator running the IceBT client (e.g. emulator-5554).")
        print("--bt-server=<serial>  Emulator running the IceBT server (e.g. emulator-5556).")
        print("--bt-image=<sdk>      System image used to create the AVDs for --bt-emulators.")
        print("--uuid=<uuid>         RFCOMM service UUID used when bonding.")

    def __init__(self, options, *args, **kargs):
        Driver.__init__(self, options, *args, **kargs)
        self.id = "controller"
        self.endpoints = ""
        self.clean = False
        self.btSetup = ""  # path to the btbond APK; when set, run Bluetooth device setup and exit
        self.btBond = ""  # peer emulator serial; when set, bond --device to it and exit
        self.uuid = ""  # RFCOMM service UUID used by the btbond bond
        self.btDiagnostics = False  # when set, dump adb diagnostics for --device and exit
        self.btPrepare = False  # when set, prepare + bond --bt-client/--bt-server and exit
        self.btEmulators = False  # when set, create + boot the two emulators and exit
        self.btClient = ""  # emulator serial running the IceBT client
        self.btServer = ""  # emulator serial running the IceBT server
        self.btImage = ""  # system image used to create the AVDs
        # NB: don't add a self.device here -- parseOptions consumes the options it recognizes, so a
        # self.device would swallow --device before the per-mapping Config can read it. The per-device
        # Bluetooth modes read the emulator serial from the config instead (see runBluetoothDevice).
        parseOptions(
            self,
            options,
            {
                "clean": "clean",
                "bt-setup": "btSetup",
                "bt-bond": "btBond",
                "bt-diagnostics": "btDiagnostics",
                "bt-prepare": "btPrepare",
                "bt-emulators": "btEmulators",
                "bt-client": "btClient",
                "bt-server": "btServer",
                "bt-image": "btImage",
            },
        )

        if not self.endpoints:
            self.endpoints = ("tcp -h " + self.interface) if self.interface else "tcp"

    def run(self, mappings, testSuiteIds):
        if self.btEmulators:
            return self.runBluetoothEmulators()
        if self.btPrepare:
            return self.runBluetoothPrepare()
        if self.btSetup or self.btBond or self.btDiagnostics:
            return self.runBluetoothDevice()

        if isinstance(platform, Darwin):
            #
            # On macOS, we set the trust settings on the certificate to prevent
            # the Web browsers from prompting the user about the untrusted
            # certificate. Some browsers such as Chrome don't provide the
            # option to set this trust settings.
            #
            serverCert = os.path.join(toplevel, "certs", "server_cert.pem")
            if self.clean:
                if os.system("security verify-cert -c " + serverCert + " >& /dev/null") == 0:
                    sys.stdout.write("removing trust settings for the HTTP server certificate... ")
                    sys.stdout.flush()
                    if os.system("security remove-trusted-cert " + serverCert) != 0:
                        print("\nerror: couldn't remove trust settings for the HTTP server certificate")
                    else:
                        print("ok")
                else:
                    print("trust settings already removed")
                return
            else:
                if os.system("security verify-cert -c " + serverCert + " >& /dev/null") != 0:
                    sys.stdout.write("adding trust settings for the HTTP server certificate... ")
                    sys.stdout.flush()
                    if os.system("security add-trusted-cert -r trustAsRoot " + serverCert) != 0:
                        print("error: couldn't add trust settings for the HTTP server certificate")
                    print("ok")
                    print("run " + sys.argv[0] + " --clean to remove the trust setting")

        self.initCommunicator()
        import Ice

        Ice.loadSlice([os.path.join(toplevel, "scripts", "Controller.ice")])

        from Test import Common as Test_Common

        class TestCaseI(Test_Common.TestCase):
            def __init__(self, driver, current):
                self.driver = driver
                self.current = current
                self.serverSideRunning = False

            def startServerSide(self, config, c):
                self.updateCurrent(config)
                try:
                    self.serverSideRunning = True
                    return self.current.serverTestCase._startServerSide(self.current)
                except Exception:
                    self.serverSideRunning = False
                    raise Test_Common.TestCaseFailedException(
                        self.current.result.getOutput() + "\n" + traceback.format_exc()
                    )

            def stopServerSide(self, success, c):
                if self.serverSideRunning:
                    try:
                        self.current.serverTestCase._stopServerSide(self.current, success)
                        return self.current.result.getOutput()
                    except Exception as ex:
                        raise Test_Common.TestCaseFailedException(self.current.result.getOutput() + "\n" + str(ex))

            def runClientSide(self, host, config, c):
                self.updateCurrent(config)
                try:
                    self.current.clientTestCase._runClientSide(self.current, host)
                    return self.current.result.getOutput()
                except Exception as ex:
                    raise Test_Common.TestCaseFailedException(self.current.result.getOutput() + "\n" + str(ex))

            def destroy(self, c):
                if self.serverSideRunning:
                    try:
                        self.current.serverTestCase._stopServerSide(self.current, False)
                    except Exception:
                        pass
                c.adapter.remove(c.id)

            def updateCurrent(self, config):
                attrs = [
                    "protocol",
                    "mx",
                    "serialize",
                    "compress",
                    "ipv6",
                    "cprops",
                    "sprops",
                ]
                for a in attrs:
                    v = getattr(config, a)
                    if v is not None:
                        if a not in self.current.config.parsedOptions:
                            self.current.config.parsedOptions.append(a)
                        setattr(self.current.config, a, v)

        class ControllerI(Test_Common.Controller):
            def __init__(self, driver):
                self.driver = driver
                self.testcase = None

            def runTestCase(self, mapping, testsuite, testcase, cross, c):
                if self.testcase:
                    try:
                        self.testcase.destroy()
                    except Exception:
                        pass
                    self.testcase = None

                current = self.driver.getCurrent(mapping, testsuite, testcase, cross)
                prx = c.adapter.addWithUUID(TestCaseI(self.driver, current))
                self.testcase = Test_Common.TestCasePrx.uncheckedCast(
                    c.adapter.createDirectProxy(prx.ice_getIdentity())
                )
                return self.testcase

            def getTestSuites(self, mapping, c):
                mapping = Mapping.getByName(mapping)
                config = self.driver.configs[mapping]
                return [str(t) for t in mapping.getTestSuites() if not mapping.filterTestSuite(t.getId(), config)]

            def getOptionOverrides(self, c):
                return Test_Common.OptionOverrides(ipv6=([False] if not self.driver.hostIPv6 else [False, True]))

            def getHost(self, name, ipv6, c):
                pass

        import Ice

        self.initCommunicator()
        self.communicator.getProperties().setProperty("ControllerAdapter.Endpoints", self.endpoints)
        self.communicator.getProperties().setProperty("ControllerAdapter.AdapterId", str(uuid.uuid4()))
        adapter = self.communicator.createObjectAdapter("ControllerAdapter")
        adapter.add(ControllerI(self), Ice.stringToIdentity(self.id))
        adapter.activate()
        self.communicator.waitForShutdown()

    @staticmethod
    def emulatorPort(serial):
        # "emulator-5554" -> 5554
        try:
            return int(serial.rsplit("-", 1)[1])
        except (IndexError, ValueError):
            raise RuntimeError(f"expected an emulator serial like 'emulator-5554', got '{serial}'")

    def runBluetoothEmulators(self):
        # Create and boot the two emulators used by the Bluetooth harness. They are launched detached
        # so they outlive this process.
        from Util import AndroidProcessController

        if not (self.btClient and self.btServer and self.btImage):
            raise RuntimeError("--bt-emulators requires --bt-client, --bt-server and --bt-image")
        for role, serial in (("client", self.btClient), ("server", self.btServer)):
            AndroidProcessController.createBluetoothEmulator(
                f"bt_{role}", self.btImage, self.emulatorPort(serial), f"emu_{role}.log"
            )
        return 0

    def runBluetoothPrepare(self):
        # Prepare both emulators (in parallel, each with its own log) and bond the client to the
        # server. Progress goes to stderr and only the server's Bluetooth address to stdout, so
        # callers can capture it directly.
        from Util import AndroidProcessController

        if not (self.btClient and self.btServer and self.btSetup and self.uuid):
            raise RuntimeError("--bt-prepare requires --bt-client, --bt-server, --bt-setup=<apk> and --uuid")

        # Re-invoke this script per device so each emulator's setup keeps a separate log.
        running = {}
        for role, serial in (("client", self.btClient), ("server", self.btServer)):
            log = open(f"setup_{role}.log", "wb")
            running[role] = (
                subprocess.Popen(
                    [sys.executable, __file__, "--android", f"--device={serial}", f"--bt-setup={self.btSetup}"],
                    stdout=log,
                    stderr=subprocess.STDOUT,
                ),
                log,
            )
        failed = []
        for role, (process, log) in running.items():
            status = process.wait()
            log.close()
            with open(f"setup_{role}.log") as f:
                print(f"==== {role} setup ====\n{f.read()}", file=sys.stderr)
            if status != 0:
                failed.append(role)
        if failed:
            raise RuntimeError(f"Bluetooth setup failed for: {', '.join(failed)}")

        AndroidProcessController.forDevice(self.btClient).bond(self.btServer, self.uuid)
        print(f"bonded {self.btClient} to {self.btServer}", file=sys.stderr)
        print(AndroidProcessController.forDevice(self.btServer).bluetoothAddress())
        return 0

    def runBluetoothDevice(self):
        # adb-driven Bluetooth setup/diagnostics for one Android emulator, invoked by the Bluetooth CI
        # harness instead of inline `adb` shell. It reuses the adb helpers already on
        # AndroidProcessController (self.adb(), waitForBoot, install, etc.) rather than reimplementing
        # them. No Ice communicator is needed, so this returns before the controller is started.
        from Util import AndroidProcessController

        device = next((c.device for c in self.configs.values() if c.device), "")
        if not device:
            raise RuntimeError("--bt-setup/--bt-bond/--bt-diagnostics require --device=<emulator serial>")

        controller = AndroidProcessController.forDevice(device)

        if self.btSetup:
            controller.waitForBoot()
            controller.installSystemApp(
                self.btSetup, "btbond", "com.zeroc.btbond", ["android.permission.BLUETOOTH_PRIVILEGED"]
            )
            controller.enableBluetooth()
            controller.grantRuntimePermissions(
                "com.zeroc.btbond",
                ["android.permission.BLUETOOTH_CONNECT", "android.permission.BLUETOOTH_SCAN"],
            )
            print(f"BT_ADDRESS={controller.bluetoothAddress()}")

        if self.btBond:
            if not self.uuid:
                raise RuntimeError("--bt-bond requires --uuid=<uuid>")
            controller.bond(self.btBond, self.uuid)
            print(f"bonded {device} to {self.btBond}")

        if self.btDiagnostics:
            controller.diagnostics()

        return 0

    def getCurrent(self, mapping, testsuite, testcase, cross, protocol=None, host=None, args=[]):
        from Test import Common as Test_Common

        mapping = Mapping.getByName(mapping)
        if not mapping:
            raise Test_Common.TestCaseNotExistException("unknown mapping {0}".format(mapping))

        if cross:
            cross = Mapping.getByName(cross)
            if not cross:
                raise Test_Common.TestCaseNotExistException("unknown mapping {0} for cross testing".format(cross))

        ts = mapping.findTestSuite(testsuite)
        if not ts:
            raise Test_Common.TestCaseNotExistException("unknown testsuite {0}".format(testsuite))

        tc = ts.findTestCase("server" if ts.getId() == "Ice/echo" else (testcase or "client/server"))
        if not tc or not tc.getServerTestCase():
            raise Test_Common.TestCaseNotExistException("unknown testcase {0}".format(testcase))

        return ControllerDriver.Current(self, ts, tc, cross, protocol, host, args)

    def getProps(self, process, current):
        props = Driver.getProps(self, process, current)
        if isinstance(process, IceProcess) and current.host:
            props["Ice.Default.Host"] = current.host
        return props

    def getArgs(self, process, current):
        return current.args

    def isWorkerThread(self):
        return True

    def isInterrupted(self):
        return False


Driver.add("controller", ControllerDriver, default=True)

runTests()
