// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class PrinterI extends Ice.Blobject
{
    public boolean
    ice_invoke(byte[] inParams, Ice.ByteSeqHolder outParams, Ice.Current current)
    {
        Ice.Communicator communicator = current.adapter.getCommunicator();

        Ice.InputStream in = Ice.Util.createInputStream(communicator, inParams);
        in.startEncapsulation();

        if(current.operation.equals("printString"))
        {
            String message = in.readString();
            System.out.println("Printing string `" + message + "'");
            in.endEncapsulation();
            in.destroy();
            return true;
        }
        else if(current.operation.equals("printStringSequence"))
        {
            String[] seq = Demo.StringSeqHelper.read(in);
            in.endEncapsulation();
            in.destroy();
            System.out.print("Printing string sequence {");
            for(int i = 0; i < seq.length; ++i)
            {
                if(i > 0)
                {
                    System.out.print(", ");
                }
                System.out.print("'" + seq[i] + "'");
            }
            System.out.println("}");
            return true;
        }
        else if(current.operation.equals("printDictionary"))
        {
            java.util.Map<String, String> dict = Demo.StringDictHelper.read(in);
            in.endEncapsulation();
            in.destroy();
            System.out.print("Printing dictionary {");
            boolean first = true;
            for(java.util.Map.Entry<String, String> i : dict.entrySet())
            {
                if(!first)
                {
                    System.out.print(", ");
                }
                first = false;
                System.out.print(i.getKey() + "=" + i.getValue());
            }
            System.out.println("}");
            return true;
        }
        else if(current.operation.equals("printEnum"))
        {
            Demo.Color c = Demo.Color.ice_read(in);
            in.endEncapsulation();
            in.destroy();
            System.out.println("Printing enum " + c);
            return true;
        }
        else if(current.operation.equals("printStruct"))
        {
            Demo.Structure s = new Demo.Structure();
            s.ice_read(in);
            in.endEncapsulation();
            in.destroy();
            System.out.println("Printing struct: name=" + s.name + ", value=" + s.value);
            return true;
        }
        else if(current.operation.equals("printStructSequence"))
        {
            Demo.Structure[] seq = Demo.StructureSeqHelper.read(in);
            in.endEncapsulation();
            in.destroy();
            System.out.print("Printing struct sequence: {");
            for(int i = 0; i < seq.length; ++i)
            {
                if(i > 0)
                {
                    System.out.print(", ");
                }
                System.out.print(seq[i].name + "=" + seq[i].value);
            }
            System.out.println("}");
            return true;
        }
        else if(current.operation.equals("printClass"))
        {
            Demo.CHolder c = new Demo.CHolder();
            Demo.CHelper.read(in, c);
            in.readPendingObjects();
            in.endEncapsulation();
            in.destroy();
            System.out.println("Printing class: s.name=" + c.value.s.name + ", s.value=" + c.value.s.value);
            return true;
        }
        else if(current.operation.equals("getValues"))
        {
            Demo.C c = new Demo.C();
            c.s = new Demo.Structure();
            c.s.name = "green";
            c.s.value = Demo.Color.green;
            Ice.OutputStream out = Ice.Util.createOutputStream(communicator);
            out.startEncapsulation();
            Demo.CHelper.write(out, c);
            out.writeString("hello");
            out.writePendingObjects();
            out.endEncapsulation();
            outParams.value = out.finished();
            return true;
        }
        else if(current.operation.equals("throwPrintFailure"))
        {
            System.out.println("Throwing PrintFailure");
            Demo.PrintFailure ex = new Demo.PrintFailure();
            ex.reason = "paper tray empty";
            Ice.OutputStream out = Ice.Util.createOutputStream(communicator);
            out.startEncapsulation();
            out.writeException(ex);
            out.endEncapsulation();
            outParams.value = out.finished();
            return false;
        }
        else if(current.operation.equals("shutdown"))
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
