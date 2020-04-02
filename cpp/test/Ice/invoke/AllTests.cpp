//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;

static string testString = "This is a test string";

Test::MyClassPrxPtr
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    string ref = "test:" + helper->getTestEndpoint();
    Ice::ObjectPrxPtr base = communicator->stringToProxy(ref);
    test(base);

    Test::MyClassPrxPtr cl = ICE_CHECKED_CAST(Test::MyClassPrx, base);
    test(cl);

    Test::MyClassPrxPtr oneway = cl->ice_oneway();
    Test::MyClassPrxPtr batchOneway = cl->ice_batchOneway();

    cout << "testing ice_invoke... " << flush;

    {
        Ice::ByteSeq inEncaps, outEncaps;
        if(!oneway->ice_invoke("opOneway", Ice::OperationMode::Normal, inEncaps, outEncaps))
        {
            test(false);
        }

        test(batchOneway->ice_invoke("opOneway", Ice::OperationMode::Normal, inEncaps, outEncaps));
        test(batchOneway->ice_invoke("opOneway", Ice::OperationMode::Normal, inEncaps, outEncaps));
        test(batchOneway->ice_invoke("opOneway", Ice::OperationMode::Normal, inEncaps, outEncaps));
        test(batchOneway->ice_invoke("opOneway", Ice::OperationMode::Normal, inEncaps, outEncaps));
        batchOneway->ice_flushBatchRequests();

        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(testString);
        out.endEncapsulation();
        out.finished(inEncaps);

        // ice_invoke
        if(cl->ice_invoke("opString", Ice::OperationMode::Normal, inEncaps, outEncaps))
        {
            Ice::InputStream in(communicator, out.getEncoding(), outEncaps);
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
        pair<const ::Ice::Byte*, const ::Ice::Byte*> inPair(&inEncaps[0], &inEncaps[0] + inEncaps.size());
        if(cl->ice_invoke("opString", Ice::OperationMode::Normal, inPair, outEncaps))
        {
            Ice::InputStream in(communicator, out.getEncoding(), outEncaps);
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

    for(int i = 0; i < 2; ++i)
    {
        Ice::ByteSeq inEncaps, outEncaps;
        Ice::Context ctx;
        if(i == 1)
        {
            ctx["raise"] = "";
        }
        if(cl->ice_invoke("opException", Ice::OperationMode::Normal, inEncaps, outEncaps, ctx))
        {
            test(false);
        }
        else
        {
            Ice::InputStream in(communicator, cl->ice_getEncodingVersion(), outEncaps);
            in.startEncapsulation();
            try
            {
                in.throwException();
            }
            catch(const Test::MyException&)
            {
            }
            catch(...)
            {
                test(false);
            }
            in.endEncapsulation();
        }
    }

    cout << "ok" << endl;

    cout << "testing asynchronous ice_invoke... " << flush;

    {
        Ice::ByteSeq inEncaps;
        batchOneway->ice_invokeAsync("opOneway", Ice::OperationMode::Normal, inEncaps,
            [](bool, const vector<Ice::Byte>)
            {
                test(false);
            },
            [](exception_ptr)
            {
                test(false);
            },
            [](bool)
            {
                test(false);
            });
        batchOneway->ice_flushBatchRequests();
    }
    //
    // repeat with the future API.
    //
    {
        Ice::ByteSeq inEncaps;
        test(batchOneway->ice_invokeAsync("opOneway", Ice::OperationMode::Normal, inEncaps).get().returnValue);
        test(batchOneway->ice_invokeAsync("opOneway", Ice::OperationMode::Normal, inEncaps).get().returnValue);
        test(batchOneway->ice_invokeAsync("opOneway", Ice::OperationMode::Normal, inEncaps).get().returnValue);
        test(batchOneway->ice_invokeAsync("opOneway", Ice::OperationMode::Normal, inEncaps).get().returnValue);
        batchOneway->ice_flushBatchRequests();
    }

    {
        promise<bool> completed;
        Ice::ByteSeq inEncaps, outEncaps;
        oneway->ice_invokeAsync(
            "opOneway",
            Ice::OperationMode::Normal,
            inEncaps,
            nullptr,
            [&](exception_ptr ex)
            {
                completed.set_exception(ex);
            },
            [&](bool)
            {
                completed.set_value(true);
            });

        test(completed.get_future().get());
    }

    //
    // repeat with the future API.
    //

    {
        Ice::ByteSeq inEncaps, outEncaps;
        auto completed = oneway->ice_invokeAsync("opOneway", Ice::OperationMode::Normal, inEncaps);
        test(completed.get().returnValue);
    }

    {
        promise<bool> completed;
        Ice::ByteSeq inEncaps, outEncaps;
        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(testString);
        out.endEncapsulation();
        out.finished(inEncaps);

        cl->ice_invokeAsync("opString", Ice::OperationMode::Normal, inEncaps,
            [&](bool ok, vector<Ice::Byte> outParams)
            {
                outEncaps = move(outParams);
                completed.set_value(ok);
            },
            [&](exception_ptr ex)
            {
                completed.set_exception(ex);
            });
        test(completed.get_future().get());

        Ice::InputStream in(communicator, outEncaps);
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
        Ice::ByteSeq inEncaps, outEncaps;
        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(testString);
        out.endEncapsulation();
        out.finished(inEncaps);

        auto result = cl->ice_invokeAsync("opString", Ice::OperationMode::Normal, inEncaps).get();
        test(result.returnValue);

        Ice::InputStream in(communicator, result.outParams);
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
        Ice::ByteSeq inEncaps, outEncaps;
        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(testString);
        out.endEncapsulation();
        out.finished(inEncaps);

        auto inPair = make_pair(inEncaps.data(), inEncaps.data() + inEncaps.size());

        cl->ice_invokeAsync("opString", Ice::OperationMode::Normal, inPair,
            [&](bool ok, pair<const Ice::Byte*, const Ice::Byte*> outParams)
            {
                vector<Ice::Byte>(outParams.first, outParams.second).swap(outEncaps);
                completed.set_value(ok);
            },
            [&](exception_ptr ex)
            {
                completed.set_exception(ex);
            },
            [&](bool)
            {
                sent.set_value();
            });
        sent.get_future().get(); // Ensure sent callback was called
        test(completed.get_future().get());

        Ice::InputStream in(communicator, outEncaps);
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
        Ice::ByteSeq inEncaps, outEncaps;
        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.write(testString);
        out.endEncapsulation();
        out.finished(inEncaps);

        auto inPair = make_pair(inEncaps.data(), inEncaps.data() + inEncaps.size());

        auto result = cl->ice_invokeAsync("opString", Ice::OperationMode::Normal, inPair).get();
        test(result.returnValue);

        Ice::InputStream in(communicator, result.outParams);
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
        Ice::ByteSeq inEncaps, outEncaps;

        cl->ice_invokeAsync("opException", Ice::OperationMode::Normal, inEncaps,
            [&](bool ok, vector<Ice::Byte> outParams)
            {
                outEncaps = move(outParams);
                completed.set_value(ok);
            },
            [&](exception_ptr ex)
            {
                completed.set_exception(ex);
            },
            [&](bool)
            {
                sent.set_value();
            });
        sent.get_future().get(); // Ensure sent callback was called
        test(!completed.get_future().get());

        Ice::InputStream in(communicator, outEncaps);
        in.startEncapsulation();
        try
        {
            in.throwException();
            test(false);
        }
        catch(const Test::MyException&)
        {
        }
        catch(...)
        {
            test(false);
        }
    }
    //
    // repeat with the future API.
    //
    {
        Ice::ByteSeq inEncaps;
        auto result = cl->ice_invokeAsync("opException", Ice::OperationMode::Normal, inEncaps).get();
        test(!result.returnValue);

        Ice::InputStream in(communicator, result.outParams);
        in.startEncapsulation();
        try
        {
            in.throwException();
            test(false);
        }
        catch(const Test::MyException&)
        {
        }
        catch(...)
        {
            test(false);
        }
    }

    cout << "ok" << endl;
    return cl;
}
