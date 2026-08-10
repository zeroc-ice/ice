# Copyright (c) ZeroC, Inc.

from __future__ import annotations

import select
import socket
import threading
import time

# The remote address a connection proxies to, as read from the connect request.
Address = tuple[str, int]


class InvalidRequest(Exception):
    pass


class BaseConnection(threading.Thread):
    def __init__(self, sock: socket.socket):
        threading.Thread.__init__(self)
        self.socket: socket.socket | None = sock
        self.remoteSocket: socket.socket | None = None
        self.closed = False

    def response(self, success: bool) -> bytes:
        # Overridden per protocol to build the reply to a connect request.
        raise NotImplementedError()

    def sendResponse(self, success: bool) -> None:
        assert self.socket is not None
        self.socket.sendall(self.response(success))

    def request(self, sock: socket.socket) -> Address:
        # Overridden per protocol to read the connect request and return the address it asks for.
        raise NotImplementedError()

    def close(self) -> None:
        if self.closed:
            return
        self.closed = True
        try:
            if self.socket:
                self.socket.close()
                self.socket = None

            if self.remoteSocket:
                self.remoteSocket.close()
                self.remoteSocket = None
        except Exception:
            pass

    def run(self) -> None:
        try:
            assert self.socket is not None
            remoteAddr = self.request(self.socket)
            self.remoteSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            try:
                self.remoteSocket.connect(remoteAddr)
                self.sendResponse(True)
            except Exception:
                self.sendResponse(False)
                return

            try:
                while not self.closed:
                    readables, _, _ = select.select([self.socket, self.remoteSocket], [], [])
                    for r in readables:
                        w = self.remoteSocket if r == self.socket else self.socket
                        assert w is not None
                        data = r.recv(4096)
                        if len(data) == 0:
                            self.closed = True
                            break
                        w.sendall(data)
            except InvalidRequest:
                print("invalid request")
            except Exception:
                pass
        except Exception:
            pass
        finally:
            if self.socket:
                self.socket.close()
            if self.remoteSocket:
                self.remoteSocket.close()


class BaseProxy(threading.Thread):
    def __init__(self, port: int):
        threading.Thread.__init__(self)
        self.port = port
        self.closed = False
        self.cond = threading.Condition()
        self.socket: socket.socket | None = None
        self.failed: BaseException | None = None
        self.connections: list[BaseConnection] = []
        self.start()
        with self.cond:
            while not self.socket and not self.failed:
                self.cond.wait(60)
        if self.failed:
            raise self.failed

    def createConnection(self, sock: socket.socket) -> BaseConnection:
        # Overridden to build the protocol specific connection.
        raise NotImplementedError()

    def run(self) -> None:
        with self.cond:
            try:
                self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
                if hasattr(socket, "SO_REUSEPORT"):
                    try:
                        self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
                    except Exception:
                        # Ignore, this can throw on some platforms if not supported (e.g: ARMHF/Qemu)
                        pass
                self.socket.bind(("127.0.0.1", self.port))
                self.socket.listen(1)
                self.cond.notify()
            except Exception as ex:
                self.failed = ex
                # socket() itself can fail, leaving nothing to close. Notify either way, otherwise the
                # constructor sits out its full 60 second wait before reporting the failure.
                if self.socket:
                    self.socket.close()
                    self.socket = None
                self.cond.notify()
                return

        try:
            assert self.socket is not None
            while not self.closed:
                incoming, _ = self.socket.accept()
                connection = self.createConnection(incoming)
                connection.start()
                with self.cond:
                    self.connections.append(connection)
        except Exception:
            pass
        finally:
            if self.socket:
                self.socket.close()
            self.socket = None

    def terminate(self) -> None:
        with self.cond:
            if self.closed:
                return
            self.closed = True
            for c in self.connections:
                try:
                    c.close()
                    c.join()
                except Exception as ex:
                    print(ex)

        connectToSelf = None
        try:
            connectToSelf = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            connectToSelf.connect(("127.0.0.1", self.port))
        except Exception as ex:
            print(ex)
        finally:
            if connectToSelf:
                connectToSelf.close()
        self.join()


class SocksConnection(BaseConnection):
    def request(self, sock: socket.socket) -> Address:
        data = sock.recv(9)  # Read the 9 bytes request

        if not data or len(data) == 0:
            raise InvalidRequest
        if data[0] != 4:
            raise InvalidRequest
        if data[1] != 1:
            raise InvalidRequest

        port = (data[2] << 8) + data[3]
        addr = socket.inet_ntoa(data[4:8])
        return (addr, port)

    def response(self, success: bool) -> bytes:
        def encode(c: int) -> str:
            return chr(c)

        packet = encode(0)
        packet += encode(90 if success else 91)
        packet += encode(0)
        packet += encode(0)
        packet += encode(0)
        packet += encode(0)
        packet += encode(0)
        packet += encode(0)
        return bytes(packet, "ascii")


class SocksProxy(BaseProxy):
    def createConnection(self, sock: socket.socket) -> BaseConnection:
        return SocksConnection(sock)


class HttpConnection(BaseConnection):
    def request(self, sock: socket.socket) -> Address:
        def decode(c: bytes) -> str:
            return chr(c[0])

        data = ""
        while len(data) < 4 or data[len(data) - 4 :] != "\r\n\r\n":
            data += decode(sock.recv(1))

        if data.find("CONNECT ") != 0:
            raise InvalidRequest

        sep = data.find(":")
        if sep < len("CONNECT ") + 1:
            raise InvalidRequest

        host = data[len("CONNECT ") : sep]
        space = data.find(" ", sep)
        if space < sep + 1:
            raise InvalidRequest

        port = int(data[sep + 1 : space])
        return (host, port)

    def response(self, success: bool) -> bytes:
        if success:
            s = "HTTP/1.1 200 OK\r\nServer: CERN/3.0 libwww/2.17\r\n\r\n"
        else:
            s = "HTTP/1.1 404\r\n\r\n"
        return bytes(s, "ascii")

    def sendResponse(self, success: bool) -> None:
        assert self.socket is not None
        if not success:
            self.socket.sendall(self.response(False))
            return

        # Deliberately fragment the successful CONNECT response so the first read the Ice client performs
        # returns fewer than the 7 bytes it initially asks for. This exercises the proxy transport's
        # partial-read handling: a correct endRead keeps reading until the response is complete, whereas a
        # buggy one treats the short first read as a complete response and fails the connection.
        self.socket.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
        data = self.response(True)
        self.socket.sendall(data[:4])
        time.sleep(0.1)
        self.socket.sendall(data[4:])


class HttpProxy(BaseProxy):
    def createConnection(self, sock: socket.socket) -> BaseConnection:
        return HttpConnection(sock)
