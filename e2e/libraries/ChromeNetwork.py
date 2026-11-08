"""Read HTTP/2 and TLS 1.3 from the Chromium session SeleniumLibrary already opened."""

import json
from urllib.parse import urlsplit

from robot.api.deco import keyword
from robot.libraries.BuiltIn import BuiltIn


class ChromeNetwork:
    """Checks the DevTools performance log of the current SeleniumLibrary browser."""

    @keyword
    def responses_should_be_http2_over_tls_1_3(self, *urls):
        """Fail unless every response for each URL is HTTP/2 (``h2``) over TLS 1.3.

        Reads ``Network.responseReceived`` from the performance log of the browser
        opened by SeleniumLibrary. The log is consumed by this call. WebSocket
        requests are not part of ``urls`` and are not required to be HTTP/2.
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
                security = response.get("securityDetails") or {}
                if protocol != "h2":
                    raise AssertionError(f"{response['url']} used {protocol!r}, expected 'h2'")
                tls = security.get("protocol")
                if not tls:
                    raise AssertionError(
                        f"{response['url']} has no securityDetails.protocol, so TLS was not observed"
                    )
                if tls != "TLS 1.3":
                    raise AssertionError(f"{response['url']} used {tls!r}, expected 'TLS 1.3'")


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
