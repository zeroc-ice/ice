# Copyright (c) ZeroC, Inc.

from __future__ import annotations

import codecs
import io
import os
import platform
import re
import signal
import string
import subprocess
import sys
import threading
import time
import traceback
from collections.abc import Callable
from typing import Literal, Protocol, TextIO, overload

__all__ = ["Expect", "EOF", "TIMEOUT"]

if sys.platform == "win32":
    import ctypes

win32 = sys.platform == "win32"


class EOF:
    """Raised when EOF is read from a child."""

    def __init__(self, value: object):
        self.value = value

    def __str__(self) -> str:
        return str(self.value)


class TIMEOUT(Exception):
    """Raised when a read time exceeds the timeout."""

    def __init__(self, value: object):
        self.value = value

    def __str__(self) -> str:
        return str(self.value)


# An expect pattern: the string the caller asked for (or the TIMEOUT sentinel, which never matches
# but lets a caller ask for the timeout to be reported rather than raised) and its compiled regexp.
MatchPattern = tuple[str | type[TIMEOUT], re.Pattern[str] | None]

# The result of a single (non matchall) match: the buffer that was matched against, the portion
# before the match, the portion after it, the match object and the index of the pattern that
# matched. When the caller asked for TIMEOUT, "after" is the TIMEOUT sentinel and the match is None.
MatchResult = tuple[str, str, str | type[TIMEOUT], re.Match[str] | None, int]

# A trace filter is either a regexp whose matching lines are suppressed, or a function that rewrites
# the line.
TraceFilter = re.Pattern[str] | Callable[[str], str]


class WatchDog(Protocol):
    """Notified each time a child process produces output, so a hang can be distinguished from slow progress."""

    def reset(self) -> None: ...


def escape(s: str | type[TIMEOUT], escapeNewlines: bool = True) -> str:
    if s is TIMEOUT:
        return "<TIMEOUT>"
    assert isinstance(s, str)
    o = io.StringIO()
    for c in s:
        if c == "\\":
            o.write("\\\\")
        elif c == "'":
            o.write("\\'")
        elif c == '"':
            o.write('\\"')
        elif c == "\b":
            o.write("\\b")
        elif c == "\f":
            o.write("\\f")
        elif c == "\n":
            if escapeNewlines:
                o.write("\\n")
            else:
                o.write("\n")
        elif c == "\r":
            o.write("\\r")
        elif c == "\t":
            o.write("\\t")
        else:
            if c in string.printable:
                o.write(c)
            else:
                o.write("\\%03o" % ord(c))
    return o.getvalue()


def taskkill(args: str) -> None:
    p = subprocess.Popen(
        "taskkill {0}".format(args),
        shell=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
    )
    p.wait()
    assert p.stdout is not None
    p.stdout.close()


def killProcess(p: subprocess.Popen[bytes]) -> None:
    if win32:
        taskkill("/F /T /PID {0}".format(p.pid))
    else:
        os.kill(p.pid, signal.SIGKILL)


def terminateProcess(p: subprocess.Popen[bytes], hasInterruptSupport: bool = True) -> None:
    if sys.platform == "win32":
        #
        # Signals under windows are all turned into CTRL_BREAK_EVENT, except with Java since
        # CTRL_BREAK_EVENT generates a stack trace. We don't use taskkill here because it
        # doesn't work with CLI processes (it sends a WM_CLOSE event).
        #
        if hasInterruptSupport:
            try:
                ctypes.windll.kernel32.GenerateConsoleCtrlEvent(1, p.pid)  # 1 is CTRL_BREAK_EVENT
            except Exception:
                traceback.print_exc(file=sys.stdout)
                taskkill("/F /T /PID {0}".format(p.pid))
        else:
            taskkill("/F /T /PID {0}".format(p.pid))
    else:
        os.kill(p.pid, signal.SIGTERM)


