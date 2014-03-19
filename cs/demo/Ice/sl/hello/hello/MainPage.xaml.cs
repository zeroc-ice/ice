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
using System.Collections.ObjectModel;
using Demo;

namespace hello
{
    public partial class MainPage : UserControl
    {
        public MainPage()
        {
            InitializeComponent();
            ObservableCollection<string> modes = new ObservableCollection<string>();
            modes.Add("Twoway");
            modes.Add("Oneway");
            modes.Add("Batch Oneway");
            cmbModes.DataContext = modes;
            cmbModes.SelectedIndex = 0;
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.dispatcher = delegate(System.Action action, Ice.Connection connection)
            {
                Dispatcher.BeginInvoke(action);
            };
            _communicator = Ice.Util.initialize(initData);
        }

        private void timeoutValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            _timeout = (int)Math.Round(e.NewValue * 1000, 0);
            txtTimeout.Text = _timeout.ToString() + " ms";
        }

        private void delayValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            _delay = (int)Math.Round(e.NewValue * 1000, 0);
            txtDelay.Text = _delay.ToString() + " ms";
        }

        private void btnSayHelloClick(object sender, RoutedEventArgs e)
        {
            int mode = cmbModes.SelectedIndex;
            string host = txtHost.Text;
            txtOutput.Text = "";
            try
            {
                Ice.ObjectPrx proxy = _communicator.stringToProxy("hello:tcp -h " + host + " -p 4502");
                switch (mode)
                {
                case 0:
                {
                    proxy = proxy.ice_twoway();
                    break;
                }
                case 1:
                {
                    proxy = proxy.ice_oneway();
                    break;
                }
                case 2:
                {
                    proxy = proxy.ice_batchOneway();
                    btnFlush.IsEnabled = true;
                    break;
                }
                default:
                {
                    throw new ArgumentException("Invalid Mode index " + mode.ToString());
                }
                }

                if (_timeout == 0)
                {
                    proxy = proxy.ice_timeout(-1);
                }
                else
                {
                    proxy = proxy.ice_timeout(_timeout);
                }
                HelloPrx hello = HelloPrxHelper.uncheckedCast(proxy);
                if(mode != 2)
                {
                    bool response = false;
                    Ice.AsyncResult result = hello.begin_sayHello(_delay).whenCompleted(
                        () => 
                        { 
                            response = true;
                            txtOutput.Text = "Ready";
                        },
                        (Ice.Exception ex) =>
                        {
                            response = true;
                            txtOutput.Text = ex.ToString();
                        }).whenSent(
                            (bool sentSynchronously) => 
                            {
                                if(response)
                                {
                                    return; // Response was received already.                                    
                                }

                                if(mode == 0)
                                {
                                    txtOutput.Text = "Wait for response";
                                }
                                else
                                {
                                    txtOutput.Text = "Ready";
                                }
                            });
                    if(!result.sentSynchronously())
                    {
                        txtOutput.Text = "Sending request";
                    }
                }
                else
                {
                    hello.sayHello(_delay);
                    txtOutput.Text = "Queued hello request";
                }
            }
            catch(System.Exception ex)
            {
                txtOutput.Text = ex.ToString();
            }
        }

        private void btnFlushClick(object sender, RoutedEventArgs e)
        {
            _communicator.begin_flushBatchRequests().whenCompleted(
                (Ice.Exception ex) =>
                {
                    txtOutput.Text = ex.ToString();
                }).whenSent(
                    (bool sentSynchronously) => 
                    {
                        Dispatcher.BeginInvoke(delegate() 
                                        {
                                            btnFlush.IsEnabled = false;
                                            txtOutput.Text = "Flushed batch requests";
                                        });
                    });
        }

        private void btnShutdownClick(object sender, RoutedEventArgs e)
        {
            string host = txtHost.Text;
            txtOutput.Text = "Shutting down...";

            Ice.ObjectPrx proxy = _communicator.stringToProxy("hello:tcp -h " + host + " -p 4502");
            HelloPrx hello = HelloPrxHelper.uncheckedCast(proxy);
            hello.begin_shutdown().whenCompleted(
                () =>
                {
                    txtOutput.Text = "Ready";
                },
                (Ice.Exception ex) => 
                { 
                    txtOutput.Text = ex.ToString();
                });
        }

        private int _timeout = 0;
        private int _delay = 0;
        private Ice.Communicator _communicator;
    }
}
