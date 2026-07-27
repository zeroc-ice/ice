#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

from __future__ import annotations

import os
import subprocess
import sys
import uuid
from typing import IO, Any

from Util import (
    Args,
    Darwin,
    Driver,
    IceProcess,
    Mapping,
    Option,
    Process,
    Props,
    Result,
    TestCase,
    TestSuite,
    parseOptions,
    platform,
    runTests,
    toplevel,
    traceback,
)


class ControllerDriver(Driver):
    class Current(Driver.Current):
        def __init__(
            self,
            driver: ControllerDriver,
            testsuite: TestSuite,
            testcase: TestCase,
            cross: Mapping | None,
            protocol: str | None,
            host: str | None,
            args: Args,
        ):
            Driver.Current.__init__(self, driver, testsuite, Result(testsuite, driver.debug))
            self.testcase = testcase
            serverTestCase = self.testcase.getServerTestCase(cross)
            clientTestCase = self.testcase.getClientTestCase()
            # getCurrent rejects a test case without a server side before building a Current.
            assert serverTestCase is not None and clientTestCase is not None
            self.serverTestCase = serverTestCase
            self.clientTestCase = clientTestCase
            self.cross = cross
            self.host = host
            self.args = args
            if protocol:
                self.config.protocol = protocol

    @classmethod
    def getSupportedArgs(cls):
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
    def usage(cls):
        print("")
        print("Controller driver options:")
        print("--id=<identity>       The identity of the controller object.")
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

    def __init__(self, options: list[Option], *args: Any, **kargs: Any):
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

    def run(self, mappings: list[Mapping], testSuiteIds: list[str]) -> int | None:
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
                        print("\nerror: couldn't add trust settings for the HTTP server certificate")
                    else:
                        print("ok")
                    print("run " + sys.argv[0] + " --clean to remove the trust setting")

        self.initCommunicator()
        import Ice

        Ice.loadSlice([os.path.join(toplevel, "scripts", "Controller.ice")])

        from Test import Common as Test_Common  # pyright: ignore[reportMissingImports]

        class TestCaseI(Test_Common.TestCase):  # pyright: ignore[reportUntypedBaseClass]
            def __init__(self, driver: ControllerDriver, current: ControllerDriver.Current):
                self.driver = driver
                self.current = current
                self.serverSideRunning = False

            def startServerSide(self, config: Any, c: Any) -> Any:
                self.updateCurrent(config)
                try:
                    self.serverSideRunning = True
                    return self.current.serverTestCase._startServerSide(self.current)
                except Exception:
                    self.serverSideRunning = False
                    raise Test_Common.TestCaseFailedException(
                        self.current.result.getOutput() + "\n" + traceback.format_exc()
                    )

            def stopServerSide(self, success: bool, c: Any) -> str:
                if not self.serverSideRunning:
                    return self.current.result.getOutput()
                try:
                    self.current.serverTestCase._stopServerSide(self.current, success)
                except Exception as ex:
                    # Leave serverSideRunning set so destroy() retries the teardown.
                    raise Test_Common.TestCaseFailedException(self.current.result.getOutput() + "\n" + str(ex))
                self.serverSideRunning = False
                return self.current.result.getOutput()

            def runClientSide(self, host: str, config: Any, c: Any) -> str:
                self.updateCurrent(config)
                try:
                    self.current.clientTestCase._runClientSide(self.current, host)
                    return self.current.result.getOutput()
                except Exception as ex:
                    raise Test_Common.TestCaseFailedException(self.current.result.getOutput() + "\n" + str(ex))

            def destroy(self, c: Any) -> None:
                if self.serverSideRunning:
                    self.serverSideRunning = False
                    try:
                        self.current.serverTestCase._stopServerSide(self.current, False)
                    except Exception:
                        pass
                c.adapter.remove(c.id)

            def updateCurrent(self, config: Any) -> None:
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

        class ControllerI(Test_Common.Controller):  # pyright: ignore[reportUntypedBaseClass]
            def __init__(self, driver: ControllerDriver):
                self.driver = driver
                self.testcase = None

            def runTestCase(self, mapping: str, testsuite: str, testcase: str, cross: str, c: Any) -> Any:
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

            def getTestSuites(self, mapping: str, c: Any) -> list[str]:
                m = Mapping.getByName(mapping)
                config = self.driver.configs[m]
                return [str(t) for t in m.getTestSuites() if not m.filterTestSuite(t.getId(), config)]

            def getOptionOverrides(self, c: Any) -> Any:
                return Test_Common.OptionOverrides(ipv6=([False] if not self.driver.hostIPv6 else [False, True]))

            def getHost(self, protocol: str, ipv6: bool, c: Any) -> str:
                return self.driver.getHost(protocol, ipv6)

        import Ice

        self.initCommunicator()
        communicator = self.getCommunicator()
        communicator.getProperties().setProperty("ControllerAdapter.Endpoints", self.endpoints)
        communicator.getProperties().setProperty("ControllerAdapter.AdapterId", str(uuid.uuid4()))
        adapter = communicator.createObjectAdapter("ControllerAdapter")
        adapter.add(ControllerI(self), Ice.stringToIdentity(self.id))
        adapter.activate()
        communicator.waitForShutdown()

    @staticmethod
    def emulatorPort(serial: str) -> int:
        # "emulator-5554" -> 5554. The prefix has to be checked too, not just the port: adb names an
        # emulator after the port it is listening on, so a serial like "phone-5554" would boot an
        # emulator that adb knows as "emulator-5554" and leave every later command pointed at a
        # serial that does not exist -- failing slowly, as a boot timeout, rather than here.
        prefix, _, port = serial.rpartition("-")
        if prefix != "emulator" or not port.isdigit():
            raise RuntimeError(f"expected an emulator serial like 'emulator-5554', got '{serial}'")
        return int(port)

    def runBluetoothEmulators(self) -> int:
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

    def runBluetoothPrepare(self) -> int:
        # Prepare both emulators (in parallel, each with its own log) and bond the client to the
        # server. Progress goes to stderr and only the server's Bluetooth address to stdout, so
        # callers can capture it directly.
        from Util import AndroidProcessController

        if not (self.btClient and self.btServer and self.btSetup and self.uuid):
            raise RuntimeError("--bt-prepare requires --bt-client, --bt-server, --bt-setup=<apk> and --uuid")

        # Re-invoke this script per device so each emulator's setup keeps a separate log.
        running: dict[str, tuple[subprocess.Popen[bytes], IO[bytes]]] = {}
        try:
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
        except BaseException:
            # Don't leave an already-started sibling running: it would keep driving its emulator
            # through root/remount/reboot after we've given up.
            for process, log in running.values():
                process.kill()
                log.close()
            raise
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

    def runBluetoothDevice(self) -> int:
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
            controller.grantRuntimePermissions("com.zeroc.btbond", ["android.permission.BLUETOOTH_CONNECT"])
            print(f"BT_ADDRESS={controller.bluetoothAddress()}")

        if self.btBond:
            if not self.uuid:
                raise RuntimeError("--bt-bond requires --uuid=<uuid>")
            controller.bond(self.btBond, self.uuid)
            print(f"bonded {device} to {self.btBond}")

        if self.btDiagnostics:
            controller.diagnostics()

        return 0

    def getCurrent(
        self,
        mapping: str,
        testsuite: str,
        testcase: str,
        cross: str,
        protocol: str | None = None,
        host: str | None = None,
        args: Args = [],
    ) -> ControllerDriver.Current:
        from Test import Common as Test_Common  # pyright: ignore[reportMissingImports]

        # Mapping.getByName raises RuntimeError for an unknown mapping, but runTestCase is declared to
        # throw TestCaseNotExistException.
        try:
            clientMapping = Mapping.getByName(mapping)
        except RuntimeError as ex:
            raise Test_Common.TestCaseNotExistException(str(ex))

        crossMapping = None
        if cross:
            try:
                crossMapping = Mapping.getByName(cross)
            except RuntimeError as ex:
                raise Test_Common.TestCaseNotExistException("{0} for cross testing".format(ex))

        ts = clientMapping.findTestSuite(testsuite)
        if not ts:
            raise Test_Common.TestCaseNotExistException("unknown testsuite {0}".format(testsuite))

        tc = ts.findTestCase("server" if ts.getId() == "Ice/echo" else (testcase or "client/server"))
        if not tc or not tc.getServerTestCase():
            raise Test_Common.TestCaseNotExistException("unknown testcase {0}".format(testcase))

        return ControllerDriver.Current(self, ts, tc, crossMapping, protocol, host, args)

    def getProps(self, process: Process, current: Driver.Current) -> Props:
        props = Driver.getProps(self, process, current)
        if isinstance(process, IceProcess) and current.host:
            props["Ice.Default.Host"] = current.host
        return props

    def getArgs(self, process: Process, current: Driver.Current) -> Args:
        assert isinstance(current, ControllerDriver.Current)
        return current.args

    def isWorkerThread(self) -> bool:
        return True

    def isInterrupted(self) -> bool:
        return False


Driver.add("controller", ControllerDriver, default=True)

runTests()
