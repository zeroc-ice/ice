// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using Demo;

public class Client
{
    private static void menu()
    {
        Console.Out.WriteLine("usage:\n"
	                       + "s: send byte sequence\n"
			       + "o: send byte sequence as oneway\n"
			       + "r: receive byte sequence\n"
			       + "e: echo (send and receive) byte sequence\n"
			       + "x: exit\n"
			       + "?: help\n");
    }
    
    private static int run(string[] args, Ice.Communicator communicator)
    {
        Ice.Properties properties = communicator.getProperties();
        string refProperty = "Throughput.Throughput";
        string r = properties.getProperty(refProperty);
        if(r.Length == 0)
        {
            Console.Error.WriteLine("property `" + r + "' not set");
            return 1;
        }
        
        Ice.ObjectPrx b = communicator.stringToProxy(r);
        ThroughputPrx throughput = ThroughputPrxHelper.checkedCast(b);
        if(throughput == null)
        {
            Console.Error.WriteLine("invalid proxy");
            return 1;
        }
        ThroughputPrx throughputOneway = ThroughputPrxHelper.uncheckedCast(throughput.ice_oneway());
        
        byte[] seq = new byte[seqSize.value];
        
        menu();
        
        string line = null;
        do 
        {
            try
            {
                Console.Out.Write("==> ");
                Console.Out.Flush();
                line = Console.In.ReadLine();
                if(line == null)
                {
                    break;
                }
                
                // Initial ping to setup the connection.
                throughput.ice_ping();
                
                long tmsec = System.DateTime.Now.Ticks / 10000;

                int repetitions = 100;
                
                if(line.Equals("s") || line.Equals("o") || line.Equals("r") || line.Equals("e"))
                {
                    char c = line[0];
                    
                    switch (c)
                    {                       
                        case 's': 
                        case 'o': 
                        {
                            Console.Out.Write("sending");
                            break;
                        }                                            
                        case 'r': 
                        {
                            Console.Out.Write("receiving");
                            break;
                        }                              
                        case 'e': 
                        {
                            Console.Out.Write("sending and receiving");
                            break;
                        }
                    }
                    
                    Console.Out.Write(" " + repetitions + " sequences of size " + seqSize.value);
                    
                    if(c == 'o')
                    {
                        Console.Out.Write(" as oneway");
                    }
                    
                    Console.Out.WriteLine("...");
                    
                    for (int i = 0; i < repetitions; ++i)
                    {
                        switch (c)
                        {
                            
                            case 's': 
                            {
                                throughput.sendByteSeq(seq);
                                break;
                            }
                            
                            
                            case 'o': 
                            {
                                throughputOneway.sendByteSeq(seq);
                                break;
                            }
                            
                            
                            case 'r': 
                            {
                                throughput.recvByteSeq();
                                break;
                            }
                            
                            
                            case 'e': 
                            {
                                throughput.echoByteSeq(seq);
                                break;
                            }
                            }
                    }
                    
                    double dmsec = System.DateTime.Now.Ticks / 10000 - tmsec;
                    Console.Out.WriteLine("time for " + repetitions + " sequences: " + dmsec.ToString("F") + "ms");
                    Console.Out.WriteLine("time per sequence: " + ((double)(dmsec / repetitions)).ToString("F") + "ms");
                    double mbit = repetitions * seqSize.value * 8.0 / dmsec / 1000.0;
                    if(c == 'e')
                    {
                        mbit *= 2;
                    }
                    Console.Out.WriteLine("throughput: " + mbit.ToString("F") + "MBit/s");
                }
                else if(line.Equals("x"))
                {
                    // Nothing to do
                }
                else if(line.Equals("?"))
                {
                    menu();
                }
                else
                {
                    Console.Out.WriteLine("unknown command `" + line + "'");
                    menu();
                }
            }
            catch(System.Exception ex)
            {
                Console.Error.WriteLine(ex);
            }
        }
        while(!line.Equals("x"));
        
        return 0;
    }
    
    public static void Main(string[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;
        
        try
        {
            Ice.Properties properties = Ice.Util.createProperties();
            properties.load("config");
            communicator = Ice.Util.initializeWithProperties(ref args, properties);
            status = run(args, communicator);
        }
        catch(System.Exception ex)
        {
            Console.Error.WriteLine(ex);
            status = 1;
        }
        
	if(communicator != null)
	{
	    try
	    {
		communicator.destroy();
	    }
	    catch(System.Exception ex)
	    {
		Console.Error.WriteLine(ex);
		status = 1;
	    }
	}
        
        System.Environment.Exit(status);
    }
}
