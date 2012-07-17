//
// LoginView.xaml.cpp
// Implementation of the LoginView class
//

#include "pch.h"
#include "LoginView.xaml.h"
#include "MainPage.xaml.h"

using namespace chat;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

LoginView::LoginView()
{
    InitializeComponent();
    MainPage::instance()->_loginView = this;
}

void
LoginView::setError(String^ err)
{
    signin->IsEnabled = true;
    errorBorder->Visibility = Windows::UI::Xaml::Visibility::Visible;
    error->Text = err;
}

/// <summary>
/// Invoked when this page is about to be displayed in a Frame.
/// </summary>
/// <param name="e">Event data that describes how this page was reached.  The Parameter
/// property is typically used to configure the page.</param>
void LoginView::OnNavigatedTo(NavigationEventArgs^ e)
{
    LoginData loginData = MainPage::instance()->coordinator()->loginData();
    if(!loginData.hostname.empty())
    {
        hostname->Text = ref new String(IceUtil::stringToWstring(loginData.hostname).c_str());
    }
    if(!loginData.username.empty())
    {
        username->Text = ref new String(IceUtil::stringToWstring(loginData.username).c_str());
    }
    if(!loginData.password.empty())
    {
        password->Password = ref new String(IceUtil::stringToWstring(loginData.password).c_str());
    }
    (void) e;	// Unused parameter
}

void chat::LoginView::signinClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    LoginData loginData;
    loginData.hostname = IceUtil::wstringToString(hostname->Text->Data());
    loginData.username = IceUtil::wstringToString(username->Text->Data());
    loginData.password = IceUtil::wstringToString(password->Password->Data());
    signin->IsEnabled = false;
    error->Text = "";
    errorBorder->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    MainPage::instance()->coordinator()->signIn(loginData);
}
