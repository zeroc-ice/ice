# shellcheck shell=bash
# Regeneration of the certs/configuration certificates used by the IceSSL/configuration test.
# Sourced by makecerts.sh, which defines DEFAULT_DAYS, DEFAULT_PASSWORD and COMMON_PKCS12_ARGS.
#
# Every certificate uses the same distinguished name except for the common name. The openssl 'ca'
# policy below lists the RDNs common-name first so the issued certificates keep the common-name-first
# ordering the IceSSL/configuration tests expect. unique_subject is disabled because many leaf
# certificates share the same common name (e.g. "Server").

# The distinguished-name suffix appended after the per-certificate common name.
DN_SUFFIX="/OU=Ice/O=ZeroC, Inc./L=Jupiter/ST=Florida/C=US/emailAddress=info@zeroc.com"

# The subject alternative name carried by the CA certificates.
CA_SAN="IP:127.0.0.1, email:issuer@zeroc.com"

# Initialize an openssl CA database directory.
init_db() {
    local dir="$1"
    mkdir -p "${dir}/newcerts"
    : > "${dir}/index.txt"
    printf 'unique_subject = no\n' > "${dir}/index.txt.attr"
    echo "${2:-1000}" > "${dir}/serial"
    echo "1000" > "${dir}/crlnumber"
}

# Write the openssl 'ca' configuration for the CA rooted at $1 (its cert is $1/ca.pem, key
# $1/ca_key.pem). The configuration is written to $1/ca.cnf.
write_ca_cnf() {
    local dir="$1"
    cat > "${dir}/ca.cnf" <<EOF
[ ca ]
default_ca = CA_default

[ CA_default ]
dir              = ${dir}
database         = ${dir}/index.txt
new_certs_dir    = ${dir}/newcerts
serial           = ${dir}/serial
crlnumber        = ${dir}/crlnumber
certificate      = ${dir}/ca.pem
private_key      = ${dir}/ca_key.pem
default_md       = sha256
default_days     = ${DEFAULT_DAYS}
default_crl_days = ${DEFAULT_DAYS}
preserve         = no
policy           = policy_cn_first

[ policy_cn_first ]
commonName             = supplied
organizationalUnitName = optional
organizationName       = optional
localityName           = optional
stateOrProvinceName    = optional
countryName            = optional
emailAddress           = optional
EOF
}

# Create a self-signed root CA. Usage: new_root <db_dir> <cn> [extra_ca_extensions]
new_root() {
    local dir="$1" cn="$2" extra="${3:-}"
    init_db "${dir}"
    write_ca_cnf "${dir}"
    cat > "${dir}/req.cnf" <<EOF
[ req ]
prompt             = no
distinguished_name = dn
x509_extensions    = v3_ca
[ dn ]
CN = ${cn}
OU = Ice
O  = ZeroC, Inc.
L  = Jupiter
ST = Florida
C  = US
emailAddress = info@zeroc.com
[ v3_ca ]
subjectKeyIdentifier   = hash
authorityKeyIdentifier = keyid:always,issuer
basicConstraints       = critical, CA:true
keyUsage               = critical, digitalSignature, keyCertSign, cRLSign
subjectAltName         = ${CA_SAN}
${extra}
EOF
    openssl req -x509 -noenc -newkey rsa:2048 -days "${DEFAULT_DAYS}" \
        -keyout "${dir}/ca_key.pem" -out "${dir}/ca.pem" -config "${dir}/req.cnf" -extensions v3_ca
}

