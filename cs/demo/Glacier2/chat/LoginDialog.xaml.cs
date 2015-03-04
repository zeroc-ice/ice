// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace Glacier2.chat.client
{
    /// <summary>
    /// Interaction logic for LoginDialog.xaml
    /// </summary>
    public partial class LoginDialog : Window
    {
        public LoginDialog(LoginData loginData)
        {
            InitializeComponent();
            _loginData = loginData;
            txtHost.Text = _loginData.routerHost;
            txtUsername.Text = _loginData.userName;
            txtPassword.Password = _loginData.password;
        }

        public void login(object sender, RoutedEventArgs e)
        {
            _loginData.routerHost = txtHost.Text;
            _loginData.userName = txtUsername.Text;
            _loginData.password = txtPassword.Password;
            _cancel = false;
            Close();
        }

        public void cancel(object sender, RoutedEventArgs args)
        {
            _cancel = true;
            Close();
        }

        public bool ShowModal()
        {
            ShowDialog();
            return !_cancel;
        }

        private bool _cancel = true;
        private LoginData _loginData;
    }
}