class reader(threading.Thread):
    def __init__(self, desc: str | None, p: subprocess.Popen[bytes], logfile: TextIO | None):
        self.desc = desc
        self.buf = io.StringIO()
        self.cv = threading.Condition()
        self.p = p
        self._trace = False
        self._tbuf = io.StringIO()
        self._tracesuppress: list[TraceFilter] | None = None
        self.logfile = logfile
        self.watchDog: WatchDog | None = None
        self._finish = False
        threading.Thread.__init__(self)

    def setWatchDog(self, watchDog: WatchDog) -> None:
        self.watchDog = watchDog

    def run(self) -> None:
        # Depending on the platform, the value read below is either a string or a single byte, in
        # which case it's decoded incrementally since a character can span several reads.
        decoder = codecs.getincrementaldecoder("utf-8")("replace")
        assert self.p.stdout is not None
        try:
            while True:
                c = self.p.stdout.read(1)
                if not c:
                    self.cv.acquire()
                    try:
                        # Flush the character the decoder may still be holding, otherwise a process
                        # exiting in the middle of a multi-byte sequence loses its last character.
                        for char in decoder.decode(b"", True):
                            self.trace(char)
                            self.buf.write(char)
                        self.trace(None)
                        self._finish = True  # We have finished processing output
                        self.cv.notify()
                    finally:
                        self.cv.release()
                    break
                if not isinstance(c, str):
                    c = decoder.decode(c)
                    if not c:
                        continue  # Incomplete character, read more bytes.

                self.cv.acquire()
                try:
                    # Decoding can yield more than one character, so trace and buffer them one by one.
                    for char in c:
                        if char == "\r":
                            continue
                        self.trace(char)
                        self.buf.write(char)
                    if self.watchDog is not None:
                        self.watchDog.reset()
                    self.cv.notify()
                finally:
                    self.cv.release()
        except Exception as e:
            print(e)
        finally:
            # Always wake up the waiters, otherwise they would block until their timeout expires.
            self.cv.acquire()
            self._finish = True
            self.cv.notify()
            self.cv.release()

    def trace(self, c: str | None) -> None:
        if self._trace:
            if self._tracesuppress:
                if c is not None:
                    self._tbuf.write(c)
                if c == "\n" or c is None:
                    content = self._tbuf.getvalue()
                    suppress = False
                    for p in self._tracesuppress:
                        if isinstance(p, re.Pattern):
                            if p.search(content):
                                suppress = True
                                break
                        else:
                            content = p(content)
                    if not suppress:
                        sys.stdout.write(content)
                    self._tbuf.truncate(0)
                    self._tbuf.seek(0)
            elif c is not None:
                sys.stdout.write(c)
                sys.stdout.flush()

    def enabletrace(self, suppress: list[TraceFilter] | None = None) -> None:
        self.cv.acquire()
        try:
            if not self._trace:
                self._trace = True
                self._tracesuppress = suppress
                for c in self.buf.getvalue():
                    self.trace(c)
        finally:
            self.cv.release()

    def getbuf(self) -> str:
        self.cv.acquire()
        try:
            buf = self.buf.getvalue()
        finally:
            self.cv.release()
        return buf

    @overload
    def match(
        self, pattern: list[MatchPattern], timeout: float | None, matchall: Literal[False] = False
    ) -> MatchResult: ...

    @overload
    def match(self, pattern: list[MatchPattern], timeout: float | None, matchall: Literal[True]) -> str: ...

    def match(self, pattern: list[MatchPattern], timeout: float | None, matchall: bool = False) -> str | MatchResult:
        # pattern is a list of string, regexp duples.

        end = time.time() + timeout if timeout is not None else None
        start = time.time()

        # Trace the match
        if self.logfile:
            if timeout is None:
                tdesc = "<infinite>"
            else:
                tdesc = "%.2fs" % timeout
            pdesc = io.StringIO()
            if len(pattern) == 1:
                pdesc.write(escape(pattern[0][0]))
            else:
                pdesc.write("[")
                for index, (s, _) in enumerate(pattern):
                    if index > 0:
                        pdesc.write(",")
                    pdesc.write(escape(s))
                pdesc.write("]")
            self.logfile.write('%s: expect: "%s" timeout: %s\n' % (self.desc, pdesc.getvalue(), tdesc))
            self.logfile.flush()

        maxend = None

        def patternDesc() -> str:
            return ",".join([escape(s) for (s, _) in pattern])

        buf = ""
        self.cv.acquire()
        try:
            try:  # This second try/except block is necessary because of python 2.3
                while True:
                    buf = self.buf.getvalue()

                    # Try to match on the current buffer.
                    olen = len(pattern)
                    for index, p in enumerate(pattern):
                        s, regexp = p
                        if s == TIMEOUT:
                            continue
                        if not buf:
                            #
                            # Don't try to match on an empty buffer, http://bugs.python.org/issue17998
                            #
                            break
                        assert regexp is not None
                        m = regexp.search(buf)
                        if m is not None:
                            before = buf[: m.start()]
                            matched = buf[m.start() : m.end()]
                            after = buf[m.end() :]

                            if maxend is None or m.end() > maxend:
                                maxend = m.end()

                            # Trace the match
                            if self.logfile:
                                if len(pattern) > 1:
                                    self.logfile.write(
                                        '%s: match found in %.2fs.\npattern: "%s"\nbuffer: "%s||%s||%s"\n'
                                        % (
                                            self.desc,
                                            time.time() - start,
                                            escape(s),
                                            escape(before),
                                            escape(matched),
                                            escape(after),
                                        )
                                    )
                                else:
                                    self.logfile.write(
                                        '%s: match found in %.2fs.\nbuffer: "%s||%s||%s"\n'
                                        % (
                                            self.desc,
                                            time.time() - start,
                                            escape(before),
                                            escape(matched),
                                            escape(after),
                                        )
                                    )

                            if matchall:
                                del pattern[index]
                                # If all patterns have been found then
                                # truncate the buffer to the longest match,
                                # and then return.
                                if len(pattern) == 0:
                                    self.buf.truncate(0)
                                    self.buf.seek(0)
                                    self.buf.write(buf[maxend:])
                                    return buf
                                break

                            # Consume matched portion of the buffer.
                            self.buf.truncate(0)
                            self.buf.seek(0)
                            self.buf.write(after)

                            return buf, before, after, m, index

                    # If a single match was found then the match.
                    if len(pattern) != olen:
                        continue

                    # If no match and we have finished processing output raise a TIMEOUT
                    if self._finish:
                        raise TIMEOUT(
                            'timeout exceeded in match\npattern: "%s"\nbuffer: "%s"\n'
                            % (patternDesc(), escape(buf, False))
                        )

                    if end is None:
                        self.cv.wait()
                    else:
                        self.cv.wait(end - time.time())
                        if time.time() >= end:
                            # Log the failure
                            if self.logfile:
                                self.logfile.write(
                                    '%s: match failed.\npattern: "%s"\nbuffer: "%s"\n'
                                    % (self.desc, patternDesc(), escape(buf))
                                )
                                self.logfile.flush()
                            raise TIMEOUT(
                                'timeout exceeded in match\npattern: "%s"\nbuffer: "%s"\n'
                                % (patternDesc(), escape(buf, False))
                            )
            except TIMEOUT as e:
                if (TIMEOUT, None) in pattern:
                    return buf, buf, TIMEOUT, None, pattern.index((TIMEOUT, None))
                raise e
        finally:
            self.cv.release()


