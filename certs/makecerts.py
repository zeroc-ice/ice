#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, shutil, subprocess

#
# Show usage information.
#
def usage():
    print("Usage: " + sys.argv[0] + " [options] [cpp|java|.net]")
    print("")
    print("Options:")
    print("-h    Show this message.")
    print("-f    Force updates to files that otherwise would be skipped.")
    print("-d    Debugging output.")
    print("")
    print("The certificates for all languages are updated if you do not specify one.")

def newer(file1, file2):
    file1info = os.stat(file1)
    file2info = os.stat(file2)
    return file1info.st_mtime > file2info.st_mtime

def prepareCAHome(dir, force):
    if force and os.path.exists(dir):
        shutil.rmtree(dir)

    if not os.path.exists(dir):
        os.mkdir(dir)

    f = open(os.path.join(dir, "serial"), "w")
    f.write("01")
    f.close()

    f = open(os.path.join(dir, "index.txt"), "w")
    f.truncate(0)
    f.close()

    
def jksToBks(source, target):
    cmd = "keytool -importkeystore -srckeystore " + source + " -destkeystore " + target + " -srcstoretype JKS -deststoretype BKS " + \
          "-srcstorepass password -deststorepass password -provider org.bouncycastle.jce.provider.BouncyCastleProvider -noprompt"
    if debug:
        print("[debug]", cmd)


    p = subprocess.Popen(cmd, shell = True, stdin = subprocess.PIPE, stdout = subprocess.PIPE,
                        stderr = subprocess.STDOUT, bufsize = 0)

    while(True):

        line = p.stdout.readline()            
        if p.poll() is not None and not line:
            # The process terminated
            break
            
        sys.stdout.write(line)
        
        if line.find("java.lang.ClassNotFoundException: org.bouncycastle.jce.provider.BouncyCastleProvider") != -1:
            print("")
            print("WARNING: BouncyCastleProvider not found cannot export certificates for android demos in BKS format.")
            print("         You can download BKS provider from http://www.bouncycastle.org/download/bcprov-jdk15on-146.jar.")
            print("         After download copy the JAR to $JAVA_HOME/lib/ext where JAVA_HOME points to your JRE")
            print("         and run this script again.")
            print("")
            sys.exit(1)
        elif line.find("java.security.InvalidKeyException: Illegal key size") != -1:
            print("")
            print("WARNING: You need to install Java Cryptography Extension (JCE) Unlimited Strength.")
            print("         You can download it from Additional Resources section in Orcale Java Download page at:")
            print("             http://www.oracle.com/technetwork/java/javase/downloads/index.html.")
            print("")
            sys.exit(1)
            
    if p.poll() != 0:
        sys.exist(1)
#
# Check arguments
#
force = False
debug = False
lang = None
for x in sys.argv[1:]:
    if x == "-h":
        usage()
        sys.exit(0)
    elif x == "-f":
        force = True
    elif x == "-d":
        debug = True
    elif x.startswith("-"):
        print(sys.argv[0] + ": unknown option `" + x + "'")
        print("")
        usage()
        sys.exit(1)
    else:
        if lang != None or x not in ["cpp", "java", ".net"]:
            usage()
            sys.exit(1)
        lang = x

certs = "."
caHome = os.path.join(certs, "openssl", "ca")

#
# Check for cakey.pem and regenerate it if it doesn't exist or if force is true.
#
caKey = os.path.join(certs, "cakey.pem")
caCert = os.path.join(certs, "cacert.pem")
if not os.path.exists(caKey) or force:

    print("Generating new CA certificate and key...")
    if os.path.exists(caKey):
        os.remove(caKey)
    if os.path.exists(caCert):
        os.remove(caCert)

    prepareCAHome(caHome, force)

    config = os.path.join(certs, "openssl", "ice_ca.cnf")
    cmd = "openssl req -config " + config + " -x509 -days 1825 -newkey rsa:1024 -out " + \
           os.path.join(caHome, "cacert.pem") + " -outform PEM -nodes"
    if debug:
        print("[debug]", cmd)
    os.system(cmd)
    shutil.copyfile(os.path.join(caHome, "cakey.pem"), caKey)
    shutil.copyfile(os.path.join(caHome, "cacert.pem"), caCert)

    cmd = "openssl x509 -in " + caCert + " -outform DER -out " + os.path.join(certs, "cacert.der")
    if debug:
        print("[debug]", cmd)
    os.system(cmd)

else:
    print("Skipping CA certificate and key.")

