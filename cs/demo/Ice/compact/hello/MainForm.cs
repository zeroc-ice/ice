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

namespace client
{
    public partial class MainForm : Form
    {
        public MainForm()
        {
            InitializeComponent();
            try
            {
                Ice.InitializationData initData = new Ice.InitializationData();
                initData.properties = Ice.Util.createProperties();
                initData.dispatcher = delegate(Ice.VoidAction action, Ice.Connection connection)
                    {
                        this.BeginInvoke(action);
                    };
                _communicator = Ice.Util.initialize(initData);
            }
            catch(Ice.LocalException ex)
            {
                lblStatus.Text = "Exception: " + ex.ice_name();
            }
        }

        protected override void OnClosed(EventArgs e)
        {
            base.OnClosed(e);
            if(_communicator == null)
            {
                return;
            }
            _communicator.destroy();
            _communicator = null;
        }

        class SayHelloCB
        {
            public SayHelloCB(MainForm mainForm)
            {
                _mainForm = mainForm;
            }

            public void response()
            {
                lock(this)
                {
                    _response = true;
                    _mainForm.lblStatus.Text = "Ready";
                }
            }

            public void sent(bool sentSynchronously)
            {
                lock(this)
                {
                    if(_response)
                    {
                        return;
                    }
                    _mainForm.lblStatus.Text = "Waiting for response";
                }
            }

            public void exception(Ice.Exception ex)
            {
                lock(this)
                {
                    _response = true;
                    _mainForm.lblStatus.Text = "Exception: " + ex.ice_name();
                }
            }

            private MainForm _mainForm;
            private bool _response = false;
        }

        private Demo.HelloPrx createProxy()
        {
            string proxyString = "hello:tcp -p 10000";
            if (!String.IsNullOrEmpty(hostname.Text))
            {
                proxyString += " -h " + hostname.Text;
            }

            Ice.ObjectPrx proxy = _communicator.stringToProxy(proxyString);
            return Demo.HelloPrxHelper.uncheckedCast(proxy);
        }

        private void btnHello_Click(object sender, EventArgs e)
        {
            Demo.HelloPrx hello = createProxy();
            lblStatus.Text = "Sending request";
            SayHelloCB cb = new SayHelloCB(this);
            hello.begin_sayHello(0).whenCompleted(cb.response, cb.exception).whenSent(cb.sent);
        }

        private void btnShutdown_Click(object sender, EventArgs e)
        {
            Demo.HelloPrx hello = createProxy();
            lblStatus.Text = "Shutting down server";
            SayHelloCB cb = new SayHelloCB(this);
            hello.begin_shutdown().whenCompleted(cb.response, cb.exception).whenSent(cb.sent);
        }

        private Ice.Communicator _communicator;
    }
}