def splitCommand(command_line: str) -> list[str]:
    arg_list: list[str] = []
    arg = ""

    state_basic = 0
    state_esc = 1
    state_singlequote = 2
    state_doublequote = 3
    state_whitespace = 4
    state = state_basic
    pre_esc_state = state_basic

    for c in command_line:
        if state != state_esc and c == "\\":
            pre_esc_state = state
            state = state_esc
        elif state == state_basic or state == state_whitespace:
            if c == r"'":
                state = state_singlequote
            elif c == r'"':
                state = state_doublequote
            elif c.isspace():
                if state == state_whitespace:
                    None
                else:
                    arg_list.append(arg)
                    arg = ""
                    state = state_whitespace
            else:
                arg = arg + c
                state = state_basic
        elif state == state_esc:
            arg = arg + c
            state = pre_esc_state
        elif state == state_singlequote:
            if c == r"'":
                state = state_basic
            else:
                arg = arg + c
        elif state == state_doublequote:
            if c == r'"':
                state = state_basic
            else:
                arg = arg + c

    if arg != "":
        arg_list.append(arg)

    return arg_list


processes: dict[int, subprocess.Popen[bytes]] = {}


def cleanup() -> None:
    for key in processes.copy():
        try:
            killProcess(processes[key])
        except Exception:
            pass
    processes.clear()


