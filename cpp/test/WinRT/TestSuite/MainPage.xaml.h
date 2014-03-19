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
#include <collection.h>

namespace TestSuite
{
/// <summary>
/// An empty page that can be used on its own or navigated to within a Frame.
/// </summary>
[Windows::Foundation::Metadata::WebHostHidden]
public ref class MainPage sealed
{
public:
    
    MainPage();
    
    void completed();
    void failed(Platform::String^ reason);

protected:
        
    virtual void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;
    
private:

    void btnRun_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void runSelectedTest();
    Platform::Collections::Vector<Platform::String^>^ _names;
};

}
