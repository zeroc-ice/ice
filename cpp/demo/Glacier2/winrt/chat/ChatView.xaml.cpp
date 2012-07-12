//
// ChatView.xaml.cpp
// Implementation of the ChatView class
//

#include "pch.h"
#include "ChatView.xaml.h"
#include "MainPage.xaml.h"
#include <string>

using namespace chat;
using namespace std;
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

ChatView::ChatView()
{
    InitializeComponent();
    MainPage::instance()->_chatView = this;
}

void
ChatView::setError(String^ err)
{
    appendMessage(L"<system-message> " + err);
}

void 
ChatView::appendMessage(String^ message)
{
    messages->Text += message + L"\n";
    messages->UpdateLayout();
    Scroller->ScrollToVerticalOffset(Scroller->ScrollableHeight);
}

/// <summary>
/// Invoked when this page is about to be displayed in a Frame.
/// </summary>
/// <param name="e">Event data that describes how this page was reached.  The Parameter
/// property is typically used to configure the page.</param>
void ChatView::OnNavigatedTo(NavigationEventArgs^ e)
{
    (void) e;	// Unused parameter
}


void chat::ChatView::inputKeyDown(Platform::Object^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs^ e)
{
    if(e->Key == Windows::System::VirtualKey::Enter && !input->Text->IsEmpty())
    {
        string msg = IceUtil::wstringToString(input->Text->Data());
        input->Text = ref new String();
        MainPage::instance()->coordinator()->say(msg);
    }
}
