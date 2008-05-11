//
// From http://www.centerkey.com/java/browser
//

////////////////////////////////////////////////////////
// Bare Bones Browser Launch                          //
// Version 1.1                                        //
// July 8, 2005                                       //
// Supports: Mac OS X, GNU/Linux, Unix, Windows XP    //
// Example Usage:                                     //
//    String url = "http://www.centerkey.com/";       //
//    BareBonesBrowserLaunch.openURL(url);            //
// Public Domain Software -- Free to Use as You Like  //
////////////////////////////////////////////////////////

package IceGridGUI;

import java.lang.reflect.Method;
import javax.swing.JOptionPane;

public class BareBonesBrowserLaunch {

    private static final String errMsg = "Error attempting to launch web browser";

    public static void openURL(String url) {
        String osName = System.getProperty("os.name");
        try {
            if (osName.startsWith("Mac OS")) {
                Class macUtils = Class.forName("com.apple.mrj.MRJFileUtils");
                Method openURL = macUtils.getDeclaredMethod("openURL",
                                                            new Class[] {String.class});
                openURL.invoke(null, new Object[] {url});
            }
            else if (osName.startsWith("Windows"))
                Runtime.getRuntime().exec("rundll32 url.dll,FileProtocolHandler " + url);
            else { //assume Unix or Linux
                String[] browsers = {
                    "firefox", "opera", "konqueror", "mozilla", "netscape" };
                String browser = null;
                for (int count = 0; count < browsers.length && browser == null; count++)
                    if (Runtime.getRuntime().exec(
                            new String[] {"which", browsers[count]}).waitFor() == 0)
                        browser = browsers[count];
                if (browser == null)
                    throw new Exception("Could not find web browser.");
                else
                    Runtime.getRuntime().exec(new String[] {browser, url});
            }
        }
        catch (Exception e) {
            JOptionPane.showMessageDialog(null, errMsg + ":\n" + e.getLocalizedMessage());
        }
    }   
}
