// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using Demo;

namespace bidir
{
    public partial class MainPage : UserControl
    {
        public MainPage()
        {
            InitializeComponent();
        }

        private void btnRunClick(object sender, RoutedEventArgs e)
        {
            string host = txtHost.Text;
            txtOutput.Text = "";
            btnRun.IsEnabled = false;
            btnStop.IsEnabled = true;

            Ice.InitializationData initData = new Ice.InitializationData();
            initData.dispatcher = delegate(System.Action action, Ice.Connection connection)
            {
                Dispatcher.BeginInvoke(action);
            };
            _communicator = Ice.Util.initialize(initData);

            CallbackSenderPrx server = CallbackSenderPrxHelper.uncheckedCast(
                _communicator.stringToProxy("sender:tcp -h " + host + " -p 4502"));
            if(server == null)
            {
                throw new ArgumentException("invalid proxy");
            }
            
            Ice.ObjectAdapter adapter = _communicator.createObjectAdapter("");
            Ice.Identity ident = new Ice.Identity();
            ident.name = Guid.NewGuid().ToString();
            ident.category = "";
            adapter.add(new CallbackReceiverI(this), ident);
            adapter.activate();

            //
            // Establish the connection to the server by sending a ping request with AMI.
            //
            server.begin_ice_ping().whenCompleted(
                () => 
                {
                    //
                    // Associate the object adapter to the connection to receive requests
                    // over the bi-directional connection.
                    //
                    server.ice_getCachedConnection().setAdapter(adapter);

                    //
                    // Notify the server that we are ready to receive callbacks.
                    //
                    server.begin_addClient(ident).whenCompleted(
                        () => 
                        { 
                            // Nothing to do. 
                        }, 
                        (Ice.Exception ex) =>
                        {
                            appendText(ex.ToString());
                        });
                }, 
                (Ice.Exception ex) =>
                {
                    appendText(ex.ToString());
                });
        }

        private void btnStopClick(object sender, RoutedEventArgs e)
        {
            btnRun.IsEnabled = false;
            btnStop.IsEnabled = false;

            System.Threading.Thread t = new System.Threading.Thread(() =>
            {
                try
                {
                    if (_communicator != null)
                    {
                        _communicator.destroy();
                        _communicator.waitForShutdown();
                    }
                }
                catch (Ice.CommunicatorDestroyedException)
                {
                }
                catch (System.Exception ex)
                {
                    Dispatcher.BeginInvoke(() => 
                    {
                        appendText(ex.ToString());
                    });
                }
                finally
                {
                    _communicator = null;
                    Dispatcher.BeginInvoke(() =>
                    {
                        btnRun.IsEnabled = true;
                        btnStop.IsEnabled = false;
                    });
                }
            });
            t.Start();
        }

        public void appendText(string text)
        {
            txtOutput.Text += text;
            txtOutput.SelectionLength = 0;
            txtOutput.SelectionStart = txtOutput.Text.Length;
        }

        private Ice.Communicator _communicator;
    }
}
