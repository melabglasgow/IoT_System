/**********************************************************************************************
***********************************************************************************************
                Device Identity Certificates: Modify for your AWS IoT Thing
***********************************************************************************************
***********************************************************************************************/
 
/****************************************
(somecode)-certificate.pem.crt - Amazon signed PEM sertificate.
*****************************************/
 
//This Client cert is example. Use own instead.
const uint8_t clientCRT[] = "\
-----BEGIN CERTIFICATE-----\n\
MIIDBjCCAe6gAwIBAgIUVph856omeIxW3UPioq+UrX1DbwowDQYJKoZIhvcNAQEL\
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g\
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTE3MDUyNTExNTEy\
OVoXDTQ5MTIzMTIzNTk1OVowgZUxCzAJBgNVBAYTAkJZMQ4wDAYDVQQIDAVNaW5z\
azEOMAwGA1UEBwwFTWluc2sxFzAVBgNVBAoMDktsaWthLVRlY2ggTExDMRcwFQYD\
VQQLDA5LbGlrYS1UZWNoIExMQzEMMAoGA1UEAwwDUm5EMSYwJAYJKoZIhvcNAQkB\
FhdtdmF0YWxldUBrbGlrYS10ZWNoLmNvbTBZMBMGByqGSM49AgEGCCqGSM49AwEH\
A0IABCJgOQJmoTBJVPfli9Hm/JVixaxkY5rtlgrYO3hSl633A2hg0P/ue0wXDbF3\
aQ0X57IRFE4k4FEbr3UXjT/IczKjYDBeMB8GA1UdIwQYMBaAFK3YzTUPlYB2Li75\
i/z8rEogr1d6MB0GA1UdDgQWBBT18HXBaXFJuAR/0SwegnxJ+pyJ6TAMBgNVHRMB\
Af8EAjAAMA4GA1UdDwEB/wQEAwIHgDANBgkqhkiG9w0BAQsFAAOCAQEAb0Ux1aH5\
RLxjrfGqXN6rPVqh8QQRS+AyBfzmaQN8HaPZMkX5WxXLvcn0A3uWlwQxPPkcZ4zf\
51GHtFFQWB4YZ8dx8mUQ0v/j7onHjCJgZ8iDgwOyKMGtnsDZWCakQw+a6cj+NrMZ\
tzhjwCzEEP6ePcbXwErI5OOzLuWns2L/JEr2wWNkokgRuS8ewr/SQ9OLWIWa2rFM\
ahPNTb3y/qBeWdjeJmhI+TOxdqIpsF8roWP25zwo/zkzCHCjXFBrL+0CA4MpxIl9\
x02i7aAhlJ6ys80lDxdeWeeQJXRKkGknP8mcmKn3iEqqJ5s1dQePj2b5d3ldatya\
wsxQBqqZXzIWEw==\
\n\
-----END CERTIFICATE-----\n";
 
 
 
/**********************************************************************************************
***********************************************************************************************
                        Private Key: Modify for your AWS IoT Thing
***********************************************************************************************
***********************************************************************************************/
 
/********************************************************************8****************************************
nucleo.key.pem - client key generated according to readme.
**************************************************************************************************************/
 
//This Client Key is example. Use own instead.
const uint8_t clientKey[] ="\
-----BEGIN EC PARAMETERS-----\n\
BggqhkjOPQMBBw==\
-----END EC PARAMETERS-----\n\
-----BEGIN EC PRIVATE KEY-----\n\
MHcCAQEEIHPRfWSC8/k/BsqDWKuP15dXsI9fGwpkTIsLZe6mIrAAoAoGCCqGSM49\
AwEHoUQDQgAEImA5AmahMElU9+WL0eb8lWLFrGRjmu2WCtg7eFKXrfcDaGDQ/+57\
TBcNsXdpDRfnshEUTiTgURuvdReNP8hzMg==\
-----END EC PRIVATE KEY-----\n";