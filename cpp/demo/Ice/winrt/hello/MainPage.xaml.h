// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#include "MainPage.g.h"
#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <Hello.h>

namespace hello
{

public ref class MainPage sealed
{
public:

    MainPage();

private:

    Demo::HelloPrx proxy();
    bool isBatch();
    void print(const std::string&);

    void hello_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void shutdown_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void flush_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

    Ice::CommunicatorPtr _communicator;
    bool _response;
};

}
