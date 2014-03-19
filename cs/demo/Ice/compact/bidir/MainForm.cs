// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Linq;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Threading;

namespace server
{
    public partial class MainForm : Form
    {
        public MainForm()
        {
            InitializeComponent();
        }

        protected override void OnClosed(EventArgs e)
        {
            base.OnClosed(e);
            stop();
        }

        private void btnStart_Click(object sender, EventArgs e)
        {
            try
            {
                Ice.InitializationData initData = new Ice.InitializationData();
                initData.properties = Ice.Util.createProperties();
                initData.properties.setProperty("Callback.Server.Endpoints", "tcp -h 0.0.0.0 -p " + txtPort.Text);
                _communicator = Ice.Util.initialize(initData);

                Ice.ObjectAdapter adapter = _communicator.createObjectAdapter("Callback.Server");
                _sender = new CallbackSenderI(_communicator);
                adapter.add(_sender, _communicator.stringToIdentity("sender"));
                adapter.activate();

                _thread = new Thread(new ThreadStart(_sender.Run));
                _thread.Start();
                btnStart.Enabled = false;
                btnStop.Enabled = true;
            }
            catch(Exception ex)
            {
                lblStatus.Text = "Start Failed: " + Environment.NewLine + ex.ToString();
            }
            lblStatus.Text = "Server started ok";
        }

        private void btnStop_Click(object sender, EventArgs e)
        {
            stop();
        }

        private void stop()
        {
            try
            {
                if(_communicator == null)
                {
                    return;
                }
                _communicator.destroy();
                _communicator = null;

                if(_sender != null)
                {
                    _sender.destroy();
                }

                if(_thread != null)
                {
                    _thread.Join();
                }
            }
            catch(Exception ex)
            {
                lblStatus.Text = "Stop Failed: " + Environment.NewLine + ex.ToString();
            }
            finally
            {
                btnStart.Enabled = true;
                btnStop.Enabled = false;
            }
            lblStatus.Text = "Server stoped ok";
        }

        private Ice.Communicator _communicator;
        private CallbackSenderI _sender;
        private Thread _thread;
    }
}