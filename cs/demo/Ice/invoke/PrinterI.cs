// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections;

public class PrinterI : Ice.Blobject
{
    public override bool ice_invoke(byte[] inParams, out byte[] outParams, Ice.Current current)
    {
        outParams = null;

        Ice.Communicator communicator = current.adapter.getCommunicator();

        Ice.InputStream inStream = null;
        if(inParams.Length > 0)
        {
            inStream = Ice.Util.createInputStream(communicator, inParams);
        }

        if(current.operation.Equals("printString"))
        {
            string message = inStream.readString();
            inStream.destroy();
            Console.WriteLine("Printing string `" + message + "'");
            return true;
        }
        else if(current.operation.Equals("printStringSequence"))
        {
            String[] seq = Demo.StringSeqHelper.read(inStream);
            inStream.destroy();
            Console.Write("Printing string sequence {");
            for(int i = 0; i < seq.Length; ++i)
            {
                if(i > 0)
                {
                    Console.Write(", ");
                }
                Console.Write("'" + seq[i] + "'");
            }
            Console.WriteLine("}");
            return true;
        }
        else if(current.operation.Equals("printDictionary"))
        {
            Demo.StringDict dict = Demo.StringDictHelper.read(inStream);
            inStream.destroy();
            Console.Write("Printing dictionary {");
            bool first = true;
            foreach(DictionaryEntry e in dict)
            {
                if(!first)
                {
                    Console.Write(", ");
                }
                first = false;
                Console.Write(e.Key.ToString() + "=" + e.Value.ToString());
            }
            Console.WriteLine("}");
            return true;
        }
        else if(current.operation.Equals("printEnum"))
        {
            Demo.Color c = Demo.ColorHelper.read(inStream);
            inStream.destroy();
            Console.WriteLine("Printing enum " + c);
            return true;
        }
        else if(current.operation.Equals("printStruct"))
        {
            Demo.Structure s = new Demo.Structure();
            s.ice_read(inStream);
            inStream.destroy();
            Console.WriteLine("Printing struct: name=" + s.name + ", value=" + s.value);
            return true;
        }
        else if(current.operation.Equals("printStructSequence"))
        {
            Demo.Structure[] seq = Demo.StructureSeqHelper.read(inStream);
            inStream.destroy();
            Console.Write("Printing struct sequence: {");
            for(int i = 0; i < seq.Length; ++i)
            {
                if(i > 0)
                {
                    Console.Write(", ");
                }
                Console.Write(seq[i].name + "=" + seq[i].value);
            }
            Console.WriteLine("}");
            return true;
        }
        else if(current.operation.Equals("printClass"))
        {
            Demo.CHelper ch = new Demo.CHelper(inStream);
            ch.read();
            inStream.readPendingObjects();
            inStream.destroy();
            Demo.C c = ch.value;
            Console.WriteLine("Printing class: s.name=" + c.s.name + ", s.value=" + c.s.value);
            return true;
        }
        else if(current.operation.Equals("getValues"))
        {
            Demo.C c = new Demo.C();
            c.s = new Demo.Structure();
            c.s.name = "green";
            c.s.value = Demo.Color.green;
            Ice.OutputStream outStream = Ice.Util.createOutputStream(communicator);
            Demo.CHelper.write(outStream, c);
            outStream.writeString("hello");
            outStream.writePendingObjects();
            outParams = outStream.finished();
            return true;
        }
        else if(current.operation.Equals("throwPrintFailure"))
        {
            Console.WriteLine("Throwing PrintFailure");
            Demo.PrintFailure ex = new Demo.PrintFailure();
            ex.reason = "paper tray empty";
            Ice.OutputStream outStream = Ice.Util.createOutputStream(communicator);
            outStream.writeException(ex);
            outParams = outStream.finished();
            return false;
        }
        else if(current.operation.Equals("shutdown"))
        {
            current.adapter.getCommunicator().shutdown();
            return true;
        }
        else
        {
            Ice.OperationNotExistException ex = new Ice.OperationNotExistException();
            ex.id = current.id;
            ex.facet = current.facet;
            ex.operation = current.operation;
            throw ex;
        }
    }
}