#
# C++ server RSA certificate and key.
#
cppServerCert = os.path.join(certs, "s_rsa1024_pub.pem")
cppServerKey = os.path.join(certs, "s_rsa1024_priv.pem")
if force or not os.path.exists(cppServerCert) or not os.path.exists(cppServerKey) or \
   (os.path.exists(cppServerCert) and newer(caCert, cppServerCert)):

    print("Generating new C++ server RSA certificate and key...")

    if os.path.exists(cppServerCert):
        os.remove(cppServerCert)
    if os.path.exists(cppServerKey):
        os.remove(cppServerKey)

    prepareCAHome(caHome, force)

    serial = os.path.join(caHome, "serial")
    f = open(serial, "r")
    serialNum = f.read().strip()
    f.close()

    tmpKey = os.path.join(caHome, serialNum + "_key.pem")
    tmpCert = os.path.join(caHome, serialNum + "_cert.pem")
    req = os.path.join(caHome, "req.pem")
    config = os.path.join(certs, "openssl", "server.cnf")
    cmd = "openssl req -config " + config + " -newkey rsa:1024 -nodes -keyout " + tmpKey + " -keyform PEM" + \
           " -out " + req
    if debug:
        print("[debug]", cmd)
    os.system(cmd)

    cmd = "openssl ca -config " + config + " -batch -in " + req
    if debug:
        print("[debug]", cmd)
    os.system(cmd)
    shutil.move(os.path.join(caHome, serialNum + ".pem"), tmpCert)
    shutil.copyfile(tmpKey, cppServerKey)
    shutil.copyfile(tmpCert, cppServerCert)
    os.remove(req)
else:
    print("Skipping C++ server RSA certificate and key.")

#
# C++ client RSA certificate and key.
#
cppClientCert = os.path.join(certs, "c_rsa1024_pub.pem")
cppClientKey = os.path.join(certs, "c_rsa1024_priv.pem")
if force or not os.path.exists(cppClientCert) or not os.path.exists(cppClientKey) or \
   (os.path.exists(cppClientCert) and newer(caCert, cppClientCert)):

    print("Generating new C++ client RSA certificate and key...")

    if os.path.exists(cppClientCert):
        os.remove(cppClientCert)
    if os.path.exists(cppClientKey):
        os.remove(cppClientKey)

    prepareCAHome(caHome, force)

    serial = os.path.join(caHome, "serial")
    f = open(serial, "r")
    serialNum = f.read().strip()
    f.close()

    tmpKey = os.path.join(caHome, serialNum + "_key.pem")
    tmpCert = os.path.join(caHome, serialNum + "_cert.pem")
    req = os.path.join(caHome, "req.pem")
    config = os.path.join(certs, "openssl", "client.cnf")
    cmd = "openssl req -config " + config + " -newkey rsa:1024 -nodes -keyout " + tmpKey + " -keyform PEM" + \
           " -out " + req
    if debug:
        print("[debug]", cmd)
    os.system(cmd)

    cmd = "openssl ca -config " + config + " -batch -in " + req
    if debug:
        print("[debug]", cmd)
    os.system(cmd)
    shutil.move(os.path.join(caHome, serialNum + ".pem"), tmpCert)
    shutil.copyfile(tmpKey, cppClientKey)
    shutil.copyfile(tmpCert, cppClientCert)
    os.remove(req)
else:
    print("Skipping C++ client RSA certificate and key.")

#
# C++ DSA parameters.
#
dsaParams = os.path.join(certs, "dsaparam1024.pem")
if (lang == "cpp" or lang == None) and (force or not os.path.exists(dsaParams)):

    print("Generating new C++ DSA parameters...")

    if os.path.exists(dsaParams):
        os.remove(dsaParams)

    prepareCAHome(caHome, force)

    cmd = "openssl dsaparam -out " + dsaParams + " -outform PEM 1024"
    if debug:
        print("[debug]", cmd)
    os.system(cmd)
else:
    print("Skipping C++ DSA parameters.")

