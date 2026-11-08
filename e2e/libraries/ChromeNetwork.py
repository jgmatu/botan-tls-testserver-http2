"""Chromium, headed by Selenium, and the DevTools check for HTTP/2 over TLS 1.3."""

import json
import os
import shutil
from urllib.parse import urlsplit

from robot.api.deco import keyword
from robot.libraries.BuiltIn import BuiltIn
from selenium import webdriver
from selenium.webdriver.chrome.options import Options
from selenium.webdriver.chrome.service import Service

# A container has no desktop sandbox, so Chromium cannot use its setuid sandbox.
CHROMIUM_ARGUMENTS = (
    "--headless=new",
    "--ignore-certificate-errors",
    "--allow-insecure-localhost",
    "--no-sandbox",
    "--disable-dev-shm-usage",
)


class ChromeNetwork:
    """Opens Chromium with a performance log and reads the protocol of each response."""

    @keyword
    def open_test_browser(self):
        """Start headless Chromium. SeleniumLibrary keywords use this browser afterwards."""
        options = Options()
        options.binary_location = _executable("CHROMIUM_BIN", ("chromium", "chromium-browser"))
        for argument in CHROMIUM_ARGUMENTS:
            options.add_argument(argument)
        options.set_capability("goog:loggingPrefs", {"performance": "ALL"})
        options.add_experimental_option("perfLoggingPrefs", {"enableNetwork": True, "enablePage": False})
        service = Service(_executable("CHROMEDRIVER", ("chromedriver",)))
        seleniumlib = BuiltIn().get_library_instance("SeleniumLibrary")
        seleniumlib.create_webdriver("Chrome", options=options, service=service)

    @keyword
    def responses_should_be_http2_over_tls_1_3(self, *urls):
        """Fail unless every response for each URL is HTTP/2 (``h2``) over TLS 1.3.

        The performance log is consumed by this call. WebSocket requests are not
        part of ``urls`` and are not required to be HTTP/2.
        """
        driver = BuiltIn().get_library_instance("SeleniumLibrary").driver
        responses = _responses(driver.get_log("performance"))
        seen = sorted({response["url"] for response in responses})
        for url in urls:
            matched = [response for response in responses if _same_request(response["url"], url)]
            if not matched:
                raise AssertionError(f"no network response for {url}; saw:\n" + "\n".join(seen))
            for response in matched:
                protocol = response.get("protocol")
                security = response.get("securityDetails")
                if protocol != "h2":
                    raise AssertionError(f"{response['url']} used {protocol!r}, expected 'h2'")
                if not security or "protocol" not in security:
                    raise AssertionError(
                        f"{response['url']} has no securityDetails.protocol, so TLS was not observed"
                    )
                if security["protocol"] != "TLS 1.3":
                    raise AssertionError(
                        f"{response['url']} used {security['protocol']!r}, expected 'TLS 1.3'"
                    )


def _executable(env_name, names):
    configured = os.environ.get(env_name)
    if configured:
        return configured
    for name in names:
        found = shutil.which(name)
        if found:
            return found
    raise RuntimeError(f"could not find {' or '.join(names)}")


def _responses(entries):
    responses = []
    for entry in entries:
        try:
            message = json.loads(entry["message"])["message"]
        except (KeyError, TypeError, json.JSONDecodeError):
            continue
        if message.get("method") != "Network.responseReceived":
            continue
        response = message.get("params", {}).get("response")
        if response and response.get("url"):
            responses.append(response)
    return responses


def _same_request(left, right):
    return _identity(left) == _identity(right)


def _identity(url):
    parsed = urlsplit(url)
    path = parsed.path or "/"
    if path != "/":
        path = path.rstrip("/")
    return parsed.scheme, parsed.netloc, path
