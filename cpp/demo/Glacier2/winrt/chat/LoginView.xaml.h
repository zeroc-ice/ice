// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#include "LoginView.g.h"

namespace chat
{

public ref class LoginView sealed
{
public:

    LoginView();
    void setError(Platform::String^);
    void signinCompleted()
    {
        signin->IsEnabled = true;
    }

protected:

    virtual void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^) override;

private:

    void signinClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^);
};

}
