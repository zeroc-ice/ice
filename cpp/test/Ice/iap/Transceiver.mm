// Copyright (c) ZeroC, Inc.

#include "Ice/Config.h"

#if TARGET_OS_IPHONE != 0

#    include "../../src/Ice/ProtocolInstance.h"
#    include "../../src/Ice/ios/iAPTransceiver.h"
#    include "Ice/Buffer.h"
#    include "Ice/EndpointTypes.h"
#    include "Ice/Ice.h"
#    include "Ice/LocalExceptions.h"
#    include "TestHelper.h"

#    include <CoreFoundation/CoreFoundation.h>
#    include <fcntl.h>
#    include <sys/socket.h>
#    include <unistd.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;
using namespace Test;

namespace
{
    // Owns the test-side ("accessory") end of a connected socket pair, plus the CFStreams wrapping the
    // local end that the transceiver reads/writes. The transceiver is constructed non-owning, so this
    // object must outlive it.
    struct FakePeer
    {
        int peerFd{-1};
        CFReadStreamRef readStream{nullptr};
        CFWriteStreamRef writeStream{nullptr};

        ~FakePeer()
        {
            if (readStream)
            {
                CFRelease(readStream);
            }
            if (writeStream)
            {
                CFRelease(writeStream);
            }
            if (peerFd != -1)
            {
                ::close(peerFd);
            }
        }
    };

    // Spins the run loop briefly so CFStream can service the underlying socket.
    void pump(CFTimeInterval seconds = 0.02) { CFRunLoopRunInMode(kCFRunLoopDefaultMode, seconds, false); }

    // Builds a connected socket pair: the local end is wrapped in a CFStream pair (returned as the
    // NSStreams the transceiver uses), the peer end is a raw non-blocking fd the test drives directly.
    void makeFakePeer(FakePeer& peer, NSInputStream*& in, NSOutputStream*& out)
    {
        int fds[2];
        test(::socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0);
        peer.peerFd = fds[1];
        ::fcntl(peer.peerFd, F_SETFL, ::fcntl(peer.peerFd, F_GETFL, 0) | O_NONBLOCK);

        CFStreamCreatePairWithSocket(nullptr, fds[0], &peer.readStream, &peer.writeStream);
        test(peer.readStream && peer.writeStream);
        // The CFStreams own and close the local socket once.
        CFReadStreamSetProperty(peer.readStream, kCFStreamPropertyShouldCloseNativeSocket, kCFBooleanTrue);

        in = (NSInputStream*)peer.readStream;
        out = (NSOutputStream*)peer.writeStream;
    }