#
# C++ server DSA certificate and key.
#
cppServerCertDSA = os.path.join(certs, "s_dsa1024_pub.pem")
cppServerKeyDSA = os.path.join(certs, "s_dsa1024_priv.pem")
if (lang == "cpp" or lang == None) and \
   (force or not os.path.exists(cppServerCertDSA) or not os.path.exists(cppServerKeyDSA) or \
   (os.path.exists(cppServerCertDSA) and newer(caCert, cppServerCertDSA)) or \
   (os.path.exists(cppServerCertDSA) and newer(dsaParams, cppServerCertDSA))):

    print("Generating new C++ server DSA certificate and key...")

    if os.path.exists(cppServerCertDSA):
        os.remove(cppServerCertDSA)
    if os.path.exists(cppServerKeyDSA):
        os.remove(cppServerKeyDSA)

    prepareCAHome(caHome, force)

    serial = os.path.join(caHome, "serial")
    f = open(serial, "r")
    serialNum = f.read().strip()
    f.close()

    tmpKey = os.path.join(caHome, serialNum + "_key.pem")
    tmpCert = os.path.join(caHome, serialNum + "_cert.pem")
    req = os.path.join(caHome, "req.pem")
    config = os.path.join(certs, "openssl", "server.cnf")
    cmd = "openssl req -config " + config + " -newkey dsa:" + dsaParams + " -nodes -keyout " + tmpKey + \
          " -keyform PEM -out " + req
    if debug:
        print("[debug]", cmd)
    os.system(cmd)

    cmd = "openssl ca -config " + config + " -batch -in " + req
    if debug:
        print("[debug]", cmd)
    os.system(cmd)
    shutil.move(os.path.join(caHome, serialNum + ".pem"), tmpCert)
    shutil.copyfile(tmpKey, cppServerKeyDSA)
    shutil.copyfile(tmpCert, cppServerCertDSA)
    os.remove(req)
else:
    print("Skipping C++ server DSA certificate and key.")

#
# C++ client DSA certificate and key.
#
cppClientCertDSA = os.path.join(certs, "c_dsa1024_pub.pem")
cppClientKeyDSA = os.path.join(certs, "c_dsa1024_priv.pem")
if (lang == "cpp" or lang == None) and \
   (force or not os.path.exists(cppClientCertDSA) or not os.path.exists(cppClientKeyDSA) or \
   (os.path.exists(cppClientCertDSA) and newer(caCert, cppClientCertDSA)) or \
   (os.path.exists(cppClientCertDSA) and newer(dsaParams, cppClientCertDSA))):

    print("Generating new C++ client DSA certificate and key...")

    if os.path.exists(cppClientCertDSA):
        os.remove(cppClientCertDSA)
    if os.path.exists(cppClientKeyDSA):
        os.remove(cppClientKeyDSA)

    prepareCAHome(caHome, force)

    serial = os.path.join(caHome, "serial")
    f = open(serial, "r")
    serialNum = f.read().strip()
    f.close()

    tmpKey = os.path.join(caHome, serialNum + "_key.pem")
    tmpCert = os.path.join(caHome, serialNum + "_cert.pem")
    req = os.path.join(caHome, "req.pem")
    config = os.path.join(certs, "openssl", "client.cnf")
    cmd = "openssl req -config " + config + " -newkey dsa:" + dsaParams + " -nodes -keyout " + tmpKey + \
          " -keyform PEM -out " + req
    if debug:
        print("[debug]", cmd)
    os.system(cmd)

    cmd = "openssl ca -config " + config + " -batch -in " + req
    if debug:
        print("[debug]", cmd)
    os.system(cmd)
    shutil.move(os.path.join(caHome, serialNum + ".pem"), tmpCert)
    shutil.copyfile(tmpKey, cppClientKeyDSA)
    shutil.copyfile(tmpCert, cppClientCertDSA)
    os.remove(req)
else:
    print("Skipping C++ client DSA certificate and key.")

#
# .NET server RSA certificate and key.
#
csServer = os.path.join(certs, "s_rsa1024.pfx")
if (lang == ".net" or lang == None) and (force or not os.path.exists(csServer) or newer(cppServerCert, csServer)):

    print("Generating new .NET server RSA certificate and key...")

    if os.path.exists(csServer):
        os.remove(csServer)

    cmd = "openssl pkcs12 -in " + cppServerCert + " -inkey " + cppServerKey + " -export -out " + csServer + \
          " -certpbe PBE-SHA1-RC4-40 -keypbe PBE-SHA1-RC4-40 -passout pass:password"
    if debug:
        print("[debug]", cmd)
    os.system(cmd)
else:
    print("Skipping .NET server certificate and key.")

