// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System; 
using System.IO; 
using System.Net; 
using System.Net.Sockets;
using System.Threading;
using System.Text;

namespace PolicyService
{

//
// This class implements a simple policy server for
// Silverlight.
//
public sealed class PolicyServer : IDisposable
{
    public PolicyServer(string ipAddress, string policyFile)
    {
        _ipAddress = ipAddress;
        _policyFile = policyFile;
    }

    public void Start()
    {
        IPAddress address = null;
        try
        {
            address = IPAddress.Parse(_ipAddress);
        }
        catch(System.FormatException ex)
        {
            Console.WriteLine("Invalid IP address format: " + _ipAddress);
            Console.WriteLine(ex.ToString());
            return;
        }
        // Read policy file
        try
        {
            _policyBytes = File.ReadAllBytes(_policyFile);
        }
        catch(System.IO.IOException ex)
        {
            Console.WriteLine("Error reading policy file: " + _policyFile);
            Console.WriteLine(ex.ToString());
            return;
        }
        _policyListener = new TcpListener(address, 943);
        try
        {
            // Listen for policy requests
            _policyListener.Start();
        }
        catch(SocketException ex)
        {
            Console.WriteLine("Error starting Policy Server:\n" + ex.ToString());
            return;
        }
        Console.WriteLine("Policy Server started...");
        // Start policy response thread
        Thread policyThread = new Thread(ServePolicy);
        policyThread.IsBackground = true;
        policyThread.Start();
        policyThread.Join();
    }

    private void ServePolicy()
    {
        while(true)
        {
            Console.WriteLine("Accepting Policy Requests...");
            using(Socket client = _policyListener.AcceptSocket())
            {
                Console.WriteLine("Policy Request Accepted...");

                // Get policy request header
                byte[] buffer = new byte[1024];
                int bytesReceived = client.Receive(buffer);

                // Basic check of request header
                string header = Encoding.UTF8.GetString(buffer, 0, bytesReceived);
                if(header == "<policy-file-request/>")
                {
                    client.Send(_policyBytes, 0, _policyBytes.Length, SocketFlags.None);
                }
            }
        }
    }

    public void Dispose()
    {
    }

    private string _ipAddress = "";
    private string _policyFile = "";
    private byte[] _policyBytes = null; // byte array used to store the response in memory.
    private TcpListener _policyListener = null;
}

class Program
{
    static void Main(string[] args)
    {
        if(args.Length == 2)
        {
            // Start our Policy Service
            using (PolicyServer server = new PolicyServer(args[0], args[1]))
            {
                server.Start();
            }
        }
        else
        {
            Console.WriteLine("Usage: PolicyServer <ip-address> <policy-file>");
        }
    }
}

}
