// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

using namespace std;
using namespace Ice;
using namespace Test;

Test::MyClassPrx
allTests(Test::TestHelper* helper)
{
    static const string testString = "This is a test string";

    CommunicatorPtr communicator = helper->communicator();
    Test::MyClassPrx cl(communicator, "test:" + helper->getTestEndpoint());
    Test::MyClassPrx oneway = cl->ice_oneway();
    Test::MyClassPrx batchOneway = cl->ice_batchOneway();

    cout << "testing ice_invoke... " << flush;

    {
        ByteSeq inEncaps, outEncaps;
        if (!oneway->ice_invoke("opOneway", OperationMode::Normal, inEncaps, outEncaps))
        {
            test(false);
        }

        test(batchOneway->ice_invoke("opOneway", OperationMode::Normal, inEncaps, outEncaps));
        test(batchOneway->ice_invoke("opOneway", OperationMode::Normal, inEncaps, outEncaps));
        test(batchOneway->ice_invoke("opOneway", OperationMode::Normal, inEncaps, outEncaps));
        test(batchOneway->ice_invoke("opOneway", OperationMode::Normal, inEncaps, outEncaps));
        batchOneway->ice_flushBatchRequests();

        OutputStream out(communicator);
        out.startEncapsulation();
        out.write(testString);
        out.endEncapsulation();
        out.finished(inEncaps);

        // ice_invoke
        if (cl->ice_invoke("opString", OperationMode::Normal, inEncaps, outEncaps))
        {
            InputStream in(communicator, out.getEncoding(), outEncaps);
            in.startEncapsulation();
            string s;
            in.read(s);
            test(s == testString);
            in.read(s);
            test(s == testString);
            in.endEncapsulation();
        }
        else
        {
            test(false);
        }

        // ice_invoke with array mapping
        pair<const byte*, const byte*> inPair(&inEncaps[0], &inEncaps[0] + inEncaps.size());
        if (cl->ice_invoke("opString", OperationMode::Normal, inPair, outEncaps))
        {
            InputStream in(communicator, out.getEncoding(), outEncaps);
            in.startEncapsulation();
            string s;
            in.read(s);
            test(s == testString);
            in.read(s);
            test(s == testString);
            in.endEncapsulation();
        }
        else
        {
            test(false);
        }
    }

    for (int i = 0; i < 2; ++i)
    {
        ByteSeq inEncaps, outEncaps;
        Context ctx;
        if (i == 1)
        {
            ctx["raise"] = "";
        }
        if (cl->ice_invoke("opException", OperationMode::Normal, inEncaps, outEncaps, ctx))
        {
            test(false);
        }
        else
        {
            InputStream in(communicator, cl->ice_getEncodingVersion(), outEncaps);
            in.startEncapsulation();
            try
            {
                in.throwException();
            }
            catch (const Test::MyException&)
            {
            }
            catch (...)
            {
                test(false);
            }
            in.endEncapsulation();
        }
    }

    cout << "ok" << endl;

    cout << "testing asynchronous ice_invoke... " << flush;
    {
        ByteSeq inEncaps;
        batchOneway->ice_invokeAsync(
            "opOneway",
            OperationMode::Normal,
            inEncaps,
            [](bool, const vector<byte>&) { test(false); },
            [](exception_ptr) { test(false); },
            [](bool) { test(false); });
        batchOneway->ice_flushBatchRequests();
    }
    //
    // repeat with the future API.
    //
    {
        ByteSeq inEncaps;
        {
            auto [success, _] = batchOneway->ice_invokeAsync("opOneway", OperationMode::Normal, inEncaps).get();
            test(success);
        }
        {
            auto [success, _] = batchOneway->ice_invokeAsync("opOneway", OperationMode::Normal, inEncaps).get();
            test(success);
        }
        {
            auto [success, _] = batchOneway->ice_invokeAsync("opOneway", OperationMode::Normal, inEncaps).get();
            test(success);
        }
        {
            auto [success, _] = batchOneway->ice_invokeAsync("opOneway", OperationMode::Normal, inEncaps).get();
            test(success);
        }

        batchOneway->ice_flushBatchRequests();
    }

    {
        promise<bool> completed;
        ByteSeq inEncaps, outEncaps;
        oneway->ice_invokeAsync(
            "opOneway",
            OperationMode::Normal,
            inEncaps,
            nullptr,
            [&](exception_ptr ex) { completed.set_exception(ex); },
            [&](bool) { completed.set_value(true); });

        test(completed.get_future().get());
    }

    //
    // repeat with the future API.
    //

    {
        ByteSeq inEncaps, outEncaps;
        auto completed = oneway->ice_invokeAsync("opOneway", OperationMode::Normal, inEncaps);
        auto [success, outParams] = completed.get();
        test(success);
    }

    {
        promise<bool> completed;
        ByteSeq inEncaps, outEncaps;
        OutputStream out(communicator);
        out.startEncapsulation();
        out.write(testString);
        out.endEncapsulation();
        out.finished(inEncaps);

        cl->ice_invokeAsync(
            "opString",
            OperationMode::Normal,
            inEncaps,
            [&](bool ok, vector<byte> outParams)
            {
                outEncaps = std::move(outParams);
                completed.set_value(ok);
            },
            [&](exception_ptr ex) { completed.set_exception(ex); });
        test(completed.get_future().get());

        InputStream in(communicator, outEncaps);
        in.startEncapsulation();
        string s;
        in.read(s);
        test(s == testString);
        in.read(s);
        test(s == testString);
        in.endEncapsulation();
    }
    //
    // repeat with the future API.
    //
    {
        ByteSeq inEncaps, outEncaps;
        OutputStream out(communicator);
        out.startEncapsulation();
        out.write(testString);
        out.endEncapsulation();
        out.finished(inEncaps);

        auto [success, outParams] = cl->ice_invokeAsync("opString", OperationMode::Normal, inEncaps).get();
        test(success);

        InputStream in(communicator, outParams);
        in.startEncapsulation();
        string s;
        in.read(s);
        test(s == testString);
        in.read(s);
        test(s == testString);
        in.endEncapsulation();
    }

    {
        promise<bool> completed;
        promise<void> sent;
        ByteSeq inEncaps, outEncaps;
        OutputStream out(communicator);
        out.startEncapsulation();
        out.write(testString);
        out.endEncapsulation();
        out.finished(inEncaps);

        auto inPair = make_pair(inEncaps.data(), inEncaps.data() + inEncaps.size());

        cl->ice_invokeAsync(
            "opString",
            OperationMode::Normal,
            inPair,
            [&](bool ok, pair<const byte*, const byte*> outParams)
            {
                vector<byte>(outParams.first, outParams.second).swap(outEncaps);
                completed.set_value(ok);
            },
            [&](exception_ptr ex) { completed.set_exception(ex); },
            [&](bool) { sent.set_value(); });
        sent.get_future().get(); // Ensure sent callback was called
        test(completed.get_future().get());

        InputStream in(communicator, outEncaps);
        in.startEncapsulation();
        string s;
        in.read(s);
        test(s == testString);
        in.read(s);
        test(s == testString);
        in.endEncapsulation();
    }
    //
    // repeat with the future API.
    //

    {
        ByteSeq inEncaps, outEncaps;
        OutputStream out(communicator);
        out.startEncapsulation();
        out.write(testString);
        out.endEncapsulation();
        out.finished(inEncaps);

        auto inPair = make_pair(inEncaps.data(), inEncaps.data() + inEncaps.size());

        auto [success, outParams] = cl->ice_invokeAsync("opString", OperationMode::Normal, inPair).get();
        test(success);

        InputStream in(communicator, outParams);
        in.startEncapsulation();
        string s;
        in.read(s);
        test(s == testString);
        in.read(s);
        test(s == testString);
        in.endEncapsulation();
    }

    {
        promise<bool> completed;
        promise<void> sent;
        ByteSeq inEncaps, outEncaps;

        cl->ice_invokeAsync(
            "opException",
            OperationMode::Normal,
            inEncaps,
            [&](bool ok, vector<byte> outParams)
            {
                outEncaps = std::move(outParams);
                completed.set_value(ok);
            },
            [&](exception_ptr ex) { completed.set_exception(ex); },
            [&](bool) { sent.set_value(); });
        sent.get_future().get(); // Ensure sent callback was called
        test(!completed.get_future().get());

        InputStream in(communicator, outEncaps);
        in.startEncapsulation();
        try
        {
            in.throwException();
            test(false);
        }
        catch (const Test::MyException&)
        {
        }
        catch (...)
        {
            test(false);
        }
    }
    //
    // repeat with the future API.
    //
    {
        ByteSeq inEncaps;
        auto [success, outParams] = cl->ice_invokeAsync("opException", OperationMode::Normal, inEncaps).get();
        test(!success);

        InputStream in(communicator, outParams);
        in.startEncapsulation();
        try
        {
            in.throwException();
            test(false);
        }
        catch (const Test::MyException&)
        {
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;
    return cl;
}