#
# .NET client RSA certificate and key.
#
csClient = os.path.join(certs, "c_rsa1024.pfx")
if (lang == ".net" or lang == None) and (force or not os.path.exists(csClient) or \
   (os.path.exists(csClient) and newer(cppClientCert, csClient))):

    print("Generating new .NET client RSA certificate and key...")

    if os.path.exists(csClient):
        os.remove(csClient)

    cmd = "openssl pkcs12 -in " + cppClientCert + " -inkey " + cppClientKey + " -export -out " + csClient + \
          " -certpbe PBE-SHA1-RC4-40 -keypbe PBE-SHA1-RC4-40 -passout pass:password"
    if debug:
        print("[debug]", cmd)
    os.system(cmd)
else:
    print("Skipping .NET client certificate and key.")

#
# Java truststore.
#
truststore = "certs.jks"
if (lang == "java" or lang == None) and (force or not os.path.exists(truststore) or \
   (os.path.exists(truststore) and newer(caCert, truststore))):

    print("Generating Java truststore...")

    if os.path.exists(truststore):
        os.remove(truststore)

    cacert = os.path.join(certs, "cacert.der")

    cmd = "keytool -import -alias cacert -file " + cacert + " -keystore " + truststore + \
          " -storepass password -noprompt"
    if debug:
        print("[debug]", cmd)
    os.system(cmd)
else:
    print("Skipping Java truststore.")

#
# Java server keystore.
#
serverKeystore = "server.jks"
if (lang == "java" or lang == None) and (force or not os.path.exists(serverKeystore) or \
   (os.path.exists(serverKeystore) and newer(cppServerCert, serverKeystore))):

    print("Generating Java server keystore...")

    if os.path.exists(serverKeystore):
        os.remove(serverKeystore)

    #
    # Convert OpenSSL key/certificate pairs into PKCS12 format and then
    # import them into a Java keystore.
    #
    tmpFile = os.path.join(certs, "server.p12")
    cmd = "openssl pkcs12 -in " + cppServerCert + " -inkey " + cppServerKey + " -export -out " + tmpFile + \
          " -name rsakey -passout pass:password -certfile " + caCert
    if debug:
        print("[debug]", cmd)
    os.system(cmd)
    cmd = "java -classpath . ImportKey " + tmpFile + " rsakey " + caCert + " " + serverKeystore + " password"
    if debug:
        print("[debug]", cmd)
    os.system(cmd)
    os.remove(tmpFile)
else:
    print("Skipping Java server keystore.")
    
    
if not os.path.exists("server.bks") or newer(serverKeystore, "server.bks"):

    if os.path.exists("server.bks"):
        os.remove("server.bks")

    print("Converting Java server truststore to BKS...")
    
    jksToBks("server.jks", "server.bks")
    
    #
    # Replace server.bks files in android demo and test directories
    #
    for d in ['../java/test/android', '../java/demo/android']:
        for root, dirnames, filenames in os.walk(d):
            for f in filenames:
                if f == "server.bks":
                    shutil.copyfile("server.bks", os.path.join(root, f))

#
# Java client keystore.
#
clientKeystore = "client.jks"
if (lang == "java" or lang == None) and (force or not os.path.exists(clientKeystore) or \
   (os.path.exists(clientKeystore) and newer(cppClientCert, clientKeystore))):

    print("Generating Java client keystore...")

    if os.path.exists(clientKeystore):
        os.remove(clientKeystore)

    #
    # Convert OpenSSL key/certificate pairs into PKCS12 format and then
    # import them into a Java keystore.
    #
    tmpFile = os.path.join(certs, "client.p12")
    cmd = "openssl pkcs12 -in " + cppClientCert + " -inkey " + cppClientKey + " -export -out " + tmpFile + \
          " -name rsakey -passout pass:password -certfile " + caCert
    if debug:
        print("[debug]", cmd)
    os.system(cmd)
    cmd = "java -classpath . ImportKey " + tmpFile + " rsakey " + caCert + " " + clientKeystore + " password"
    if debug:
        print("[debug]", cmd)
    os.system(cmd)
    os.remove(tmpFile)
else:
    print("Skipping Java client keystore.")
    
if not os.path.exists("client.bks") or newer(clientKeystore, "client.bks"):

    if os.path.exists("client.bks"):
        os.remove("client.bks")

    print("Converting Java client truststore to BKS...")
    
    jksToBks("client.jks", "client.bks")
    
    #
    # Replace client.bks files in android demo and test directories
    #
    for d in ['../java/test/android', '../java/demo/android']:
        for root, dirnames, filenames in os.walk(d):
            for f in filenames:
                if f == "client.bks":
                    shutil.copyfile("client.bks", os.path.join(root, f))
#
# Done.
#
print("Done.")
