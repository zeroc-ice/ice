// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
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
            updateProxy();
        }

        private void updateProxy()
        {
            string host = txtHost.Text;
            if(host.Length == 0)
            {
                txtOutput.Text = "Host is empty";
                _helloPrx = null;
                return;
            }
            txtOutput.Text = "";
            try
            {
                Ice.ObjectPrx proxy = _communicator.stringToProxy("hello:tcp -h " + host + " -p 4502");
                switch (_mode)
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
                            throw new ArgumentException("Invalid Mode index " + _mode.ToString());
                        }
                }

                if (_timeout == 0)
                {
                    proxy = proxy.ice_invocationTimeout(-1);
                }
                else
                {
                    proxy = proxy.ice_invocationTimeout(_timeout);
                }

                //
                // The batch requests associated to the proxy are lost when we
                // update the proxy.
                //
                btnFlush.IsEnabled = false;

                _helloPrx = HelloPrxHelper.uncheckedCast(proxy);
            }
            catch (System.Exception ex)
            {
                txtOutput.Text = ex.ToString();
            }
        }

        private void timeoutValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            _timeout = (int)Math.Round(e.NewValue * 1000, 0);
            txtTimeout.Text = _timeout.ToString() + " ms";
            updateProxy();
        }

        private void delayValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            _delay = (int)Math.Round(e.NewValue * 1000, 0);
            txtDelay.Text = _delay.ToString() + " ms";
        }

        private void modeSelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            _mode = cmbModes.SelectedIndex;
            updateProxy();
        }

        private void hostTextChanged(object sender, TextChangedEventArgs e)
        {
            if(txtHost.Text.Length == 0)
            {
                btnSayHello.IsEnabled = false;
                btnShutdown.IsEnabled = false;
                btnFlush.IsEnabled = false;
            }
            else
            {
                btnSayHello.IsEnabled = true;
                btnShutdown.IsEnabled = true;
                btnFlush.IsEnabled = false;
            }
            updateProxy();
        }

        private void btnSayHelloClick(object sender, RoutedEventArgs e)
        {
            try
            {
                if(_helloPrx == null)
                {
                    return;
                }
                if(_mode != 2)
                {
                    bool response = false;
                    Ice.AsyncResult result = _helloPrx.begin_sayHello(_delay).whenCompleted(
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

                                if(_mode == 0)
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
                    btnFlush.IsEnabled = true;
                    _helloPrx.sayHello(_delay);
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
            if(_helloPrx == null)
            {
                return;
            }
            _helloPrx.begin_ice_flushBatchRequests().whenCompleted(
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
            if(_helloPrx == null)
            {
                return;
            }

            try
            {
                if(!_helloPrx.ice_isBatchOneway())
                {
                    txtOutput.Text = "Shutting down...";
                    _helloPrx.begin_shutdown().whenCompleted(
                        () =>
                        {
                            txtOutput.Text = "Ready";
                        },
                        (Ice.Exception ex) =>
                        {
                            txtOutput.Text = ex.ToString();
                        });
                }
                else
                {
                    btnFlush.IsEnabled = true;
                    _helloPrx.shutdown();
                    txtOutput.Text = "Queued shutdown request";
                }
            }
            catch(System.Exception ex)
            {
                txtOutput.Text = ex.ToString();
            }
        }
        private int _timeout = 0;
        private int _delay = 0;
        private int _mode = 0;
        private Ice.Communicator _communicator;
        private Demo.HelloPrx _helloPrx;
    }
}
