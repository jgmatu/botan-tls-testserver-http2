*** Settings ***
Resource         ../resources/browser.resource
Resource         ../resources/server.resource
Library          ../libraries/ChromeNetwork.py
Suite Setup      Start Test Server
Suite Teardown   Stop Test Server
Test Setup       Open Test Browser
Test Teardown    Close All Browsers

*** Test Cases ***
The Page And The API Use HTTP/2 Over TLS 1.3
    Go To    ${BASE_URL}/
    Wait Until Element Does Not Contain    id=kex_algo    unknown    timeout=15s
    Wait Until Keyword Succeeds    15s    500ms    Element Text Should Be    id=ws_counter    1
    ${shown}=    Get Text    id=kex_algo
    ${api}=    Execute Async Javascript
    ...    var callback = arguments[arguments.length - 1];
    ...    fetch('/api/connection_details').then(function (response) { return response.json(); }).then(callback);
    Should Be Equal    ${shown}    ${api}[kex_algo]
    Responses Should Be HTTP2 Over TLS 1 3    ${BASE_URL}/    ${BASE_URL}/api/connection_details

Chromium Negotiates The Hybrid ML-KEM Group
    [Tags]    pqc
    Go To    ${BASE_URL}/
    Wait Until Element Does Not Contain    id=kex_algo    unknown    timeout=15s
    Element Text Should Be    id=kex_algo    x25519/ML-KEM-768
    Element Text Should Be    id=kex_algo_type    (quantum secure)
