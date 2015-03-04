#
# This ProGuard configuration file illustrates how to process applications.
# Usage:
#     java -jar proguard.jar @applications.pro
#

# Specify the input jars, output jars, and library jars.

# Preserve all public applications.

-keepclasseswithmembers public class * {
    public static void main(java.lang.String[]);
}

# Following options are useful for debugging. 
# -printseeds
# -verbose

# Preserve all annotations.

-keepattributes *Annotation*

# Parse non-public libraries to prevent undefined symbol errors on some platforms.

-dontskipnonpubliclibraryclasses

# Preserve all native method names and the names of their classes.

-keepclasseswithmembernames class * {
    native <methods>;
}

# Preserve a method that is required in all enumeration classes.

-keepclassmembers class * extends java.lang.Enum {
    public **[] values();
}

-dontusemixedcaseclassnames
-dontoptimize
-dontobfuscate

-keepclassmembers class * implements java.io.Serializable {
    static final long serialVersionUID;
    private void writeObject(java.io.ObjectOutputStream);
    private void readObject(java.io.ObjectInputStream);
    java.lang.Object writeReplace();
    java.lang.Object readResolve();
}

# Your application may contain more items that need to be preserved; 
# typically classes that are dynamically created using Class.forName:

-keep public class Ice.** {
  public *;
}
-keep public class IceGrid.** {
  public *;
}
-keep public class IceSSL.** {
  public *;
}
-keep interface IceGrid.**
-keep class com.jgoodies.looks.plastic.PlasticXPLookAndFeel
-keep class com.jgoodies.looks.plastic.PlasticFieldCaret
-keep class com.jgoodies.looks.windows.WindowsFieldCaret 
-keep public class javax.**
-keep interface javax.**
-keep class * implements javax.**
-keep class IceMX.**
-keep interface IceMX.**

#
# Without this directive, the menus on Windows don't look good
#
-keepclassmembers class com.jgoodies.looks.** {
   <methods>;
}

#
# More keeps to suppress Notes
#
-keep public class * {
    public protected *;
}

-keep public class Ice.DispatchStatus {
    int __value;
    Ice.DispatchStatus[] __values;
    int value();
}

-keep public class javax.swing.plaf.metal.MetalLookAndFeel {
    javax.swing.plaf.metal.MetalTheme getCurrentTheme();
}

-keep public class IceGridGUI.BareBonesBrowserLaunch {
    void openURL(java.lang.String);
}

-keep public class java.net.DatagramSocketImpl {
    java.io.FileDescriptor fd;
}
