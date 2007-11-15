// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Net.Sockets;

namespace Ice
{

    public sealed class SysLoggerI :  Logger
    {
        public SysLoggerI(string ident)
        {
            _ident = ident;
            
            //
            // Open a datagram socket to communicate with the localhost
            // syslog daemon.
            // 
            try
            {
                _host = IceInternal.Network.getAddress(System.Net.Dns.GetHostName(), _port).Address;
                _socket = new UdpClient();
                _socket.Connect(_host, _port);
            }
            catch(System.Exception ex)
            {
                throw new Ice.DNSException(ex);
            }
        }
        
        public void print(string message)
        {
            log(LOG_INFO, message);
        }

        public void trace(string category, string message)
        {
            log(LOG_INFO, category + ": " + message);
        }
        
        public void warning(string message)
        {
            log(LOG_WARNING, message);
        }
        
        public void error(string message)
        {
            log(LOG_ERR, message);
        }
        
        private void log(int severity, string message)
        {
            try
            {
                //
                // Create a syslog message as defined by the RFC 3164:
                // <PRI>HEADER MSG. PRI is the priority and is calculated
                // from the facility and the severity. We don't specify
                // the HEADER. MSG contains the identifier followed by a
                // colon character and the message.
                //
                
                int priority = (LOG_USER << 3) | severity;
                
                string msg = '<' + priority + '>' + _ident + ": " + message;
                
                byte[] buf = new byte[msg.Length];
                for(int i = 0; i < msg.Length; i++)
                {
                    buf[i] = (byte)msg[i];
                }
                _socket.Send(buf, buf.Length);
            }
            catch(System.IO.IOException ex)
            {
                Ice.SocketException se = new Ice.SocketException(ex);
                throw se;
            }
        }
        
        private string _ident;
        private UdpClient _socket;
        private System.Net.IPAddress _host;
        private static int _port = 514;
        
        //
        // Syslog facilities facilities (as defined in syslog.h)
        // 
        private static readonly int LOG_USER = 1;
        
        //
        // Syslog priorities (as defined in syslog.h)
        // 
        private static readonly int LOG_ERR = 3;
        private static readonly int LOG_WARNING = 4;
        private static readonly int LOG_INFO = 6;
    }

}
