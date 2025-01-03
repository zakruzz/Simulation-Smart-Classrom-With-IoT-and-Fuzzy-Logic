#include <pgmspace.h>
 
#define SECRET
#define THINGNAME "iotprivate"                         //change this
 
const char WIFI_SSID[] = "zaki";               //change this
const char WIFI_PASSWORD[] = "ojokdibagi";           //change this
const char AWS_IOT_ENDPOINT[] = "a82xtr20xi4cb-ats.iot.ap-southeast-1.amazonaws.com";       //change this
 
// Amazon Root CA 1
static const char AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)EOF";
 
// Device Certificate                                               //change this
static const char AWS_CERT_CRT[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
MIIDWjCCAkKgAwIBAgIVAJNxpIQCAJLs6oTau66Cy2AJAaeOMA0GCSqGSIb3DQEB
CwUAME0xSzBJBgNVBAsMQkFtYXpvbiBXZWIgU2VydmljZXMgTz1BbWF6b24uY29t
IEluYy4gTD1TZWF0dGxlIFNUPVdhc2hpbmd0b24gQz1VUzAeFw0yNDEyMzEwNjUw
MzdaFw00OTEyMzEyMzU5NTlaMB4xHDAaBgNVBAMME0FXUyBJb1QgQ2VydGlmaWNh
dGUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDV0CeK7uR3u6bY/svD
8SjxXXp0LtNQHKxBL2mxBPFnbi+g+xxIuGKLmou/fYoIiUZzoVbAX3RN9SueoXk6
3shRkcxK34HSuZDQDM3JXRDzquySvBVD36vHlqQ1YrEqEDaKgq6OB7seMGHwgXkm
Ts4TV1RGkmPrzlySKZEoAnAxFaw7nxZnQYYNalRJ9TmAm12GGWNeedoCXb1HGkBD
XE7XyF16qsjZt86bvWqg0LNeC98g8s4FuiC2eCcradUkQ5Axp6OF5E01JhewjRRw
h5kJyqlwBJ38ztQ1AXXTfbmaAJp5v9aa7MqbNe/cFRd0l611f3JREwc2zBT8dRcA
bETFAgMBAAGjYDBeMB8GA1UdIwQYMBaAFEmERr9vh9pJD9kgLOOvlRoH3YAAMB0G
A1UdDgQWBBQjnlqYWkEXlzsv34ZhkkJLFcdw2TAMBgNVHRMBAf8EAjAAMA4GA1Ud
DwEB/wQEAwIHgDANBgkqhkiG9w0BAQsFAAOCAQEAbFHRdyPGho3LB7Dsdp587FIa
CQtkPn8DZIy3+O2jRm2/Wg6bwyy0tZhC/RaSpOtG7LgKNR/pBl9iTD7BP5z4Bnu8
OV0fDMjfn/uO5TS3UwT15UF5rtVyqNb/BujNZzRIDGyqf2R6Ec8LNnmhoJasUaCJ
VGQOa+tqsNzNy6c8CGZcLTNYhZPovE9A4qlyYwBxocGwAQPcfW7pADhfayA5vTfL
7xZhasPyhQT3teLwCsjo+DYlypSG9I3y82qSh/trSx4aUA9oaO9Gh5edEK/lz/Pr
Q8xmhRXozQ34fpO6jG7RbPdBIWB/rzuFSTraZFHQ+FgYNN+QLQI6B16BHsVPvg==
-----END CERTIFICATE-----
)KEY";
 
// Device Private Key                                               //change this
static const char AWS_CERT_PRIVATE[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
MIIEowIBAAKCAQEA1dAniu7kd7um2P7Lw/Eo8V16dC7TUBysQS9psQTxZ24voPsc
SLhii5qLv32KCIlGc6FWwF90TfUrnqF5Ot7IUZHMSt+B0rmQ0AzNyV0Q86rskrwV
Q9+rx5akNWKxKhA2ioKujge7HjBh8IF5Jk7OE1dURpJj685ckimRKAJwMRWsO58W
Z0GGDWpUSfU5gJtdhhljXnnaAl29RxpAQ1xO18hdeqrI2bfOm71qoNCzXgvfIPLO
BbogtngnK2nVJEOQMaejheRNNSYXsI0UcIeZCcqpcASd/M7UNQF10325mgCaeb/W
muzKmzXv3BUXdJetdX9yURMHNswU/HUXAGxExQIDAQABAoIBAB/nXm8htF+VVZEd
e+Qx8O0LCMGbVxamIKHdZETwuuC3FU5NCexj7vhryBx7rAQYD7YjReGQnLchNTAH
YiKBqZxaSASIE0QRQmV485/vTh89yu0eOYIWNviiJsB8iHkBUFIcq9UvnCEmEQcl
GqG3spI6Pq4FG/nMog2BPFKfAoSgNGsOP54ZMbhlUa/a6CWttHtIWrIaGSzLmyWZ
py3ckDngvxqEgyXUGrtDJiMqLbpJxOwt/rdHWW4Xhjvm1Ed5hbO0umy1MJ9q1kF0
cOMtiu3LCf6eL6OTHL/oQlbXj0aWrme7hPSlp8HNr2mDzTwxxorSrDMQiaZ0FXzS
am0K9cUCgYEA74ElKUDPVSkOuhbx/mx07le62Fu7qev/+pCrA5UkCq1bG1pWWmAE
bWa9kR8ZXbqcO7cDl2FbkAr/FacxqFgW1L54eXkj69DUd4cvJRCz4cdkvxtQamN/
zJR0afkaEK76e/CZLaYWt2a9jAkv6j25X0sLmXQBcpzGewGyRhkl1pcCgYEA5IoH
TI/YhxUgnrKJ+MDNL/yHl27/ISsa+wsgmjqsp17xEVlHOFr8liFlElUFZtoIEgux
njkxVhIYEYFdQ+e+9gFf9ShhQ99srTeQSyqRj1QLFNPf4wcOQW3KYE1rpCZRxoM8
lGAtrxzEYW+y+QOwZSKKfbDIDWp2YVTlcZn8ZwMCgYA24SfLuR8JC3xlZaaTvvRY
POACfZqVyA8Csi2Nh5oXnDt5HA0XblL1Yopje3XLhVwRTtFS2asqaWqj4oi9Y1nv
66kdZIW3nkVhEifaXWZqgM7z/2VAy4XYMdgvBDZVxlJ9Hddrn1grNC6QrPY+vWAB
lueUhQFj6sqMGwpnh5GlBwKBgHt3joXwWJxx75ONBTkcgD3igiZv0unKbeMemdh4
xnoNstMaln19umQpWn9S634rYJzPMIA4iczkLZiXeRmMhABHsd2ymQmgFQhnsk7J
iqYSU4JHDPeMAkma1PC+It8lzJwgnoPSihvGwNmbDOe29H9O798DuLB2ycm/oc2D
0XadAoGBAMwGk5S+cEhsuu47z+Z57JqmCeaoP3mAmBYbGugngifCvwIQBT7GOEzO
At55ckQHtEF+bS+Vz7W8HA1Cwbe3zyq5vqtb93aIgHh3vMeSv9rK/07eQIe4oZGQ
rekPxgXSZmImIETNEpQMYNoJqtmf/FLlrwXSZfI5qTJkwzRCO4Oz
-----END RSA PRIVATE KEY-----
)KEY";
