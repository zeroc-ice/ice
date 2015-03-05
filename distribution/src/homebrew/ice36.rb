require 'formula'

class Ice36 < Formula
  homepage 'http://www.zeroc.com'
  #url 'https://www.zeroc.com/download/Ice/3.6/Ice-3.6b.tar.gz'
  url 'http://dev.vpn.zeroc.com/share/Ice-3.6.0.tar.gz'
  #sha1 'dcab7e14b3e42fa95af58f7e804f6fd9a17cb6b2'

  option 'with-java-8', 'Compile with Java 8 support.'
  option 'without-java', 'Compile without Java support.'

  if build.with? "java-8"
    depends_on :java => "1.8"
  elsif build.with? "java"
    depends_on :java => "1.7"
  end

  depends_on 'mcpp'
  depends_on 'berkeley-db53'

  def install
    inreplace "cpp/src/slice2py/Makefile" do |s|
        s.sub! /install:/, "dontinstall:"
    end
    inreplace "cpp/src/slice2rb/Makefile" do |s|
        s.sub! /install:/, "dontinstall:"
    end
    inreplace "cpp/src/slice2js/Makefile" do |s|
        s.sub! /install:/, "dontinstall:"
    end

    if not (build.with? "java" or build.with? "java-8")
      inreplace "cpp/src/slice2java/Makefile" do |s|
          s.sub! /install:/, "dontinstall:"
      end
      inreplace "cpp/src/slice2freezej/Makefile" do |s|
          s.sub! /install:/, "dontinstall:"
      end
    end

    # Unset ICE_HOME as it interferes with the build
    ENV.delete('ICE_HOME')
    ENV.delete('USE_BIN_DIST')
    ENV.delete('CPPFLAGS')
    ENV.O2

    args = %W[
      prefix=#{prefix}
      embedded_runpath_prefix=#{prefix}
      USR_DIR_INSTALL=yes
      OPTIMIZE=yes
      DB_HOME=#{HOMEBREW_PREFIX}/opt/berkeley-db53
    ]
    #DB_HOME=#{libexec}
    # Setting this gets rid of the optimization level and the arch flags.
    #
    #args << "CXXFLAGS=#{ENV.cflags}"

    cd "cpp" do
      system "make", "install", *args
    end

    cd "objc" do
      system "make", "install", *args
    end

    if (build.with? "java" or build.with? "java-8")
        cd "java" do
          system "make", "install", *args
        end
    end

    cd "php" do
        args << "install_phpdir=#{lib}/share/php"
        args << "install_libdir=#{lib}/php/extensions"
        system "make", "install", *args
    end
  end

  test do
    system "#{bin}/icebox", "--version"
  end

  def caveats
    <<-EOS.undent
      To enable IcePHP, you will need to change your php.ini
      to load the IcePHP extension. You can do this by adding
      IcePHP.dy to your list of extensions:

          extension=#{prefix}/lib/php/extensions/IcePHP.dy

      Typical Ice PHP scripts will also expect to be able to 'require Ice.php'.

      You can ensure this is possible by appending the path to
      Ice's PHP includes to your global include_path in php.ini:

          include_path=<your-original-include-path>:#{prefix}/lib/share/php

      However, you can also accomplish this on a script-by-script basis
      or via .htaccess if you so desire...

      If you installed with java support the IceGrid GUI app bundles
      were installed.

      Run `brew linkapps ice36` to symlink these to /Applications.
    EOS
  end
end
