// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Printer.h>

using namespace std;
using namespace Demo;

class InvokeClient : public Ice::Application
{
public:

    virtual int run(int, char*[]);
    virtual void interruptCallback(int);

private:

    void menu();
};

int
main(int argc, char* argv[])
{
    InvokeClient app;
    return app.main(argc, argv, "config.client");
}

static ostream&
operator<<(ostream& out, Demo::Color c)
{
    switch(c)
    {
    case Demo::red:
        out << "red";
        break;
    case Demo::green:
        out << "green";
        break;
    case Demo::blue:
        out << "blue";
        break;
    }
    return out;
}

int
InvokeClient::run(int argc, char* argv[])
{
    //
    // Since this is an interactive demo we want the custom interrupt
    // callback to be called when the process is interrupted.
    //
    callbackOnInterrupt();

    Ice::ObjectPrx obj = communicator()->propertyToProxy("Printer.Proxy");

    menu();

    char ch;
    do
    {
        try
        {
            cout << "==> ";
            cin >> ch;
            if(ch == '1')
            {
                //
                // Marshal the in parameter.
                //
                Ice::ByteSeq inParams, outParams;
                Ice::OutputStreamPtr out = Ice::createOutputStream(communicator());
                out->writeString("The streaming API works!");
                out->finished(inParams);

                //
                // Invoke operation.
                //
                if(!obj->ice_invoke("printString", Ice::Normal, inParams, outParams))
                {
                    cout << "Unknown user exception" << endl;
                }
            }
            else if(ch == '2')
            {
                //
                // Marshal the in parameter.
                //
                Ice::ByteSeq inParams, outParams;
                Ice::OutputStreamPtr out = Ice::createOutputStream(communicator());
                Demo::StringSeq arr;
                arr.push_back("The");
                arr.push_back("streaming");
                arr.push_back("API");
                arr.push_back("works!");
                out->writeStringSeq(arr);
                out->finished(inParams);

                //
                // Invoke operation.
                //
                if(!obj->ice_invoke("printStringSequence", Ice::Normal, inParams, outParams))
                {
                    cout << "Unknown user exception" << endl;
                }
            }
            else if(ch == '3')
            {
                //
                // Marshal the in parameter.
                //
                Ice::ByteSeq inParams, outParams;
                Ice::OutputStreamPtr out = Ice::createOutputStream(communicator());
                Demo::StringDict dict;
                dict["The"] = "streaming";
                dict["API"] = "works!";
                Demo::ice_writeStringDict(out, dict);
                out->finished(inParams);

                //
                // Invoke operation.
                //
                if(!obj->ice_invoke("printDictionary", Ice::Normal, inParams, outParams))
                {
                    cout << "Unknown user exception" << endl;
                }
            }
            else if(ch == '4')
            {
                //
                // Marshal the in parameter.
                //
                Ice::ByteSeq inParams, outParams;
                Ice::OutputStreamPtr out = Ice::createOutputStream(communicator());
                Demo::ice_writeColor(out, Demo::green);
                out->finished(inParams);

                //
                // Invoke operation.
                //
                if(!obj->ice_invoke("printEnum", Ice::Normal, inParams, outParams))
                {
                    cout << "Unknown user exception" << endl;
                }
            }
            else if(ch == '5')
            {
                //
                // Marshal the in parameter.
                //
                Ice::ByteSeq inParams, outParams;
                Ice::OutputStreamPtr out = Ice::createOutputStream(communicator());
                Demo::Structure s;
                s.name = "red";
                s.value = Demo::red;
                Demo::ice_writeStructure(out, s);
                out->finished(inParams);

                //
                // Invoke operation.
                //
                if(!obj->ice_invoke("printStruct", Ice::Normal, inParams, outParams))
                {
                    cout << "Unknown user exception" << endl;
                }
            }
            else if(ch == '6')
            {
                //
                // Marshal the in parameter.
                //
                Ice::ByteSeq inParams, outParams;
                Ice::OutputStreamPtr out = Ice::createOutputStream(communicator());
                Demo::StructureSeq arr;
                arr.push_back(Demo::Structure());
                arr.back().name = "red";
                arr.back().value = Demo::red;
                arr.push_back(Demo::Structure());
                arr.back().name = "green";
                arr.back().value = Demo::green;
                arr.push_back(Demo::Structure());
                arr.back().name = "blue";
                arr.back().value = Demo::blue;
                Demo::ice_writeStructureSeq(out, arr);
                out->finished(inParams);

                //
                // Invoke operation.
                //
                if(!obj->ice_invoke("printStructSequence", Ice::Normal, inParams, outParams))
                {
                    cout << "Unknown user exception" << endl;
                }
            }
            else if(ch == '7')
            {
                //
                // Marshal the in parameter.
                //
                Ice::ByteSeq inParams, outParams;
                Ice::OutputStreamPtr out = Ice::createOutputStream(communicator());
                Demo::CPtr c = new Demo::C;
                c->s.name = "blue";
                c->s.value = Demo::blue;
                Demo::ice_writeC(out, c);
                out->writePendingObjects();
                out->finished(inParams);

                //
                // Invoke operation.
                //
                if(!obj->ice_invoke("printClass", Ice::Normal, inParams, outParams))
                {
                    cout << "Unknown user exception" << endl;
                }
            }
            else if(ch == '8')
            {
                //
                // Invoke operation.
                //
                Ice::ByteSeq inParams, outParams;
                if(!obj->ice_invoke("getValues", Ice::Normal, inParams, outParams))
                {
                    cout << "Unknown user exception" << endl;
                    continue;
                }

                //
                // Unmarshal the results.
                //
                Ice::InputStreamPtr in = Ice::createInputStream(communicator(), outParams);
                Demo::CPtr c;
                Demo::ice_readC(in, c);
                string str = in->readString();
                in->readPendingObjects();
                cout << "Got string `" << str << "' and class: s.name=" << c->s.name
                     << ", s.value=" << c->s.value << endl;
            }
            else if(ch == '9')
            {
                //
                // Invoke operation.
                //
                Ice::ByteSeq inParams, outParams;
                if(obj->ice_invoke("throwPrintFailure", Ice::Normal, inParams, outParams))
                {
                    cout << "Expected exception" << endl;
                    continue;
                }

                Ice::InputStreamPtr in = Ice::createInputStream(communicator(), outParams);
                try
                {
                    in->throwException();
                }
                catch(const Demo::PrintFailure&)
                {
                    // Expected.
                }
                catch(const Ice::UserException&)
                {
                    cout << "Unknown user exception" << endl;
                }
            }
            else if(ch == 's')
            {
                Ice::ByteSeq inParams, outParams;
                obj->ice_invoke("shutdown", Ice::Normal, inParams, outParams);
            }
            else if(ch == 'x')
            {
                // Nothing to do.
            }
            else if(ch == '?')
            {
                menu();
            }
            else
            {
                cout << "unknown command `" << ch << "'" << endl;
                menu();
            }
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
        }
    }
    while(cin.good() && ch != 'x');

    return EXIT_SUCCESS;
}

void
InvokeClient::interruptCallback(int)
{
    try
    {
        communicator()->destroy();
    }
    catch(const IceUtil::Exception& ex)
    {
        cerr << appName() << ": " << ex << endl;
    }
    catch(...)
    {
        cerr << appName() << ": unknown exception" << endl;
    }
    exit(EXIT_SUCCESS);
}

void
InvokeClient::menu()
{
    cout <<
        "usage:\n"
        "1: print string\n"
        "2: print string sequence\n"
        "3: print dictionary\n"
        "4: print enum\n"
        "5: print struct\n"
        "6: print struct sequence\n"
        "7: print class\n"
        "8: get values\n"
        "9: throw exception\n"
        "s: shutdown server\n"
        "x: exit\n"
        "?: help\n";
}
