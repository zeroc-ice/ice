// Copyright (c) ZeroC, Inc.

#include "Ice/Config.h"

#if TARGET_OS_IPHONE != 0

#    include "../../src/Ice/EventHandler.h"
#    include "../../src/Ice/Instance.h"
#    include "../../src/Ice/ProtocolInstance.h"
#    include "../../src/Ice/Selector.h"
#    include "../../src/Ice/ios/iAPTransceiver.h"
#    include "Ice/Buffer.h"
#    include "Ice/EndpointTypes.h"
#    include "Ice/Ice.h"
#    include "Ice/LocalExceptions.h"
#    include "TestHelper.h"

#    include <CoreFoundation/CoreFoundation.h>
#    include <chrono>
#    include <fcntl.h>
#    include <sys/socket.h>
#    include <thread>
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

    // Builds a connected socket pair: the local end is wrapped in a CFStream pair (returned as the
    // NSStreams the transceiver opens and uses), the peer end is a raw non-blocking fd the test drives directly.
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

    Buffer makeBuffer(const string& data)
    {
        const auto* p = reinterpret_cast<const byte*>(data.data());
        return Buffer{p, p + data.size()};
    }

    // The event handler the transceiver's native info is registered with: it only carries the native info, the
    // completions are consumed by the test through the selector.
    class TestHandler final : public EventHandler
    {
    public:
        TestHandler(NativeInfoPtr nativeInfo) : _nativeInfo(std::move(nativeInfo)) {}

        bool startAsync(SocketOperation) final { return false; }
        bool finishAsync(SocketOperation) final { return false; }
        void message(ThreadPoolCurrent&) final {}
        void finished(ThreadPoolCurrent&, bool) final {}
        [[nodiscard]] string toString() const final { return "iAP test handler"; }
        NativeInfoPtr getNativeInfo() final { return _nativeInfo; }

    private:
        const NativeInfoPtr _nativeInfo;
    };

    // Collects the completions posted by the transceiver, the way the thread pool does with the selector.
    class Completions
    {
    public:
        Completions(const CommunicatorPtr& communicator, const shared_ptr<IceObjC::iAPTransceiver>& transceiver)
            : _selector(getInstance(communicator)),
              _handler(make_shared<TestHandler>(transceiver->getNativeInfo()))
        {
            _selector.setup(1);
            _selector.initialize(_handler.get());
        }

        ~Completions() { _selector.destroy(); }

        // Waits for the next completion and returns the completed operation.
        SocketOperation wait()
        {
            SocketOperation status = SocketOperationNone;
            size_t count = 0;
            int error = 0;
            EventHandler* handler = _selector.getNextHandler(status, count, error, 5); // 5 seconds timeout
            test(handler == _handler.get());
            return status;
        }

    private:
        Selector _selector;
        shared_ptr<TestHandler> _handler;
    };

    // Drives initialize() to StateConnected: the connect completes once both streams are open.
    void connect(const shared_ptr<IceObjC::iAPTransceiver>& transceiver, Completions& completions)
    {
        Buffer rb, wb;
        test(transceiver->initialize(rb, wb) == SocketOperationConnect);
        test(completions.wait() == SocketOperationConnect);
        test(transceiver->initialize(rb, wb) == SocketOperationNone);
    }

    // Writes the whole buffer with the asynchronous write operations, like the connection does. Returns the
    // number of operations it took.
    int writeAll(const shared_ptr<IceObjC::iAPTransceiver>& transceiver, Completions& completions, Buffer& buf)
    {
        int operations = 0;
        while (transceiver->write(buf) == SocketOperationWrite)
        {
            test(transceiver->startWrite(buf));
            test(completions.wait() == SocketOperationWrite);
            transceiver->finishWrite(buf);
            ++operations;
        }
        return operations;
    }

    int readAll(const shared_ptr<IceObjC::iAPTransceiver>& transceiver, Completions& completions, Buffer& buf)
    {
        int operations = 0;
        while (transceiver->read(buf) == SocketOperationRead)
        {
            transceiver->startRead(buf);
            test(completions.wait() == SocketOperationRead);
            transceiver->finishRead(buf);
            ++operations;
        }
        return operations;
    }

    // Reads from the peer fd until 'expected' bytes have arrived.
    string drainPeer(FakePeer& peer, size_t expected)
    {
        string received;
        for (int i = 0; i < 5000 && received.size() < expected; ++i)
        {
            char tmp[64 * 1024];
            ssize_t n = ::read(peer.peerFd, tmp, sizeof(tmp));
            if (n > 0)
            {
                received.append(tmp, static_cast<size_t>(n));
            }
            else
            {
                this_thread::sleep_for(chrono::milliseconds(1));
            }
        }
        return received;
    }

    // Writes 'data' to the (non-blocking) peer fd.
    void feedPeer(FakePeer& peer, const string& data)
    {
        size_t sent = 0;
        for (int i = 0; i < 5000 && sent < data.size(); ++i)
        {
            ssize_t n = ::write(peer.peerFd, data.data() + sent, data.size() - sent);
            if (n > 0)
            {
                sent += static_cast<size_t>(n);
            }
            else
            {
                this_thread::sleep_for(chrono::milliseconds(1));
            }
        }
        test(sent == data.size());
    }

    void testReadWrite(const CommunicatorPtr& communicator, const ProtocolInstancePtr& instance)
    {
        cout << "testing iAP transceiver read/write... " << flush;
        FakePeer peer;
        NSInputStream* in;
        NSOutputStream* out;
        makeFakePeer(peer, in, out);

        auto transceiver = make_shared<IceObjC::iAPTransceiver>(instance, in, out, "test");
        Completions completions(communicator, transceiver);
        connect(transceiver, completions);

        // Write a payload through the transceiver and read it back from the peer.
        const string payload = "hello iap";
        Buffer wbuf = makeBuffer(payload);
        writeAll(transceiver, completions, wbuf);
        test(wbuf.i == wbuf.b.end());
        test(drainPeer(peer, payload.size()) == payload);

        // Write from the peer and read it back through the transceiver.
        const string reply = "from accessory";
        feedPeer(peer, reply);

        vector<byte> readData(reply.size());
        Buffer rbuf{readData.data(), readData.data() + readData.size()};
        readAll(transceiver, completions, rbuf);
        test(rbuf.i == rbuf.b.end());
        test(string(reinterpret_cast<const char*>(readData.data()), readData.size()) == reply);

        transceiver->close();
        cout << "ok" << endl;
    }

    void testPartialIO(const CommunicatorPtr& communicator, const ProtocolInstancePtr& instance)
    {
        cout << "testing iAP transceiver partial I/O... " << flush;
        FakePeer peer;
        NSInputStream* in;
        NSOutputStream* out;
        makeFakePeer(peer, in, out);

        auto transceiver = make_shared<IceObjC::iAPTransceiver>(instance, in, out, "test");
        Completions completions(communicator, transceiver);
        connect(transceiver, completions);

        // A payload much larger than the socket buffers, so a single operation cannot transfer it and the
        // connection has to start several. Content varies per byte to catch corruption across the partial
        // transfers.
        const size_t size = 512 * 1024;
        string payload(size, '\0');
        for (size_t i = 0; i < size; ++i)
        {
            payload[i] = static_cast<char>('a' + (i % 26));
        }

        // Write direction: the peer drains concurrently, otherwise the socket buffers fill up.
        {
            string received;
            thread drainer([&] { received = drainPeer(peer, size); });
            Buffer wbuf = makeBuffer(payload);
            int operations = writeAll(transceiver, completions, wbuf);
            drainer.join();
            test(operations > 1);
            test(received == payload);
        }

        // Read direction: the peer sends the payload while the transceiver reads it into one buffer.
        {
            thread feeder([&] { feedPeer(peer, payload); });
            vector<byte> readData(size);
            Buffer rbuf{readData.data(), readData.data() + readData.size()};
            int operations = readAll(transceiver, completions, rbuf);
            feeder.join();
            test(operations > 1);
            test(string(reinterpret_cast<const char*>(readData.data()), readData.size()) == payload);
        }

        transceiver->close();
        cout << "ok" << endl;
    }

    void testConnectionLost(const CommunicatorPtr& communicator, const ProtocolInstancePtr& instance)
    {
        cout << "testing iAP transceiver connection loss... " << flush;
        FakePeer peer;
        NSInputStream* in;
        NSOutputStream* out;
        makeFakePeer(peer, in, out);

        auto transceiver = make_shared<IceObjC::iAPTransceiver>(instance, in, out, "test");
        Completions completions(communicator, transceiver);
        connect(transceiver, completions);

        // Close the peer end; the read operation must complete with ConnectionLostException.
        ::close(peer.peerFd);
        peer.peerFd = -1;

        vector<byte> readData(16);
        Buffer rbuf{readData.data(), readData.data() + readData.size()};
        transceiver->startRead(rbuf);
        test(completions.wait() == SocketOperationRead);
        try
        {
            transceiver->finishRead(rbuf);
            test(false);
        }
        catch (const Ice::ConnectionLostException&)
        {
            // Expected
        }

        transceiver->close();
        cout << "ok" << endl;
    }

    void testCloseWithPendingRead(const CommunicatorPtr& communicator, const ProtocolInstancePtr& instance)
    {
        cout << "testing iAP transceiver close with a pending read... " << flush;
        FakePeer peer;
        NSInputStream* in;
        NSOutputStream* out;
        makeFakePeer(peer, in, out);

        auto transceiver = make_shared<IceObjC::iAPTransceiver>(instance, in, out, "test");
        Completions completions(communicator, transceiver);
        connect(transceiver, completions);

        // Start a read the peer never satisfies; close() must complete it, the thread pool waits for it.
        vector<byte> readData(16);
        Buffer rbuf{readData.data(), readData.data() + readData.size()};
        transceiver->startRead(rbuf);
        transceiver->close();
        test(completions.wait() == SocketOperationRead);
        try
        {
            transceiver->finishRead(rbuf);
            test(false);
        }
        catch (const Ice::ConnectionLostException&)
        {
            // Expected
        }
        cout << "ok" << endl;
    }
}

void
allTestsTransceiver(const Ice::CommunicatorPtr& communicator)
{
    auto instance = make_shared<ProtocolInstance>(communicator, iAPEndpointType, "iap", false);
    testReadWrite(communicator, instance);
    testPartialIO(communicator, instance);
    testConnectionLost(communicator, instance);
    testCloseWithPendingRead(communicator, instance);
}

#endif