# Create an intermediate CA signed by its parent.
# Usage: new_intermediate <db_dir> <parent_db_dir> <cn> [extra_ca_extensions]
new_intermediate() {
    local dir="$1" parent="$2" cn="$3" extra="${4:-}"
    init_db "${dir}"
    write_ca_cnf "${dir}"
    cat > "${dir}/ext.cnf" <<EOF
[ v3_ca ]
subjectKeyIdentifier   = hash
authorityKeyIdentifier = keyid:always,issuer
basicConstraints       = critical, CA:true
keyUsage               = critical, digitalSignature, keyCertSign, cRLSign
${extra}
EOF
    openssl req -new -noenc -newkey rsa:2048 -keyout "${dir}/ca_key.pem" -out "${dir}/ca.csr" \
        -subj "/CN=${cn}${DN_SUFFIX}"
    openssl ca -batch -notext -config "${parent}/ca.cnf" -in "${dir}/ca.csr" \
        -extfile "${dir}/ext.cnf" -extensions v3_ca -out "${dir}/ca.pem"
    rm -f "${dir}/ca.csr"
}

# Issue a leaf certificate. Writes <out>_priv.pem, <out>_pub.pem and <out>.p12.
# Usage: issue <out> <ca_db_dir> <cn> <ext_block> [p12_chain_pem] [p12_pass] [faketime_offset] [legacy=yes]
# Pass legacy=no to export the PKCS12 with the modern defaults (used for the empty-password certs;
# the legacy macOS-Keychain args do not produce a verifiable MAC for an empty password).
issue() {
    local out="$1" cadir="$2" cn="$3" ext="$4" chain="${5:-}" pass="${6-${DEFAULT_PASSWORD}}" ft="${7:-}" legacy="${8:-yes}"
    local key="${out}_priv.pem" cert="${out}_pub.pem" csr="${out}.csr" extf="${out}.ext"
    cat > "${extf}" <<EOF
[ v3_leaf ]
subjectKeyIdentifier   = hash
authorityKeyIdentifier = keyid:always,issuer
keyUsage               = digitalSignature, nonRepudiation, keyEncipherment
issuerAltName          = issuer:copy
${ext}
EOF
    openssl req -new -noenc -newkey rsa:2048 -keyout "${key}" -out "${csr}" -subj "/CN=${cn}${DN_SUFFIX}"
    local cmd=(openssl ca -batch -notext -config "${cadir}/ca.cnf" -in "${csr}"
               -extfile "${extf}" -extensions v3_leaf -out "${cert}")
    if [ -n "${ft}" ]; then faketime -f "${ft}" "${cmd[@]}"; else "${cmd[@]}"; fi
    local certfile=()
    [ -n "${chain}" ] && certfile=(-certfile "${chain}")
    local p12args=("${COMMON_PKCS12_ARGS[@]}")
    [ "${legacy}" = "no" ] && p12args=()
    openssl pkcs12 -export -out "${out}.p12" -inkey "${key}" -in "${cert}" "${certfile[@]}" \
        -name cert -passout pass:"${pass}" "${p12args[@]}"
    # Append the issuer chain (intermediates, excluding the root) to the PEM certificate as well, so
    # SecureTransport can build the chain up to the root when the certificate is loaded from PEM.
    [ -n "${chain}" ] && cat "${chain}" >> "${cert}"
    rm -f "${csr}" "${extf}"
}

# Revoke certificates in a CA and (re)generate its CRL. Usage: revoke <ca_db_dir> <crl_out> <cert...>
revoke() {
    local cadir="$1" crl="$2"; shift 2
    local c
    for c in "$@"; do
        openssl ca -config "${cadir}/ca.cnf" -revoke "${c}" -batch
    done
    openssl ca -config "${cadir}/ca.cnf" -gencrl -out "${crl}" -crldays "${DEFAULT_DAYS}"
}

# Import a PKCS12 key entry as a JKS keystore. Usage: jks_key <p12> <jks> [srcalias=cert] [destalias=srcalias]
jks_key() {
    local srcalias="${3:-cert}" destalias="${4:-${3:-cert}}"
    rm -f "$2"
    keytool -importkeystore -srckeystore "$1" -srcstoretype PKCS12 -srcstorepass "${DEFAULT_PASSWORD}" \
        -destkeystore "$2" -deststoretype JKS -deststorepass "${DEFAULT_PASSWORD}" \
        -srcalias "${srcalias}" -destalias "${destalias}"
}

