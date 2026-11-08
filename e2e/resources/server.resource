*** Settings ***
Library    Process
Library    OperatingSystem

*** Variables ***
${PORT}         50443
${CERT_DIR}     /tmp/testserver-e2e-certs
${ROOT}         ${CURDIR}/../..

*** Keywords ***
Start Test Server
    [Documentation]    Throwaway localhost certificate, then testserver on port 50443.
    Create Directory    ${CERT_DIR}
    Run    botan keygen --algo=ECDSA --params=secp384r1 --output=${CERT_DIR}/ca.key
    Run    botan keygen --algo=ECDSA --params=secp384r1 --output=${CERT_DIR}/server.key
    Run    botan gen_self_signed --ca --country=DE --dns=localhost --hash=SHA-384 --output=${CERT_DIR}/ca.pem ${CERT_DIR}/ca.key localhost
    Run    botan gen_pkcs10 --output=${CERT_DIR}/server.req ${CERT_DIR}/server.key localhost
    Run    botan sign_cert --output=${CERT_DIR}/server.pem ${CERT_DIR}/ca.pem ${CERT_DIR}/ca.key ${CERT_DIR}/server.req
    Start Process    ${ROOT}/build/testserver
    ...    --cert    ${CERT_DIR}/server.pem
    ...    --key    ${CERT_DIR}/server.key
    ...    --port    ${PORT}
    ...    --policy    ${ROOT}/policies/tls_hybrid_pqc_browser.txt
    ...    --document-root    ${ROOT}/webroot
    ...    cwd=${ROOT}
    ...    stdout=${CERT_DIR}/server.log
    ...    stderr=STDOUT
    Wait Until Keyword Succeeds    30s    0.2s    Port Is Open    ${PORT}
    Set Suite Variable    ${BASE_URL}    https://localhost:${PORT}

Stop Test Server
    Run Keyword And Ignore Error    Terminate Process
    Run Keyword And Ignore Error    Wait For Process    timeout=5s
    Run Keyword And Ignore Error    Remove Directory    ${CERT_DIR}    recursive=True

Port Is Open
    [Arguments]    ${port}
    Run    bash -c 'echo >/dev/tcp/127.0.0.1/${port}'