class Expect(object):
    def __init__(
        self,
        command: str,
        startReader: bool = True,
        timeout: float = 30,
        logfile: TextIO | None = None,
        mapping: str | None = None,
        desc: str | None = None,
        cwd: str | None = None,
        env: dict[str, str] | None = None,
        preexec_fn: Callable[[], object] | None = None,
    ):
        self.buf = ""  # The part before the match
        self.before = ""  # The part before the match
        self.after: str | type[TIMEOUT] = ""  # The part after the match
        self.matchindex = 0  # the index of the matched pattern
        self.match: re.Match[str] | None = None  # The last match
        self.mapping = mapping  # The mapping of the test.
        self.exitstatus: int | None = None  # The exitstatus, either -signal or, if positive, the exit code.
        self.killed: int | None = None  # If killed, the signal that was sent.
        self.desc = desc
        self.logfile = logfile
        self.timeout = timeout
        self.p: subprocess.Popen[bytes] | None = None
        self.cwd = cwd

        if self.logfile:
            self.logfile.write('spawn: "%s"\n' % command)
            self.logfile.flush()

        if win32:
            # Don't rely on win32api
            # import win32process
            # creationflags = win32process.CREATE_NEW_PROCESS_GROUP)
            #
            # We can't use shell=True because terminate() wouldn't
            # work. This means the PATH isn't searched for the
            # command.
            #
            CREATE_NEW_PROCESS_GROUP = 512
            self.p = subprocess.Popen(
                command,
                env=env,
                cwd=cwd,
                shell=False,
                bufsize=0,
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                creationflags=CREATE_NEW_PROCESS_GROUP,
            )
        else:
            self.p = subprocess.Popen(
                splitCommand(command),
                env=env,
                cwd=cwd,
                shell=False,
                bufsize=0,
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                preexec_fn=preexec_fn,
            )
        global processes
        processes[self.p.pid] = self.p

        self.r: reader | None = reader(desc, self.p, logfile)

        # The thread is marked as a daemon thread. This is done so that if
        # an expect script runs off the end of main without kill/wait on each
        # spawned process the script will not hang trying to join with the
        # reader thread.
        self.r.daemon = True

        if startReader:
            self.startReader()

    def __str__(self) -> str:
        return "{0} pid={1}".format(self.desc, "<none>" if self.p is None else self.p.pid)

    def startReader(self, watchDog: WatchDog | None = None) -> None:
        assert self.r is not None
        if watchDog is not None:
            self.r.setWatchDog(watchDog)
        self.r.start()

    def expect(self, pattern: str | list[str], timeout: float | None = 60) -> int:
        """pattern is either a string, or a list of string regexp patterns.

        timeout == None expect can block indefinitely.

        timeout == -1 then the default is used.
        """
        if timeout == -1:
            timeout = self.timeout

        if not isinstance(pattern, list):
            pattern = [pattern]

        def compile(s: str) -> re.Pattern[str] | None:
            if isinstance(s, str):
                return re.compile(s, re.S)
            return None

        compiled: list[MatchPattern] = [(p, compile(p)) for p in pattern]
        assert self.r is not None
        try:
            (
                self.buf,
                self.before,
                self.after,
                self.match,
                self.matchindex,
            ) = self.r.match(compiled, timeout)
        except TIMEOUT as e:
            self.buf = ""
            self.before = ""
            self.after = ""
            self.match = None
            self.matchindex = 0
            raise e
        return self.matchindex

    def expectall(self, pattern: list[str], timeout: float | None = 60) -> None:
        """pattern is a list of string regexp patterns.

        timeout == None expect can block indefinitely.

        timeout == -1 then the default is used.
        """
        if timeout == -1:
            timeout = self.timeout

        compiled: list[MatchPattern] = [(p, re.compile(p, re.S)) for p in pattern]
        assert self.r is not None
        try:
            self.buf = self.r.match(compiled, timeout, matchall=True)
            self.before = ""
            self.after = ""
            self.matchindex = 0
            self.match = None
        except TIMEOUT as e:
            self.buf = ""
            self.before = ""
            self.after = ""
            self.matchindex = 0
            self.match = None
            raise e

    def sendline(self, data: str) -> None:
        """send data to the application."""
        if self.logfile:
            self.logfile.write('%s: sendline: "%s"\n' % (self.desc, escape(data)))
            self.logfile.flush()
        data = data + "\n"
        assert self.p is not None and self.p.stdin is not None
        self.p.stdin.write(data.encode("utf-8"))

    def wait(self, timeout: float | None = None) -> int | None:
        """Wait for the application to terminate for up to timeout seconds, or
        raises a TIMEOUT exception. If timeout is None, the wait is
        indefinite.

        The exit status is returned. A negative exit status means
        the application was killed by a signal.
        """
        if self.p is None:
            return self.exitstatus

        # Unfortunately, with the subprocess module there is no
        # better method of doing a timed wait.
        if timeout is not None:
            end = time.time() + timeout
            while time.time() < end and self.p and self.p.poll() is None:
                time.sleep(0.1)
            if self.p and self.p.poll() is None:
                raise TIMEOUT("timed wait exceeded timeout")
        elif win32:
            # We poll on Windows or otherwise KeyboardInterrupt isn't delivered
            while self.p.poll() is None:
                time.sleep(0.5)

        # Another thread may have terminated the process while we were polling above. Pyright assumes
        # single-threaded execution and so considers this check redundant with the one on entry.
        if self.p is None:  # pyright: ignore[reportUnnecessaryComparison]
            return self.exitstatus

        self.exitstatus = self.p.wait()

        # A Windows application killed with CTRL_BREAK. Fudge the exit status.
        if win32 and self.exitstatus != 0 and self.killed is not None:
            self.exitstatus = -self.killed
        global processes
        if self.p.pid in processes:
            del processes[self.p.pid]
        self.p = None
        assert self.r is not None
        self.r.join()
        # Simulate a match on EOF
        self.buf = self.r.getbuf()
        self.before = self.buf
        self.after = ""
        #
        # Without this we get warnings when runing with python_d on Windows
        #
        # ResourceWarning: unclosed file <_io.TextIOWrapper name=3 encoding='cp1252'>
        #
        assert self.r.p.stdout is not None and self.r.p.stdin is not None
        self.r.p.stdout.close()
        self.r.p.stdin.close()
        self.r = None

        return self.exitstatus

    def terminate(self) -> None:
        """Terminate the process."""

        if self.p is None:
            return

        def kill() -> None:
            ex: BaseException | None = None
            for _ in range(5):
                try:
                    if not self.p:
                        return
                    killProcess(self.p)
                    self.wait()
                    ex = None
                    break
                except KeyboardInterrupt as e:
                    ex = e
                    raise
                except Exception as e:
                    ex = e

            if ex:
                print(ex)
                raise ex

        try:
            self.wait(timeout=0.5)
            return
        except KeyboardInterrupt:
            kill()
            raise
        except TIMEOUT:
            pass

        try:
            assert self.p is not None
            terminateProcess(self.p, self.hasInterruptSupport())
        except KeyboardInterrupt:
            kill()
            raise
        except Exception:
            traceback.print_exc(file=sys.stdout)

        # If the break does not terminate the process within 5
        # seconds, then kill the process.
        try:
            self.wait(timeout=5)
            return
        except KeyboardInterrupt:
            kill()
            raise
        except TIMEOUT:
            kill()

    def kill(self, sig: int) -> None:
        """Send the signal to the process."""
        self.killed = sig  # Save the sent signal.
        assert self.p is not None
        if win32:
            terminateProcess(self.p, self.hasInterruptSupport())
        else:
            os.kill(self.p.pid, sig)

    def trace(self, suppress: list[TraceFilter] | None = None) -> None:
        assert self.r is not None
        self.r.enabletrace(suppress)

    def waitSuccess(self, exitstatus: int = 0, timeout: float | None = None) -> None:
        """Wait for the process to terminate for up to timeout seconds, and
        validate the exit status is as expected."""
        self.wait(timeout)
        self.testExitStatus(exitstatus)

    def getOutput(self) -> str:
        if self.p is None:
            return self.buf
        assert self.r is not None
        return self.r.getbuf()

    def hasInterruptSupport(self) -> bool:
        """Return True if the application gracefully terminated, False otherwise."""
        if win32 and self.mapping == "java":
            return False
        return True

    def testExitStatus(self, exitstatus: int) -> None:
        def test(result: int | None, expected: int | list[int]) -> None:
            if not win32 and result == -2:  # Interrupted by Ctrl-C, simulate KeyboardInterrupt
                raise KeyboardInterrupt()
            if isinstance(expected, list):
                if result not in expected:
                    raise RuntimeError("unexpected exit status: expected either: {0}, got {1}".format(expected, result))
            else:
                if expected != result:
                    raise RuntimeError("unexpected exit status: expected: {0}, got {1}\n".format(expected, result))

        if self.killed is not None:
            #
            # If we explicitly signaled the process, the exitstatus might not be the expected status. Java
            # returns 128 + signal for the exit status. Other language mapping executables either return
            # -signal or 0 depending on the signal value (in general 0 SIGTERM and -2 for SIGINT). On Windows,
            # we explicitly set the exitstatus to -signal when we explicitly kill the process.
            #
            if not win32 and self.mapping == "java":
                test(self.exitstatus, 128 + self.killed)
            else:
                test(self.exitstatus, [exitstatus, -self.killed])
        else:
            test(self.exitstatus, exitstatus)

    def stackDump(self) -> None:
        assert self.p is not None
        match platform.system():
            case "Linux":
                dumpCmd = [
                    "gdb",
                    "-q",
                    "-n",
                    "-batch",
                    "-ex",
                    f"attach {self.p.pid}",
                    "-ex",
                    "thread apply all bt",
                    "-ex",
                    "detach",
                    "-ex",
                    "quit",
                ]
            case "Darwin":
                dumpCmd = [
                    "lldb",
                    "-p",
                    f"{self.p.pid}",
                    "-o",
                    "thread backtrace all",
                    "-o",
                    "process detach",
                    "-o",
                    "quit",
                ]
            case _:
                print(f"stackDump not supported on {platform.system()}")
                return

        try:
            # The args should be a list
            args = self.p.args
            assert isinstance(args, list)
            cmd = args[0]
            # The cmd should be absolute path
            assert os.path.isabs(cmd)

            exe = os.path.split(cmd)[1]
            assert self.cwd is not None
            coreDumpFile = os.path.join(self.cwd, f"{exe}-{time.strftime('%m%d%y-%H%M')}-coredump.log")
            print(f"(dumping stack for {cmd} to {coreDumpFile})")
            with open(coreDumpFile, "w") as f:
                subprocess.run(dumpCmd, stdout=f, stderr=f)
        except Exception as e:
            print(e)
