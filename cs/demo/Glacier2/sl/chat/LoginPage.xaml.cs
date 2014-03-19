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
using System.Windows.Navigation;

namespace chat
{
    public partial class LoginPage : Page
    {
        public LoginPage()
        {
            InitializeComponent();
            
            Coordinator.LoginData loginData = new Coordinator.LoginData();
            txtHost.Text = loginData.host;
            txtUsername.Text = loginData.username;

            //
            // Initialize values from previous page.
            //
            LoginPage previous = Coordinator.getInstance().getLoginPage();
            if(previous != null)
            {
                txtHost.Text = previous.txtHost.Text;
                txtUsername.Text = previous.txtUsername.Text;
                txtPassword.Password = previous.txtPassword.Password;
                txtError.Text = previous.txtError.Text;
            }

            if (String.IsNullOrEmpty(txtError.Text))
            {
                txtError.Visibility = System.Windows.Visibility.Collapsed;
            }
            else
            {
                txtError.Visibility = System.Windows.Visibility.Visible;
            }
            Coordinator.getInstance().setLoginPage(this);
        }

        string PageTitle { get{return "Login";} }

        bool KeepAlive { get{return true;} }

        bool CanNavigateAway { get{return true;} }

        private void btnLoginClick(object sender, RoutedEventArgs e)
        {
            Coordinator.LoginData loginData = new Coordinator.LoginData();
            loginData.username = txtUsername.Text;
            loginData.password = txtPassword.Password;
            loginData.host = txtHost.Text;
            Coordinator.getInstance().doLogin(loginData);
        }
    }
}
