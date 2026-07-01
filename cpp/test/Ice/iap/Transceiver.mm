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
        test([in streamStatus] >= NSStreamStatusOpen);
        test([out streamStatus] >= NSStreamStatusOpen);
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

    // Minimal selector-callback stub recording the last operation reported by the transceiver.
    class TestReadyCallback final : public SelectorReadyCallback
    {
    public:
        void readyCallback(SocketOperation op, int = 0) final
        {
            lock_guard lock(_mutex);
            _ops = static_cast<SocketOperation>(_ops | op);
        }

        SocketOperation ops()
        {
            lock_guard lock(_mutex);
            return _ops;
        }

    private:
        mutex _mutex;
        SocketOperation _ops{SocketOperationNone};
    };

    void testRunLoopRegistration(const ProtocolInstancePtr& instance)
    {
        cout << "testing iAP transceiver run-loop registration... " << flush;
        FakePeer peer;
        NSInputStream* in;
        NSOutputStream* out;
        makeFakePeer(peer, in, out);

        auto transceiver = make_shared<IceObjC::iAPTransceiver>(instance, in, out, "test");

        // Let the transceiver open and schedule the streams, and drive the open completion through the
        // delegate callback (mirrors how Ice's selector thread uses the transceiver).
        TestReadyCallback callback;
        transceiver->initStreams(&callback);
        transceiver->registerWithRunLoop(SocketOperationConnect);

        for (int i = 0; i < 200 && (callback.ops() & SocketOperationConnect) == 0; ++i)
        {
            pump();
        }
        test((callback.ops() & SocketOperationConnect) != 0);

        transceiver->unregisterFromRunLoop(SocketOperationWrite, false);
        transceiver->closeStreams();
        cout << "ok" << endl;
    }
}

void
allTestsTransceiver(const Ice::CommunicatorPtr& communicator)
{
    auto instance = make_shared<ProtocolInstance>(communicator, iAPEndpointType, "iap", false);
    testReadWrite(instance);
    testConnectionLost(instance);
    testRunLoopRegistration(instance);
}

#endif
