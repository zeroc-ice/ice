// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <PrinterI.h>

using namespace std;

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

bool
PrinterI::ice_invoke(const vector<Ice::Byte>& inParams, vector<Ice::Byte>& outParams, const Ice::Current& current)
{
    Ice::CommunicatorPtr communicator = current.adapter->getCommunicator();

    Ice::InputStreamPtr in = Ice::createInputStream(communicator, inParams);
    in->startEncapsulation();

    if(current.operation == "printString")
    {
        string message;
        in->read(message);
        in->endEncapsulation();
        cout << "Printing string `" << message << "'" << endl;
        return true;
    }
    else if(current.operation == "printStringSequence")
    {
        Demo::StringSeq seq;
        in->read(seq);
        in->endEncapsulation();
        cout << "Printing string sequence {";
        for(Demo::StringSeq::iterator p = seq.begin(); p != seq.end(); ++p)
        {
            if(p != seq.begin())
            {
                cout << ", ";
            }
            cout << "'" << *p << "'";
        }
        cout << "}" << endl;
        return true;
    }
    else if(current.operation == "printDictionary")
    {
        Demo::StringDict dict;
        in->read(dict);
        in->endEncapsulation();
        cout << "Printing dictionary {";
        for(Demo::StringDict::iterator p = dict.begin(); p != dict.end(); ++p)
        {
            if(p != dict.begin())
            {
                cout << ", ";
            }
            cout << p->first << "=" << p->second;
        }
        cout << "}" << endl;
        return true;
    }
    else if(current.operation == "printEnum")
    {
        Demo::Color c;
        in->read(c);
        in->endEncapsulation();
        cout << "Printing enum " << c << endl;
        return true;
    }
    else if(current.operation == "printStruct")
    {
        Demo::Structure s;
        in->read(s);
        in->endEncapsulation();
        cout << "Printing struct: name=" << s.name << ", value=" << s.value << endl;
        return true;
    }
    else if(current.operation == "printStructSequence")
    {
        Demo::StructureSeq seq;
        in->read(seq);
        in->endEncapsulation();
        cout << "Printing struct sequence: {";
        for(Demo::StructureSeq::iterator p = seq.begin(); p != seq.end(); ++p)
        {
            if(p != seq.begin())
            {
                cout << ", ";
            }
            cout << p->name << "=" << p->value;
        }
        cout << "}" << endl;
        return true;
    }
    else if(current.operation == "printClass")
    {
        Demo::CPtr c;
        in->read(c);
        in->endEncapsulation();
        cout << "Printing class: s.name=" << c->s.name << ", s.value=" << c->s.value << endl;
        return true;
    }
    else if(current.operation == "getValues")
    {
        Demo::CPtr c = new Demo::C;
        c->s.name = "green";
        c->s.value = Demo::green;
        Ice::OutputStreamPtr out = Ice::createOutputStream(communicator);
        out->startEncapsulation();
        out->write(c);
        out->write("hello");
        out->endEncapsulation();
        out->finished(outParams);
        return true;
    }
    else if(current.operation == "throwPrintFailure")
    {
        cout << "Throwing PrintFailure" << endl;
        Demo::PrintFailure ex;
        ex.reason = "paper tray empty";
        Ice::OutputStreamPtr out = Ice::createOutputStream(communicator);
        out->startEncapsulation();
        out->write(ex);
        out->endEncapsulation();
        out->finished(outParams);
        return false;
    }
    else if(current.operation == "shutdown")
    {
        current.adapter->getCommunicator()->shutdown();
        return true;
    }
    else
    {
        Ice::OperationNotExistException ex(__FILE__, __LINE__);
        ex.id = current.id;
        ex.facet = current.facet;
        ex.operation = current.operation;
        throw ex;
    }

    return false;
}