    void openAndScheduleStreams(NSInputStream* in, NSOutputStream* out)
    {
        [in scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        [out scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        [in open];
        [out open];
        for (int i = 0; i < 200 && ([in streamStatus] < NSStreamStatusOpen || [out streamStatus] < NSStreamStatusOpen);
             ++i)
        {
            pump();
        }
        test([in streamStatus] == NSStreamStatusOpen);
        test([out streamStatus] == NSStreamStatusOpen);
    }

    Buffer makeBuffer(const string& data)
    {
        const auto* p = reinterpret_cast<const byte*>(data.data());
        return Buffer{p, p + data.size()};
    }

    // Reads from the peer fd until 'expected' bytes have arrived, pumping the run loop so the transceiver's
    // queued writes reach the socket.
    string drainPeer(FakePeer& peer, size_t expected)
    {
        string received;
        for (int i = 0; i < 500 && received.size() < expected; ++i)
        {
            char tmp[256];
            ssize_t n = ::read(peer.peerFd, tmp, sizeof(tmp));
            if (n > 0)
            {
                received.append(tmp, static_cast<size_t>(n));
            }
            else
            {
                pump();
            }
        }
        return received;
    }

    void testReadWrite(const ProtocolInstancePtr& instance)
    {
        cout << "testing iAP transceiver read/write... " << flush;
        FakePeer peer;
        NSInputStream* in;
        NSOutputStream* out;
        makeFakePeer(peer, in, out);

        auto transceiver = make_shared<IceObjC::iAPTransceiver>(instance, in, out, "test");
        openAndScheduleStreams(in, out);

        // Drive initialize() to StateConnected (the transceiver is not registered, so this is immediate).
        Buffer rb, wb;
        SocketOperation op = SocketOperationConnect;
        for (int i = 0; i < 100 && op != SocketOperationNone; ++i)
        {
            op = transceiver->initialize(rb, wb);
        }
        test(op == SocketOperationNone);

        // Write a payload through the transceiver and read it back from the peer.
        const string payload = "hello iap";
        Buffer wbuf = makeBuffer(payload);
        for (int i = 0; i < 500 && wbuf.i != wbuf.b.end(); ++i)
        {
            if (transceiver->write(wbuf) == SocketOperationNone)
            {
                break;
            }
            pump();
        }
        test(wbuf.i == wbuf.b.end());
        test(drainPeer(peer, payload.size()) == payload);

        // Write from the peer and read it back through the transceiver.
        const string reply = "from accessory";
        test(::write(peer.peerFd, reply.data(), reply.size()) == static_cast<ssize_t>(reply.size()));

        vector<byte> readData(reply.size());
        Buffer rbuf{readData.data(), readData.data() + readData.size()};
        for (int i = 0; i < 500 && rbuf.i != rbuf.b.end(); ++i)
        {
            if (transceiver->read(rbuf) == SocketOperationNone)
            {
                break;
            }
            pump();
        }
        test(rbuf.i == rbuf.b.end());
        test(string(reinterpret_cast<const char*>(readData.data()), readData.size()) == reply);

        transceiver->closeStreams();
        cout << "ok" << endl;
    }

    void testPartialIO(const ProtocolInstancePtr& instance)
    {
        cout << "testing iAP transceiver partial I/O... " << flush;
        FakePeer peer;
        NSInputStream* in;
        NSOutputStream* out;
        makeFakePeer(peer, in, out);

        auto transceiver = make_shared<IceObjC::iAPTransceiver>(instance, in, out, "test");
        openAndScheduleStreams(in, out);

        Buffer rb, wb;
        for (int i = 0; i < 100 && transceiver->initialize(rb, wb) != SocketOperationNone; ++i)
        {
        }

        // A payload much larger than the socket buffers, so a single write()/read() cannot transfer it in
        // one pass and must defer (SocketOperationWrite/Read). Content varies per byte to catch corruption
        // across the partial transfers.
        const size_t size = 512 * 1024;
        string payload(size, '\0');
        for (size_t i = 0; i < size; ++i)
        {
            payload[i] = static_cast<char>('a' + (i % 26));
        }

        // Write direction: interleave writing through the transceiver with draining the peer.
        Buffer wbuf = makeBuffer(payload);
        string received;
        bool writeDeferred = false;
        while (wbuf.i != wbuf.b.end() || received.size() < size)
        {
            if (wbuf.i != wbuf.b.end() && transceiver->write(wbuf) == SocketOperationWrite)
            {
                writeDeferred = true;
            }
            char tmp[256 * 1024];
            ssize_t n = ::read(peer.peerFd, tmp, sizeof(tmp));
            if (n > 0)
            {
                received.append(tmp, static_cast<size_t>(n));
            }
            pump();
        }
        test(writeDeferred);
        test(received == payload);

        // Read direction: the peer sends the payload while the transceiver reads it into one buffer.
        vector<byte> readData(size);
        Buffer rbuf{readData.data(), readData.data() + readData.size()};
        size_t sent = 0;
        bool readDeferred = false;
        while (rbuf.i != rbuf.b.end())
        {
            if (sent < size)
            {
                ssize_t n = ::write(peer.peerFd, payload.data() + sent, size - sent);
                if (n > 0)
                {
                    sent += static_cast<size_t>(n);
                }
            }
            if (transceiver->read(rbuf) == SocketOperationRead)
            {
                readDeferred = true;
            }
            pump();
        }
        test(readDeferred);
        test(string(reinterpret_cast<const char*>(readData.data()), readData.size()) == payload);

        transceiver->closeStreams();
        cout << "ok" << endl;
    }

    void testConnectionLost(const ProtocolInstancePtr& instance)
    {
        cout << "testing iAP transceiver connection loss... " << flush;
        FakePeer peer;
        NSInputStream* in;
        NSOutputStream* out;
        makeFakePeer(peer, in, out);

        auto transceiver = make_shared<IceObjC::iAPTransceiver>(instance, in, out, "test");
        openAndScheduleStreams(in, out);

        Buffer rb, wb;
        SocketOperation op = SocketOperationConnect;
        for (int i = 0; i < 100 && op != SocketOperationNone; ++i)
        {
            op = transceiver->initialize(rb, wb);
        }
        test(op == SocketOperationNone);

        // Close the peer end; a subsequent read must surface ConnectionLostException.
        ::close(peer.peerFd);
        peer.peerFd = -1;

        vector<byte> readData(16);
        Buffer rbuf{readData.data(), readData.data() + readData.size()};
        bool lost = false;
        for (int i = 0; i < 500 && !lost; ++i)
        {
            try
            {
                transceiver->read(rbuf);
                pump();
            }
            catch (const Ice::ConnectionLostException&)
            {
                lost = true;
            }
        }
        test(lost);

        transceiver->closeStreams();
        cout << "ok" << endl;
    }

    // Selector-callback stub that mirrors what Ice's selector does on iOS: it feeds each readiness
    // notification straight back into unregisterFromRunLoop (see EventHandlerWrapper::ready in
    // Selector.cpp), which is what finishes the transceiver's opening phase. Once that call returns
    // SocketOperationConnect, the connect is complete.
    class TestReadyCallback final : public SelectorReadyCallback
    {
    public:
        TestReadyCallback(IceObjC::iAPTransceiver* transceiver) : _transceiver(transceiver) {}

        void readyCallback(SocketOperation op, int error = 0) final
        {
            SocketOperation result = _transceiver->unregisterFromRunLoop(op, error != 0);
            lock_guard lock(_mutex);
            _ops = static_cast<SocketOperation>(_ops | op);
            if (result & SocketOperationConnect)
            {
                _openingComplete = true;
            }
        }

        SocketOperation ops()
        {
            lock_guard lock(_mutex);
            return _ops;
        }

        bool openingComplete()
        {
            lock_guard lock(_mutex);
            return _openingComplete;
        }

    private:
        IceObjC::iAPTransceiver* _transceiver;
        mutex _mutex;
        SocketOperation _ops{SocketOperationNone};
        bool _openingComplete{false};
    };

    void testRunLoopRegistration(const ProtocolInstancePtr& instance)
    {
        cout << "testing iAP transceiver run-loop registration... " << flush;
        FakePeer peer;
        NSInputStream* in;
        NSOutputStream* out;
        makeFakePeer(peer, in, out);

        auto transceiver = make_shared<IceObjC::iAPTransceiver>(instance, in, out, "test");

        // initialize() begins the connect: StateNeedConnect -> StateConnectPending, requesting Connect.
        Buffer rb, wb;
        test(transceiver->initialize(rb, wb) == SocketOperationConnect);

        // Register with the run loop and drive the open completion through the delegate callback, which
        // (like the real selector) feeds each notification back into unregisterFromRunLoop.
        TestReadyCallback callback{transceiver.get()};
        transceiver->initStreams(&callback);
        transceiver->registerWithRunLoop(SocketOperationConnect);

        for (int i = 0; i < 200 && !callback.openingComplete(); ++i)
        {
            pump();
        }
        test(callback.ops() & SocketOperationConnect);
        test(callback.openingComplete());

        // With the opening phase complete, initialize() advances StateConnectPending -> StateConnected.
        test(transceiver->initialize(rb, wb) == SocketOperationNone);

        transceiver->closeStreams();
        cout << "ok" << endl;
    }
}

void
allTestsTransceiver(const Ice::CommunicatorPtr& communicator)
{
    auto instance = make_shared<ProtocolInstance>(communicator, iAPEndpointType, "iap", false);
    testReadWrite(instance);
    testPartialIO(instance);
    testConnectionLost(instance);
    testRunLoopRegistration(instance);
}

#endif
