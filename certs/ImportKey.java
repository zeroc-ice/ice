//
// Java's keytool program does not provide any way to import a private key,
// so this simple utility imports a private key and certificate chain from a
// PKCS12 file into a Java keystore.
//
import java.security.Key;
import java.security.KeyStore;
import java.security.cert.Certificate;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;

public class ImportKey
{
    public static void
    main(String[] args)
    {
        if(args.length != 5)
        {
            //
            // Arguments:
            //
            // pkcs12-file      A file in PKCS12 format that contains the
            //                  private key and certificate chain.
            //
            // alias            The key's friendly name in pkcs12-file and the
            //                  alias for the key in the new keystore.
            //
            // cert-file        The CA certificate file in DER format.
            //
            // keystore-file    The name of the keystore file to update or
            //                  create.
            //
            // password         The password to use for the key and keystore.
            //
            System.err.println("Usage: ImportKey pkcs12-file alias cert-file " +
                               "keystore-file password");
            System.exit(1);
        }

        final String pkcs12File = args[0];
        final String alias = args[1];
        final String certFile = args[2];
        final String keystoreFile = args[3];
        final char[] password = args[4].toCharArray();

        try
        {
            KeyStore src = KeyStore.getInstance("PKCS12");
            src.load(new java.io.FileInputStream(pkcs12File), password);

            KeyStore dest = KeyStore.getInstance("JKS");
            java.io.File f = new java.io.File(keystoreFile);
            if(f.exists())
            {
                dest.load(new java.io.FileInputStream(f), password);
            }
            else
            {
                dest.load(null, null);
            }
            Certificate[] chain = src.getCertificateChain(alias);
            Key key = src.getKey(alias, password);

            CertificateFactory cf = CertificateFactory.getInstance("X.509");
            X509Certificate cert = (X509Certificate)
                cf.generateCertificate(new java.io.FileInputStream(certFile));

            dest.setKeyEntry(alias, key, password, chain);
            dest.setCertificateEntry("cacert", cert);

            dest.store(new java.io.FileOutputStream(keystoreFile), password);
        }
        catch(Exception ex)
        {
            ex.printStackTrace();
            System.exit(1);
        }

        System.exit(0);
    }
}
