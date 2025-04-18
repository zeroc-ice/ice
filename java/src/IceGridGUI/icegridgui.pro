# Copyright (c) ZeroC, Inc.

# Preserve all public applications.

-keepclasseswithmembers public class * {
    public static void main(java.lang.String[]);
}

# Following options are useful for debugging.
# -printseeds
# -verbose

-dontnote jdk.internal.jimage.**
-dontnote jdk.internal.jrtfs.**
-dontnote module-info

#
# This classes are not longer part of JDK 10 on non Windows platforms
# and will cause a build error
#
-dontwarn com.jgoodies.looks.windows.*

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

-dontnote com.zeroc.Ice.CommunicatorObserverI*
-dontnote com.zeroc.Ice.ClassSliceLoader
-dontnote com.zeroc.Ice.DefaultPackageSliceLoader
-dontnote com.zeroc.Ice.ModuleToPackageSliceLoader
-dontnote com.zeroc.Ice.InvocationObserverI*
-dontnote com.zeroc.Ice.MetricsMap
-dontnote com.zeroc.Ice.ObjectPrx
-dontnote com.zeroc.Ice.PluginManagerI
-dontnote com.zeroc.Ice.IceMX.Observer
-dontnote com.zeroc.Ice.IceMX.ObserverFactory

-dontnote com.zeroc.IceBox.ServiceManagerI
-dontnote com.zeroc.IceGridGUI.Coordinator
-dontnote com.zeroc.IceGridGUI.LiveDeployment.MetricsView*

-dontnote com.jgoodies.**
-dontnote com.javafx.**
-dontnote com.sun.**

-dontwarn sun.misc.Unsafe

-dontusemixedcaseclassnames
-dontoptimize
-dontobfuscate

-keepclassmembers class * implements java.io.Serializable {
    private static final long serialVersionUID;
    private void writeObject(java.io.ObjectOutputStream);
    private void readObject(java.io.ObjectInputStream);
    java.lang.Object writeReplace();
    java.lang.Object readResolve();
}

-keepclass class * {
    java.lang.String ice_staticId();
}

-keepclass class com.zeroc.Ice.* {
    java.lang.String typeId;
}
# Your application may contain more items that need to be preserved;
# typically classes that are dynamically created using Class.forName:

-keep,includedescriptorclasses public class com.zeroc.Ice.** {
  public *;
}
-keep public class com.zeroc.IceGrid.** {
  public *;
}
-keep public class com.zeroc.IceSSL.** {
  public *;
}
-keep interface com.zeroc.IceGrid.**
-keep class com.zeroc.Ice.IceMX.**
-keep interface com.zeroc.Ice.IceMX.**
-keep class com.zeroc.IceStorm.IceMX.**
-keep interface com.zeroc.IceStorm.IceMX.**
-keep class com.zeroc.Glacier2.IceMX.**
-keep interface com.zeroc.Glacier2.IceMX.**

-keep,includedescriptorclasses class com.jgoodies.looks.plastic.PlasticXPLookAndFeel
-keep,includedescriptorclasses class com.jgoodies.looks.plastic.PlasticFieldCaret
-keep,includedescriptorclasses class com.jgoodies.looks.windows.WindowsFieldCaret

#
# Without this directive, the menus on Windows don't look good
#
-keepclassmembers class com.jgoodies.looks.** {
   <methods>;
}

#
# More keeps to suppress Notes
#
-keep,includedescriptorclasses public class * {
    public protected *;
}
