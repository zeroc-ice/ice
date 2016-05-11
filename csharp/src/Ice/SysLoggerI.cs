// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
        public SysLoggerI(string prefix, string facilityString)
        {
            int facility;
            if(facilityString.Equals("LOG_KERN"))
            {
                facility = LOG_KERN;
            }
            else if(facilityString.Equals("LOG_USER"))
            {
                facility = LOG_USER;
            }
            else if(facilityString.Equals("LOG_MAIL"))
            {
                facility = LOG_MAIL;
            }
            else if(facilityString.Equals("LOG_DAEMON"))
            {
                facility = LOG_DAEMON;
            }
            else if(facilityString.Equals("LOG_AUTH"))
            {
                facility = LOG_AUTH;
            }
            else if(facilityString.Equals("LOG_SYSLOG"))
            {
                facility = LOG_SYSLOG;
            }
            else if(facilityString.Equals("LOG_LPR"))
            {
                facility = LOG_LPR;
            }
            else if(facilityString.Equals("LOG_NEWS"))
            {
                facility = LOG_NEWS;
            }
            else if(facilityString.Equals("LOG_UUCP"))
            {
                facility = LOG_UUCP;
            }
            else if(facilityString.Equals("LOG_CRON"))
            {
                facility = LOG_CRON;
            }
            else if(facilityString.Equals("LOG_AUTHPRIV"))
            {
                facility = LOG_AUTHPRIV;
            }
            else if(facilityString.Equals("LOG_FTP"))
            {
                facility = LOG_FTP;
            }
            else if(facilityString.Equals("LOG_LOCAL0"))
            {
                facility = LOG_LOCAL0;
            }
            else if(facilityString.Equals("LOG_LOCAL1"))
            {
                facility = LOG_LOCAL1;
            }
            else if(facilityString.Equals("LOG_LOCAL2"))
            {
                facility = LOG_LOCAL2;
            }
            else if(facilityString.Equals("LOG_LOCAL3"))
            {
                facility = LOG_LOCAL3;
            }
            else if(facilityString.Equals("LOG_LOCAL4"))
            {
                facility = LOG_LOCAL4;
            }
            else if(facilityString.Equals("LOG_LOCAL5"))
            {
                facility = LOG_LOCAL5;
            }
            else if(facilityString.Equals("LOG_LOCAL6"))
            {
                facility = LOG_LOCAL6;
            }
            else if(facilityString.Equals("LOG_LOCAL7"))
            {
                facility = LOG_LOCAL7;
            }
            else
            {
                throw new Ice.InitializationException("Invalid value for Ice.SyslogFacility: " + facilityString);
            }
            initialize(prefix, facility);
        }
        
        private SysLoggerI(string prefix, int facility)
        {
            initialize(prefix, facility);
        }

        private void initialize(string prefix, int facility)
        {
            _prefix = prefix;
            _facility = facility;
            
            //
            // Open a datagram socket to communicate with the localhost
            // syslog daemon.
            // 
            try
            {
                _host = ((System.Net.IPEndPoint)IceInternal.Network.getAddressForServer(
                             System.Net.Dns.GetHostName(), _port, IceInternal.Network.EnableBoth, false)).Address;
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

        public string getPrefix()
        {
            return _prefix;
        }

        public Logger cloneWithPrefix(string prefix)
        {
            return new SysLoggerI(prefix, _facility);
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
                
                int priority = (_facility << 3) | severity;
                
                string msg = '<' + priority + '>' + _prefix + ": " + message;
                
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
        
        private string _prefix;
        private int _facility;
        private UdpClient _socket;
        private System.Net.IPAddress _host;
        private static int _port = 514;
        
        //
        // Syslog facilities (as defined in syslog.h)
        // 
        private const int LOG_KERN = 0;
        private const int LOG_USER = 1;
        private const int LOG_MAIL = 2;
        private const int LOG_DAEMON = 3;
        private const int LOG_AUTH = 4;
        private const int LOG_SYSLOG = 5;
        private const int LOG_LPR = 6;
        private const int LOG_NEWS = 7;
        private const int LOG_UUCP = 8;
        private const int LOG_CRON = 9;
        private const int LOG_AUTHPRIV = 10;
        private const int LOG_FTP = 11;
        private const int LOG_LOCAL0 = 16;
        private const int LOG_LOCAL1 = 17;
        private const int LOG_LOCAL2 = 18;
        private const int LOG_LOCAL3 = 19;
        private const int LOG_LOCAL4 = 20;
        private const int LOG_LOCAL5 = 21;
        private const int LOG_LOCAL6 = 22;
        private const int LOG_LOCAL7 = 23;
        
        //
        // Syslog priorities (as defined in syslog.h)
        // 
        private const int LOG_ERR = 3;
        private const int LOG_WARNING = 4;
        private const int LOG_INFO = 6;
    }

}
