//
// LoginView.xaml.h
// Declaration of the LoginView class
//

#pragma once

#include "LoginView.g.h"

namespace chat
{
    ref class MainPage;

    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public ref class LoginView sealed
    {
    public:

        LoginView();

        void setError(Platform::String^ err);

    protected:

        virtual void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;

    private:

        void signinClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

        friend ref class MainPage;
    };
}
