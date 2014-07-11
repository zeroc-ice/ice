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
                initData.dispatcher = (Ice.VoidAction action, Ice.Connection connection) =>
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

        private void response(ref bool haveResponse)
        {
            haveResponse = true;
            lblStatus.Text = "Ready";
        }

        private void sent(ref bool haveResponse, bool sentSynchronously)
        {
            if(haveResponse)
            {
                return;
            }
            lblStatus.Text = "Waiting for response";
        }

        private void exception(ref bool haveResponse, Ice.Exception ex)
        {
            haveResponse = true;
            lblStatus.Text = "Exception: " + ex.ice_name();
        }

        private void btnHello_Click(object sender, EventArgs e)
        {
            Demo.HelloPrx hello = createProxy();
            lblStatus.Text = "Sending request";
            bool haveResponse = false;
            hello.begin_sayHello(0).whenCompleted(
                () =>
                {
                    response(ref haveResponse);
                },
                (Ice.Exception ex) =>
                {
                    exception(ref haveResponse);
                }
            ).whenSent(
                (bool sentSynchronously) =>
                {
                    sent(ref haveResponse, sentSynchronously);
                });
        }

        private void btnShutdown_Click(object sender, EventArgs e)
        {
            Demo.HelloPrx hello = createProxy();
            lblStatus.Text = "Shutting down server";
            bool haveResponse = false;
            hello.begin_shutdown().whenCompleted(
                () =>
                {
                    response(ref haveResponse);
                },
                (Ice.Exception ex) =>
                {
                    exception(ref haveResponse);
                }
            ).whenSent(
                (bool sentSynchronously) =>
                {
                    sent(ref haveResponse, sentSynchronously);
                });
        }

        private Ice.Communicator _communicator;
    }
}
