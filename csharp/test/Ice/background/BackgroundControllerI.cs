//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

namespace ZeroC.Ice.Test.Background
{
    internal class BackgroundController : IBackgroundController
    {
        public void pauseCall(string opName, Current current)
        {
            lock (this)
            {
                _pausedCalls.Add(opName);
            }
        }

        public void resumeCall(string opName, Current current)
        {
            lock (this)
            {
                _pausedCalls.Remove(opName);
                System.Threading.Monitor.PulseAll(this);
            }
        }

        internal void checkCallPause(Current current)
        {
            lock (this)
            {
                while (_pausedCalls.Contains(current.Operation))
                {
                    System.Threading.Monitor.Wait(this);
                    break;
                }
            }
        }

        public void initializeException(bool enable, Current current)
        {
            _configuration.InitializeException(enable ? new TransportException("") : null);
        }

        public void readReady(bool enable, Current current)
        {
            _configuration.ReadReady(enable);
        }

        public void readException(bool enable, Current current)
        {
            _configuration.ReadException(enable ? new TransportException("") : null);
        }

        public void writeReady(bool enable, Current current)
        {
            _configuration.WriteReady(enable);
        }

        public void writeException(bool enable, Current current)
        {
            _configuration.WriteException(enable ? new TransportException("") : null);
        }

        public void buffered(bool enable, Current current)
        {
            _configuration.Buffered(enable);
        }

        internal BackgroundController(ObjectAdapter adapter)
        {
            _adapter = adapter;
            _configuration = Configuration.GetInstance();
        }
        private ObjectAdapter _adapter;
        private Configuration _configuration;
        private HashSet<string> _pausedCalls = new HashSet<string>();
    }
}
