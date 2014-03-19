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
    /// Interaction logic for CancelDialog.xaml
    /// </summary>
    public partial class CancelDialog : Window
    {
        public CancelDialog()
        {
            InitializeComponent();
        }

        public void cancel(object sender, RoutedEventArgs args)
        {
            _cancel = true;
            Close();
        }

        public bool ShowModal()
        {
            ShowDialog();
            return _cancel;
        }

        private bool _cancel = false;
    }
}
