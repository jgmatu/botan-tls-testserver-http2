*** Settings ***
Library    SeleniumLibrary

*** Variables ***
${CHROMIUM_BIN}     %{CHROMIUM_BIN=/usr/bin/chromium}
${CHROMEDRIVER}     %{CHROMEDRIVER=/usr/bin/chromedriver}

*** Keywords ***
Open Test Browser
    [Documentation]    Headless Chromium via SeleniumLibrary. A container has no display and no sandbox.
    ${options}=    Catenate    SEPARATOR=;
    ...    add_argument("--headless=new")
    ...    add_argument("--ignore-certificate-errors")
    ...    add_argument("--allow-insecure-localhost")
    ...    add_argument("--no-sandbox")
    ...    add_argument("--disable-dev-shm-usage")
    ...    binary_location="${CHROMIUM_BIN}"
    ...    set_capability("goog:loggingPrefs", {"performance": "ALL"})
    ...    add_experimental_option("perfLoggingPrefs", {"enableNetwork": True, "enablePage": False})
    Open Browser    browser=chrome    options=${options}    service=executable_path="${CHROMEDRIVER}"
