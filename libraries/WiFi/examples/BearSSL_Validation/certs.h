#pragma once

////////////////////////////////////////////////////////////
// certificate chain for www.akamai.com:443

const char* ssl_host = "www.akamai.com";
const uint16_t ssl_port = 443;

// openssl s_client -connect www.akamai.com:443  < /dev/null 2>/dev/null | openssl x509 -fingerprint -noout -in /dev/stdin | cut -f2 -d= | sed 's/^/const char fingerprint_ssl[] = "/' | sed 's/$/";/'
const char fingerprint_ssl[] = "D6:6C:EF:1A:8A:CE:27:18:D4:0C:83:2C:AE:4D:1B:B9:4C:B8:C9:83";

// openssl s_client -connect www.akamai.com:443 < /dev/null 2>/dev/null | openssl x509 -pubkey -noout
const char pubkey_ssl[] = R"PUBKEY(
-----BEGIN PUBLIC KEY-----
MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAE2BD04+A9idhYCHJ4RBy2KHtvJAsz
KLkAzZOWDEqjNu4plNmiWMymBw0rJggZOU4/TE+a4KqzBgsus9QkFZh88w==
-----END PUBLIC KEY-----
)PUBKEY";

// openssl s_client -connect www.akamai.com:443 < /dev/null 2> /dev/null | openssl x509

const char cert_CA[]  = R"CERT(
-----BEGIN CERTIFICATE-----
MIIGAzCCBOugAwIBAgIQA0bb8BLx/7BiUlnlhKlA6TANBgkqhkiG9w0BAQsFADBP
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMSkwJwYDVQQDEyBE
aWdpQ2VydCBUTFMgUlNBIFNIQTI1NiAyMDIwIENBMTAeFw0yNDA5MDMwMDAwMDBa
Fw0yNTA5MDMyMzU5NTlaMHYxCzAJBgNVBAYTAlVTMRYwFAYDVQQIEw1NYXNzYWNo
dXNldHRzMRIwEAYDVQQHEwlDYW1icmlkZ2UxIjAgBgNVBAoTGUFrYW1haSBUZWNo
bm9sb2dpZXMsIEluYy4xFzAVBgNVBAMTDnd3dy5ha2FtYWkuY29tMFkwEwYHKoZI
zj0CAQYIKoZIzj0DAQcDQgAE2BD04+A9idhYCHJ4RBy2KHtvJAszKLkAzZOWDEqj
Nu4plNmiWMymBw0rJggZOU4/TE+a4KqzBgsus9QkFZh886OCA30wggN5MB8GA1Ud
IwQYMBaAFLdrouqoqoSMeeq02g+YssWVdrn0MB0GA1UdDgQWBBR1CWtuGpgU+vEH
b6yp42l2leyozDAlBgNVHREEHjAcgg53d3cuYWthbWFpLmNvbYIKYWthbWFpLmNv
bTA+BgNVHSAENzA1MDMGBmeBDAECAjApMCcGCCsGAQUFBwIBFhtodHRwOi8vd3d3
LmRpZ2ljZXJ0LmNvbS9DUFMwDgYDVR0PAQH/BAQDAgOIMB0GA1UdJQQWMBQGCCsG
AQUFBwMBBggrBgEFBQcDAjCBjwYDVR0fBIGHMIGEMECgPqA8hjpodHRwOi8vY3Js
My5kaWdpY2VydC5jb20vRGlnaUNlcnRUTFNSU0FTSEEyNTYyMDIwQ0ExLTQuY3Js
MECgPqA8hjpodHRwOi8vY3JsNC5kaWdpY2VydC5jb20vRGlnaUNlcnRUTFNSU0FT
SEEyNTYyMDIwQ0ExLTQuY3JsMH8GCCsGAQUFBwEBBHMwcTAkBggrBgEFBQcwAYYY
aHR0cDovL29jc3AuZGlnaWNlcnQuY29tMEkGCCsGAQUFBzAChj1odHRwOi8vY2Fj
ZXJ0cy5kaWdpY2VydC5jb20vRGlnaUNlcnRUTFNSU0FTSEEyNTYyMDIwQ0ExLTEu
Y3J0MAwGA1UdEwEB/wQCMAAwggF+BgorBgEEAdZ5AgQCBIIBbgSCAWoBaAB2ABLx
TjS9U3JMhAYZw48/ehP457Vih4icbTAFhOvlhiY6AAABkbhvAhwAAAQDAEcwRQIh
AKw2mqyB5bKpwGN5Dn4rLL5pjsNY03bEnNpVXltFpOtDAiBmwhRQg/V+whwdynkK
OOyuChdAd2JH76KE+XISMjPM3gB1AObSMWNAd4zBEEEG13G5zsHSQPaWhIb7uocy
Hf0eN45QAAABkbhvAm8AAAQDAEYwRAIgOw/ZyiiIsgDbM55pvJmV/CvwA5AGWbpC
lB9q78TiJj4CIC2d9w9Ntb3HTOrLQvwSiNOrpop+deZ3fa1LQ92miaclAHcAzPsP
aoVxCWX+lZtTzumyfCLphVwNl422qX5UwP5MDbAAAAGRuG8CMAAABAMASDBGAiEA
oSSK9/26Uk3fqp7btK2SaEyWM/u++sOZXMeEOefnKwUCIQCqKysJztOF58PueXeE
pLliBWZUnjUrvPIbtxp31gQQITANBgkqhkiG9w0BAQsFAAOCAQEAAoJryZtJc8oc
+hf5vflkW2w0PdJ5NpbmjZOMTyutBRaJpxrDqKnjKBEOp6liZ2pDbUngGAwLEM30
KRBwJCc44u7w8Kj/KOoIZFoh9SiONAEn1qXWF0nFkYAD8J8yYwzkJy7fa3sdodaI
ex5NJosK/tyXjEBOcTrBsXkzs55pjSi+KJiiOHs/hDuBdyDtJs1ENAOswd8AWMSk
S9WycNxxEFH9VQgwgUhaI5Lc9J8rt3IJur1wp1JoDwdjIhs1M0rofgdW716UpmQ9
visdfGu8fCAlDSBPPC6gUPlFGFf4ujUmprJyBQ5xsYva0sVqhGbaoxtV7FdMDJkg
nDRma+9lPA==
-----END CERTIFICATE-----
)CERT";