# Import one or more certificates as trusted entries in a JKS keystore. Usage: jks_trust <jks> <alias:cert>...
jks_trust() {
    local jks="$1"; shift
    rm -f "${jks}"
    local pair
    for pair in "$@"; do
        keytool -importcert -noprompt -alias "${pair%%:*}" -file "${pair#*:}" \
            -keystore "${jks}" -storetype JKS -storepass "${DEFAULT_PASSWORD}"
    done
}

# The CRL distribution point / authority information access extension blocks (the test serves these
# over HTTP from scripts/tests/IceSSL/revocationutil.py on ports 20001/20002).
ca3_crldp="crlDistributionPoints = URI:http://127.0.0.1:20001/ca3.crl.pem"
cai3_crldp="crlDistributionPoints = URI:http://127.0.0.1:20001/cai3.crl.pem"
ca4_aia="authorityInfoAccess = OCSP;URI:http://127.0.0.1:20002, caIssuers;URI:http://127.0.0.1:20001/cacert4.der"
cai4_aia="authorityInfoAccess = OCSP;URI:http://127.0.0.1:20002, caIssuers;URI:http://127.0.0.1:20001/cai4.der"

generate_configuration() {
    pushd configuration > /dev/null

    # Remove the previously generated material, keeping the custom-extension inputs and the RNG seed.
    rm -rf db
    find . -maxdepth 1 -type f \( -name '*.pem' -o -name '*.der' -o -name '*.p12' -o -name '*.jks' \
        -o -name '*.csr' \) ! -name 'cacert_custom.req' -exec rm -f {} +

    # ---- Certificate authorities -------------------------------------------------------------
    new_root db/ca1 "ZeroC Test CA 1"
    new_intermediate db/ca1/intermediate1 db/ca1 "ZeroC Test Intermediate CA 1"
    new_intermediate db/ca1/intermediate1/intermediate1 db/ca1/intermediate1 "ZeroC Test Intermediate CA 2"
    new_root db/ca2 "ZeroC Test CA 2" "extendedKeyUsage = serverAuth"
    new_root db/ca3 "ZeroC Test CA 3"
    new_intermediate db/ca3/intermediate1 db/ca3 "ZeroC Test Intermediate CA 3" "${cai3_crldp}"
    new_root db/ca4 "ZeroC Test CA 4"
    new_intermediate db/ca4/intermediate1 db/ca4 "ZeroC Test Intermediate CA 4" "${cai4_aia}"

    # The serial file controls the leaf serial numbers; s_rsa_ca1 / c_rsa_ca1 are pinned to 1 / 2
    # because the IceSSL.FindCert tests assert those values.
    echo 01 > db/ca1/serial

    # Output CA certificates (PEM + DER) and the CA bundle.
    local ca
    for ca in 1 2 3 4; do
        cp "db/ca${ca}/ca.pem" "cacert${ca}.pem"
        openssl x509 -in "cacert${ca}.pem" -out "cacert${ca}.der" -outform DER
    done
    cp db/ca3/intermediate1/ca.pem cai3.pem; openssl x509 -in cai3.pem -out cai3.der -outform DER
    cp db/ca4/intermediate1/ca.pem cai4.pem; openssl x509 -in cai4.pem -out cai4.der -outform DER
    cat cacert1.pem cacert2.pem cacert3.pem cacert4.pem > cacerts.pem

    # ca2 doubles as a self-signed server certificate.
    cp db/ca2/ca.pem cacert2_pub.pem
    cp db/ca2/ca_key.pem cacert2_priv.pem
    openssl pkcs12 -export -out cacert2.p12 -inkey cacert2_priv.pem -in cacert2_pub.pem \
        -name ca -passout pass:"${DEFAULT_PASSWORD}" "${COMMON_PKCS12_ARGS[@]}"

    # ---- ca1 leaves --------------------------------------------------------------------------
    local server_eku="extendedKeyUsage = serverAuth"
    local client_eku="extendedKeyUsage = clientAuth"
    # s_rsa_ca1 (serial 1) and c_rsa_ca1 (serial 2) are issued first to pin their serials.
    issue s_rsa_ca1 db/ca1 "Server" "${server_eku}
subjectAltName = IP:127.0.0.1, DNS:server"
    issue c_rsa_ca1 db/ca1 "Client" "${client_eku}
subjectAltName = IP:127.0.0.1, DNS:client"

    # Expired certificates (no SAN). faketime shifts the whole signing back DEFAULT_DAYS+1.
    issue s_rsa_ca1_exp db/ca1 "Server" "${server_eku}" "" "${DEFAULT_PASSWORD}" "-$((DEFAULT_DAYS + 1))d"
    issue c_rsa_ca1_exp db/ca1 "Client" "${client_eku}" "" "${DEFAULT_PASSWORD}" "-$((DEFAULT_DAYS + 1))d"

    # Common-name / subject-alternative-name matrix (all serverAuth).
    issue s_rsa_ca1_cn1  db/ca1 "Server"      "${server_eku}
subjectAltName = DNS:localhost"
    issue s_rsa_ca1_cn2  db/ca1 "Server"      "${server_eku}
subjectAltName = DNS:localhostXX"
    issue s_rsa_ca1_cn3  db/ca1 "localhost"   "${server_eku}"
    issue s_rsa_ca1_cn4  db/ca1 "localhostXX" "${server_eku}"
    issue s_rsa_ca1_cn5  db/ca1 "localhost"   "${server_eku}
subjectAltName = DNS:localhostXX"
    issue s_rsa_ca1_cn6  db/ca1 "Server"      "${server_eku}
subjectAltName = IP:127.0.0.1"
    issue s_rsa_ca1_cn7  db/ca1 "Server"      "${server_eku}
subjectAltName = IP:127.0.0.2"
    issue s_rsa_ca1_cn8  db/ca1 "127.0.0.1"   "${server_eku}"
    issue s_rsa_ca1_cn9  db/ca1 "127.0.0.1"   "${server_eku}
subjectAltName = IP:127.0.0.1, IP:::1"
    issue s_rsa_ca1_cn10 db/ca1 "127.0.0.1"   "${server_eku}
subjectAltName = DNS:host1, DNS:host2"
    issue s_rsa_ca1_cn11 db/ca1 "127.0.0.1"   "${server_eku}
subjectAltName = IP:127.0.0.1, IP:127.0.0.2, DNS:host1, DNS:host2"

    # Extended-key-usage matrix (no SAN, p12 + pem only).
    issue rsa_ca1_none                db/ca1 "None"                   ""
    issue rsa_ca1_serverAuth          db/ca1 "Server Auth"           "extendedKeyUsage = serverAuth"
    issue rsa_ca1_clientAuth          db/ca1 "Client Auth"           "extendedKeyUsage = clientAuth"
    issue rsa_ca1_codeSigning         db/ca1 "Code Signing"          "extendedKeyUsage = codeSigning"
    issue rsa_ca1_emailProtection     db/ca1 "Email Protection"      "extendedKeyUsage = emailProtection"
    issue rsa_ca1_timeStamping        db/ca1 "Time Stamping"         "extendedKeyUsage = timeStamping"
    issue rsa_ca1_ocspSigning         db/ca1 "OCSP Signing"          "extendedKeyUsage = OCSPSigning"
    issue rsa_ca1_anyExtendedKeyUsage db/ca1 "Any Extended Key Usage" "extendedKeyUsage = anyExtendedKeyUsage"

    # s_rsa_wroot_ca1 and the password-protected certs reuse the s_rsa_ca1 / c_rsa_ca1 key + cert.
    cp s_rsa_ca1_priv.pem s_rsa_wroot_ca1_priv.pem
    cp s_rsa_ca1_pub.pem  s_rsa_wroot_ca1_pub.pem
    openssl pkcs12 -export -out s_rsa_wroot_ca1.p12 -inkey s_rsa_wroot_ca1_priv.pem \
        -in s_rsa_wroot_ca1_pub.pem -certfile cacert1.pem -name cert \
        -passout pass:"${DEFAULT_PASSWORD}" "${COMMON_PKCS12_ARGS[@]}"

    # Password-protected key + p12 (prompt password "server" / "client").
    cp s_rsa_ca1_pub.pem s_rsa_pass_ca1_pub.pem
    openssl pkey -in s_rsa_ca1_priv.pem -out s_rsa_pass_ca1_priv.pem -aes256 -passout pass:server
    openssl pkcs12 -export -out s_rsa_pass_ca1.p12 -inkey s_rsa_ca1_priv.pem -in s_rsa_ca1_pub.pem \
        -name cert -passout pass:server "${COMMON_PKCS12_ARGS[@]}"
    cp c_rsa_ca1_pub.pem c_rsa_pass_ca1_pub.pem
    openssl pkey -in c_rsa_ca1_priv.pem -out c_rsa_pass_ca1_priv.pem -aes256 -passout pass:client
    openssl pkcs12 -export -out c_rsa_pass_ca1.p12 -inkey c_rsa_ca1_priv.pem -in c_rsa_ca1_pub.pem \
        -name cert -passout pass:client "${COMMON_PKCS12_ARGS[@]}"

    # ---- ca1 intermediates -------------------------------------------------------------------
    issue s_rsa_cai1 db/ca1/intermediate1 "Server" "${server_eku}
subjectAltName = IP:127.0.0.1, DNS:server" db/ca1/intermediate1/ca.pem
    cat db/ca1/intermediate1/intermediate1/ca.pem db/ca1/intermediate1/ca.pem > cai_chain2.pem
    issue s_rsa_cai2 db/ca1/intermediate1/intermediate1 "Server" "${server_eku}
subjectAltName = IP:127.0.0.1, DNS:server" cai_chain2.pem
    issue c_rsa_cai2 db/ca1/intermediate1/intermediate1 "Client" "${client_eku}
subjectAltName = IP:127.0.0.1, DNS:client" cai_chain2.pem
    rm -f cai_chain2.pem

    # ---- ca2 leaves --------------------------------------------------------------------------
    issue s_rsa_ca2 db/ca2 "Server" "${server_eku}
subjectAltName = IP:127.0.0.1, DNS:server"
    issue c_rsa_ca2 db/ca2 "Client" "${client_eku}
subjectAltName = IP:127.0.0.1, DNS:client"

    # ---- ca3 / cai3 (CRL) leaves -------------------------------------------------------------
    issue s_rsa_ca3 db/ca3 "Server" "${server_eku}
${ca3_crldp}
subjectAltName = IP:127.0.0.1, DNS:server"
    issue c_rsa_ca3 db/ca3 "Client" "${client_eku}
${ca3_crldp}
subjectAltName = IP:127.0.0.1, DNS:client"
    issue s_rsa_ca3_revoked db/ca3 "Server ca3 revoked" "${server_eku}
${ca3_crldp}
subjectAltName = IP:127.0.0.1, DNS:server"
    issue c_rsa_ca3_revoked db/ca3 "Client ca3 revoked" "${client_eku}
${ca3_crldp}
subjectAltName = IP:127.0.0.1, DNS:client"
    issue s_rsa_cai3 db/ca3/intermediate1 "Server cai3" "${server_eku}
${cai3_crldp}
subjectAltName = IP:127.0.0.1, DNS:server" db/ca3/intermediate1/ca.pem
    issue s_rsa_cai3_revoked db/ca3/intermediate1 "Server cai3 revoked" "${server_eku}
${cai3_crldp}
subjectAltName = IP:127.0.0.1, DNS:server" db/ca3/intermediate1/ca.pem
    issue c_rsa_cai3 db/ca3/intermediate1 "Client cai3" "${client_eku}
${cai3_crldp}
subjectAltName = IP:127.0.0.1, DNS:client" db/ca3/intermediate1/ca.pem
    issue c_rsa_cai3_revoked db/ca3/intermediate1 "Client cai3 revoked" "${client_eku}
${cai3_crldp}
subjectAltName = IP:127.0.0.1, DNS:client" db/ca3/intermediate1/ca.pem

    # ---- ca4 / cai4 (OCSP) leaves ------------------------------------------------------------
    issue s_rsa_ca4 db/ca4 "Server ca4" "${server_eku}
${ca4_aia}
subjectAltName = IP:127.0.0.1, DNS:server"
    issue s_rsa_ca4_revoked db/ca4 "Server ca4 revoked" "${server_eku}
${ca4_aia}
subjectAltName = IP:127.0.0.1, DNS:server"
    issue s_rsa_ca4_unknown db/ca4 "Server ca4 unknown" "${server_eku}
${ca4_aia}
subjectAltName = IP:127.0.0.1, DNS:server"
    issue c_rsa_ca4 db/ca4 "Client ca4" "${client_eku}
${ca4_aia}
subjectAltName = IP:127.0.0.1, DNS:client"
    issue c_rsa_ca4_revoked db/ca4 "Client ca4 revoked" "${client_eku}
${ca4_aia}
subjectAltName = IP:127.0.0.1, DNS:client"
    issue c_rsa_ca4_unknown db/ca4 "Client ca4 unknown" "${client_eku}
${ca4_aia}
subjectAltName = IP:127.0.0.1, DNS:client"
    issue s_rsa_cai4 db/ca4/intermediate1 "Server cai4" "${server_eku}
${cai4_aia}
subjectAltName = IP:127.0.0.1, DNS:server" db/ca4/intermediate1/ca.pem
    issue s_rsa_cai4_revoked db/ca4/intermediate1 "Server cai4 revoked" "${server_eku}
${cai4_aia}
subjectAltName = IP:127.0.0.1, DNS:server" db/ca4/intermediate1/ca.pem
    issue c_rsa_cai4 db/ca4/intermediate1 "Client cai4" "${client_eku}
${cai4_aia}
subjectAltName = IP:127.0.0.1, DNS:client" db/ca4/intermediate1/ca.pem
    issue c_rsa_cai4_revoked db/ca4/intermediate1 "Client cai4 revoked" "${client_eku}
${cai4_aia}
subjectAltName = IP:127.0.0.1, DNS:client" db/ca4/intermediate1/ca.pem

    # revocationutil.py reads the issued ca4 / cai4 certificates from the CA database directories.
    cp s_rsa_ca4_pub.pem         db/ca4/s_rsa_ca4.pem
    cp s_rsa_ca4_revoked_pub.pem db/ca4/s_rsa_ca4_revoked.pem
    cp s_rsa_cai4_pub.pem         db/ca4/intermediate1/s_rsa_cai4.pem
    cp s_rsa_cai4_revoked_pub.pem db/ca4/intermediate1/s_rsa_cai4_revoked.pem

    # ---- Revocation: CRLs (ca3/cai3) and the OCSP database index (ca4/cai4) ------------------
    revoke db/ca3 ca3.crl.pem s_rsa_ca3_revoked_pub.pem c_rsa_ca3_revoked_pub.pem db/ca3/intermediate1/ca.pem
    revoke db/ca3/intermediate1 cai3.crl.pem s_rsa_cai3_revoked_pub.pem
    cat ca3.crl.pem cai3.crl.pem > ca.crl.pem
    revoke db/ca4 ca4.crl.pem s_rsa_ca4_revoked_pub.pem db/ca4/intermediate1/ca.pem
    revoke db/ca4/intermediate1 cai4.crl.pem s_rsa_cai4_revoked_pub.pem

    # revocationutil.py loads the ca4 / cai4 CA keys with the password "password", so encrypt them
    # now that all the openssl 'ca' operations that need the plaintext keys are done.
    local cak
    for cak in db/ca4/ca_key.pem db/ca4/intermediate1/ca_key.pem; do
        openssl pkey -in "${cak}" -out "${cak}.enc" -aes256 -passout pass:"${DEFAULT_PASSWORD}"
        mv "${cak}.enc" "${cak}"
    done

    # revocationutil.py expects the OCSP database index to contain only revoked (R) entries.
    local idx
    for idx in db/ca4/index.txt db/ca4/intermediate1/index.txt; do
        grep '^R' "${idx}" > "${idx}.tmp" || true
        mv "${idx}.tmp" "${idx}"
    done

    # ---- Custom-extension certificate (self-signed, reuses the committed .req / .ext) ---------
    openssl req -new -noenc -newkey rsa:2048 -keyout cakey1.pem -out cacert_custom.csr -config cacert_custom.req
    openssl x509 -req -in cacert_custom.csr -signkey cakey1.pem -days "${DEFAULT_DAYS}" \
        -extfile cacert_custom.ext -out cacert_custom.pem
    rm -f cacert_custom.csr

    # ---- Diffie-Hellman parameters -----------------------------------------------------------
    openssl dhparam -outform DER -out dh_params512.der 512
    openssl dhparam -outform DER -out dh_params1024.der 1024

    # ---- Password-less certificates (empty PKCS12 password, separate CA) ---------------------
    new_root db/password_less "Password Less Tests CA"
    issue password_less_server db/password_less "Server" "${server_eku}
subjectAltName = IP:127.0.0.1, DNS:server" "" "" "" no
    issue password_less_client db/password_less "Client" "${client_eku}
subjectAltName = IP:127.0.0.1, DNS:client" "" "" "" no
    cp db/password_less/ca.pem password_less_cacert.pem
    openssl x509 -in password_less_cacert.pem -out password_less_cacert.der -outform DER
    # Strip the per-leaf key/cert pem we do not ship for the password-less certs.
    rm -f password_less_server_priv.pem password_less_server_pub.pem \
          password_less_client_priv.pem password_less_client_pub.pem

    # ---- Java keystores ----------------------------------------------------------------------
    local leaf
    for leaf in s_rsa_ca1 c_rsa_ca1 s_rsa_ca1_exp c_rsa_ca1_exp \
                s_rsa_ca1_cn1 s_rsa_ca1_cn2 s_rsa_ca1_cn3 s_rsa_ca1_cn4 \
                s_rsa_ca1_cn5 s_rsa_ca1_cn6 s_rsa_ca1_cn7 s_rsa_ca1_cn8 \
                s_rsa_ca2 c_rsa_ca2 s_rsa_cai1 s_rsa_cai2 c_rsa_cai2 s_rsa_wroot_ca1; do
        jks_key "${leaf}.p12" "${leaf}.jks"
    done
    jks_trust cacert1.jks ca:cacert1.pem
    jks_trust cacert2.jks ca:cacert2.pem
    jks_trust cacerts.jks cacert1:cacert1.pem cacert2:cacert2.pem
    jks_key cacert2.p12 s_cacert2.jks ca ca

    # ---- Cleanup -----------------------------------------------------------------------------
    # Keep only the artifacts the tests load; drop the openssl CA working files except those the
    # OCSP responder reads from db/ca4.
    find db -type d -name newcerts -exec rm -rf {} + 2>/dev/null || true
    find db -type f \( -name '*.cnf' -o -name '*.csr' -o -name 'serial*' -o -name 'crlnumber*' \
        -o -name 'index.txt.attr*' -o -name '*.old' \) \
        -exec rm -f {} + 2>/dev/null || true
    # Remove the db material for the CAs the OCSP responder does NOT read (it only reads db/ca4).
    rm -rf db/ca1 db/ca2 db/ca3 db/password_less db/ca4/crlnumber

    popd > /dev/null
}
