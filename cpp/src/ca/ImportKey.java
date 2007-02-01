// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

//
// Java's keytool program does not provide any way to import a private
// key, so this simple utility imports a private key and certificate
// chain from a PKCS12 file into a Java keystore.
//
import java.security.Key;
import java.security.KeyStore;
import java.security.cert.Certificate;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;

public class ImportKey
{
    static private char[]
    readPassword(String f)
    {
        try
        {
            java.io.InputStream s = new java.io.FileInputStream(f);
            int n = s.available();
            byte[] raw = new byte[n];
            s.read(raw);
            s.close();
            
            return (new String(raw)).toCharArray();
        }
        catch(Exception ex)
        {
            ex.printStackTrace();
            System.exit(1);
        }
        return null;
    }
        
    public static void
    main(String[] args)
    {
        if(args.length < 5)
        {
            //
            // Arguments:
            //
            // pkcs12-file A file in PKCS12 format that contains the
            // private key and certificate chain.
            //
            // alias The key's friendly name in pkcs12-file and the
            // alias for the key in the new keystore.
            //
            // cert-file The CA certificate file in DER format.
            //
            // keystore-file The name of the keystore file to update
            // or create.
            //
            // store-password The file containing the password to use
            // for the keystore.
            //
            // key-password The file containing the password to use
            // for the key.
            //
            // kstore-password The file containing the password to use
            // to store the key. If this isn't provided the
            // key-password is used. This is used to import an
            // unprotected private key.
            //
            System.err.println("Usage: ImportKey pkcs12-file alias cert-file " +
                               "keystore-file store-password key-password [kstore-password]");
            System.exit(1);
        }

        final String pkcs12File = args[0];
        final String alias = args[1];
        final String certFile = args[2];
        final String keystoreFile = args[3];
        final char[] storePassword = readPassword(args[4]);
        final char[] keyPassword = readPassword(args[5]);
        char[] kstorePassword = readPassword(args[5]);
        if(args.length > 6)
        {
            kstorePassword = readPassword(args[6]);
        }
        else
        {
            kstorePassword = keyPassword;
        }

        try
        {
            KeyStore src = KeyStore.getInstance("PKCS12");
            src.load(new java.io.FileInputStream(pkcs12File), keyPassword);

            KeyStore dest = KeyStore.getInstance("JKS");
            java.io.File f = new java.io.File(keystoreFile);
            if(f.exists())
            {
                dest.load(new java.io.FileInputStream(f), storePassword);
            }
            else
            {
                dest.load(null, null);
            }
            Certificate[] chain = src.getCertificateChain(alias);
            Key key = src.getKey(alias, keyPassword);

            CertificateFactory cf = CertificateFactory.getInstance("X.509");
            X509Certificate cert = (X509Certificate)
                cf.generateCertificate(new java.io.FileInputStream(certFile));

            dest.setKeyEntry(alias, key, kstorePassword, chain);
            dest.setCertificateEntry("cacert", cert);

            dest.store(new java.io.FileOutputStream(keystoreFile), storePassword);
        }
        catch(Exception ex)
        {
            ex.printStackTrace();
            System.exit(1);
        }

        System.exit(0);
    }
}
