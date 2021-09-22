# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.serialization import Encoding
from cryptography.x509 import ocsp, load_pem_x509_certificate, ReasonFlags, SubjectKeyIdentifier
from datetime import datetime, timedelta, timezone
from functools import partial
import base64
import http.server
import os
import sys
import traceback
import threading
import urllib.parse


def load_certificate(path):
    with open(path, 'rb') as f:
        return load_pem_x509_certificate(f.read())


def load_private_key(path, password):
    with open(path, 'rb') as f:
        return serialization.load_pem_private_key(f.read(), password)


def load_db(basepath):
    """
    create an in memory database of issuer/certificates and issuer/revocations
    the issuer SKI is used as the issuer key and the certificate serial number
    as the certificates and revocations key.
    """
    db = {}
    for ca_dir, certs in [("db/ca4", ["s_rsa_ca4.pem", "s_rsa_ca4_revoked.pem", "intermediate1/ca.pem"]),
                          ("db/ca4/intermediate1", ["s_rsa_cai4.pem", "s_rsa_cai4_revoked.pem"])]:
        ca_dir = os.path.join(basepath, ca_dir)
        issuer_cert = load_certificate("{}/ca.pem".format(ca_dir))
        issuer_key = load_private_key("{}/ca_key.pem".format(ca_dir), b"password")

        issuer_sha1 = issuer_cert.extensions.get_extension_for_class(SubjectKeyIdentifier).value.digest
        db[issuer_sha1] = {}
        db[issuer_sha1]['issuer_cert'] = issuer_cert
        db[issuer_sha1]['issuer_key'] = issuer_key

        certificates = {}
        for filename in certs:
            cert = load_certificate(os.path.join(ca_dir, filename))
            certificates[cert.serial_number] = cert
        db[issuer_sha1]['certificates'] = certificates

        # index.txt in the CA directory contains the openssl-ca database
        # see https://pki-tutorial.readthedocs.io/en/latest/cadb.html
        # in our case it only contains revocation info
        with open("{}/index.txt".format(ca_dir)) as index:
            revocations = {}
            lines = index.readlines()
            for line in lines:
                tokens = line.split('\t')
                if len(tokens) != 6:
                    print("invalid line\n" + line)
                    sys.exit(1)

                assert tokens[0] == 'R'
                certinfo = {
                    "revocation_time": datetime.strptime(tokens[2], "%y%m%d%H%M%S%z"),
                    "serial_number": int(tokens[3], 16),
                }
                revocations[certinfo["serial_number"]] = certinfo
            db[issuer_sha1]['revocations'] = revocations
    return db


class OCSPHandler(http.server.BaseHTTPRequestHandler):
    "A simple handlder for OCSP GET/POST requests"

    def __init__(self, db, *args, **kwargs):
        self._db = db
        # BaseHTTPRequestHandler calls do_GET **inside** __init__ !!!
        # So we have to call super().__init__ after setting attributes.
        super().__init__(*args, **kwargs)

    def do_POST(self):
        length = int(self.headers['Content-Length'])
        data = self.rfile.read(length)
        self.validate(data)

    def do_GET(self):
        # unquote the URL and then base64 decode it striping the first /
        data = base64.b64decode(urllib.parse.unquote(self.path[1:]))
        self.validate(data)

    def validate(self, data):
        response = None
        this_update = datetime.now(timezone.utc)
        next_update = this_update + timedelta(seconds=60)
        try:
            request = ocsp.load_der_ocsp_request(data)
            serial = request.serial_number
            issuer = self._db.get(request.issuer_key_hash)
            if issuer:
                issuer_cert = issuer.get('issuer_cert')
                issuer_key = issuer.get('issuer_key')
                subject_cert = issuer.get('certificates').get(serial)
                if subject_cert is None:
                    response = ocsp.OCSPResponseBuilder.build_unsuccessful(ocsp.OCSPResponseStatus.UNAUTHORIZED)
                else:
                    cert_info = issuer.get('revocations').get(serial)
                    revoked = cert_info is not None

                    builder = ocsp.OCSPResponseBuilder().add_response(
                        cert=subject_cert,
                        issuer=issuer_cert,
                        algorithm=hashes.SHA1(),
                        cert_status=ocsp.OCSPCertStatus.REVOKED if revoked else ocsp.OCSPCertStatus.GOOD,
                        this_update=this_update,
                        next_update=next_update,
                        revocation_time=cert_info['revocation_time'] if revoked else None,
                        revocation_reason=ReasonFlags.unspecified if revoked else None)

                    builder = builder.responder_id(ocsp.OCSPResponderEncoding.HASH, issuer_cert)
                    response = builder.sign(issuer_key, hashes.SHA256())
            else:
                response = ocsp.OCSPResponseBuilder.build_unsuccessful(ocsp.OCSPResponseStatus.UNAUTHORIZED)
        except Exception:
            traceback.print_exc(file=sys.stdout)
            response = ocsp.OCSPResponseBuilder.build_unsuccessful(ocsp.OCSPResponseStatus.INTERNAL_ERROR)

        self.send_response(200)
        self.send_header("Content-Type", "application/ocsp-response")
        self.end_headers()
        self.wfile.write(response.public_bytes(Encoding.DER))

def createOCSPServer(host, port, basepath):
    db = load_db(basepath)
    handler = partial(OCSPHandler, db)
    return ThreadedServer(host, port, handler)


def createCRLServer(host, port, basepath,):
    handler = partial(http.server.SimpleHTTPRequestHandler, directory=basepath)
    return ThreadedServer(host, port, handler)


class ThreadedServer:
    # run HTPPServer in its own thread

    def __init__(self, hostname, port, handler):
        self.handler = handler
        self.server = http.server.HTTPServer((hostname, port), handler)
        self.thread = None

    def start(self):
        def serve_forever(server):
            with server:
                server.serve_forever()

        self.thread = threading.Thread(target=serve_forever, args=(self.server,))
        self.thread.setDaemon(True)
        self.thread.start()

    def shutdown(self):
        self.server.shutdown()
        self.thread.join()
