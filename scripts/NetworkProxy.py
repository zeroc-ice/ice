#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

import sys
import threading
import socket
import select


class InvalidRequest(Exception):
    pass


class BaseConnection(threading.Thread):
    def __init__(self, socket, remote):
        threading.Thread.__init__(self)
        self.socket = socket
        self.remote = remote
        self.remoteSocket = None
        self.closed = False

    def response(self, code):
        pass

    def request(self, data):
        pass

    def close(self):
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
        except:
            pass

    def run(self):
        try:
            remoteAddr = self.request(self.socket)
            self.remoteSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            try:
                self.remoteSocket.connect(remoteAddr)
                self.socket.send(self.response(True))
            except:
                self.socket.send(self.response(False))
                return

            try:
                while(not self.closed):
                    readables, writeables, exceptions = select.select([self.socket, self.remoteSocket], [], [])
                    for r in readables:
                        w = self.remoteSocket if r == self.socket else self.socket
                        data = r.recv(4096)
                        if(len(data) == 0):
                            self.closed = True
                            break
                        w.send(data)
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
    def __init__(self, port):
        threading.Thread.__init__(self)
        self.port = port
        self.closed = False
        self.cond = threading.Condition()
        self.socket = None
        self.failed = None
        self.connections = []
        self.start()
        with self.cond:
            while not self.socket and not self.failed:
                self.cond.wait(60)
        if self.failed:
            raise self.failed

    def createConnection(self):
        return None

    def run(self):
        with self.cond:
            try:
                self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
                if hasattr(socket, "SO_REUSEPORT"):
                    try:
                        self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
                    except:
                        # Ignore, this can throw on some platforms if not supported (e.g: ARMHF/Qemu)
                        pass
                self.socket.bind(("127.0.0.1", self.port))
                self.socket.listen(1)
                self.cond.notify()
            except Exception as ex:
                self.failed = ex
                self.socket.close()
                self.socket = None
                self.cond.notify()
                return

        try:
            while not self.closed:
                incoming, peer = self.socket.accept()
                connection = self.createConnection(incoming, peer)
                connection.start()
                with self.cond:
                    self.connections.append(connection)
        except:
            pass
        finally:
            self.socket.close()
            self.socket = None

    def terminate(self):
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

        try:
            connectToSelf = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            connectToSelf.connect(("127.0.0.1", self.port))
        except Exception as ex:
            print(ex)
        finally:
            connectToSelf.close()
            connectToSelf = None
        self.join()


class SocksConnection(BaseConnection):

    def request(self, s):
        def decode(c):
            return ord(c) if sys.version_info[0] == 2 else c

        data = s.recv(9)  # Read the 9 bytes request

        if not data or len(data) == 0:
            raise InvalidRequest
        if decode(data[0]) != 4:
            raise InvalidRequest
        if decode(data[1]) != 1:
            raise InvalidRequest

        port = (decode(data[2]) << 8) + decode(data[3])
        addr = socket.inet_ntoa(data[4:8])
        return (addr, port)

    def response(self, success):
        def encode(c):
            return chr(c)

        packet = encode(0)
        packet += encode(90 if success else 91)
        packet += encode(0)
        packet += encode(0)
        packet += encode(0)
        packet += encode(0)
        packet += encode(0)
        packet += encode(0)
        return packet if sys.version_info[0] == 2 else bytes(packet, "ascii")


class SocksProxy(BaseProxy):

    def createConnection(self, socket, peer):
        return SocksConnection(socket, peer)


class HttpConnection(BaseConnection):

    def request(self, s):
        def decode(c):
            return c[0] if sys.version_info[0] == 2 else chr(c[0])

        data = ""
        while(len(data) < 4 or data[len(data) - 4:] != "\r\n\r\n"):
            data += decode(s.recv(1))

        if data.find("CONNECT ") != 0:
            raise InvalidRequest

        sep = data.find(":")
        if sep < len("CONNECT ") + 1:
            raise InvalidRequest

        host = data[len("CONNECT "):sep]
        space = data.find(" ", sep)
        if space < sep + 1:
            raise InvalidRequest

        port = int(data[sep + 1:space])
        return (host, port)

    def response(self, success):
        if(success):
            s = "HTTP/1.1 200 OK\r\nServer: CERN/3.0 libwww/2.17\r\n\r\n"
        else:
            s = "HTTP/1.1 404\r\n\r\n"
        return s if sys.version_info[0] == 2 else bytes(s, "ascii")


class HttpProxy(BaseProxy):

    def createConnection(self, socket, peer):
        return HttpConnection(socket, peer)
