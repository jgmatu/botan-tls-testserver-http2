"""Starts the test server with a throwaway certificate for the Robot suite."""

import socket
import subprocess
import tempfile
import time
from pathlib import Path

from robot.api.deco import keyword
from robot.libraries.BuiltIn import BuiltIn

ROOT = Path(__file__).resolve().parents[2]


class TestServer:
    """One server process for the whole suite."""

    ROBOT_LIBRARY_SCOPE = "SUITE"

    def __init__(self):
        self._process = None
        self._workdir = None
        self._log = None

    @keyword
    def start_test_server(self, binary=None, policy=None, port=None):
        """Generate a localhost certificate, start testserver, and publish ${BASE_URL}.

        ``binary`` defaults to ``build/testserver`` under the repository root.
        ``policy`` defaults to ``policies/tls_hybrid_pqc_browser.txt``.
        ``port`` defaults to a free TCP port on 127.0.0.1.
        """
        binary_path = Path(binary) if binary else ROOT / "build" / "testserver"
        policy_path = Path(policy) if policy else ROOT / "policies" / "tls_hybrid_pqc_browser.txt"
        if not binary_path.is_file():
            raise RuntimeError(f"test server binary not found: {binary_path}")
        if not policy_path.is_file():
            raise RuntimeError(f"TLS policy not found: {policy_path}")

        listen_port = int(port) if port else _free_port()
        self._workdir = Path(tempfile.mkdtemp(prefix="testserver-e2e-"))
        key = self._workdir / "server.key"
        certificate = self._workdir / "server.pem"
        _generate_certificate(self._workdir, key, certificate)

        log_path = self._workdir / "server.log"
        self._log = log_path.open("w", encoding="utf-8")
        command = [
            str(binary_path),
            "--cert",
            str(certificate),
            "--key",
            str(key),
            "--port",
            str(listen_port),
            "--policy",
            str(policy_path),
            "--document-root",
            str(ROOT / "webroot"),
        ]
        self._process = subprocess.Popen(
            command,
            cwd=ROOT,
            stdout=self._log,
            stderr=subprocess.STDOUT,
        )
        try:
            _wait_until_accepting(listen_port, self._process, log_path)
        except Exception:
            self.stop_test_server()
            raise

        BuiltIn().set_suite_variable("${BASE_URL}", f"https://localhost:{listen_port}")

    @keyword
    def stop_test_server(self):
        """Stop the server process and delete its certificate directory."""
        if self._process is not None and self._process.poll() is None:
            self._process.terminate()
            try:
                self._process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                self._process.kill()
                self._process.wait(timeout=5)
        self._process = None
        if self._log is not None:
            self._log.close()
            self._log = None
        if self._workdir is not None:
            for path in self._workdir.iterdir():
                path.unlink(missing_ok=True)
            self._workdir.rmdir()
            self._workdir = None


def _free_port():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.bind(("127.0.0.1", 0))
        return sock.getsockname()[1]


def _generate_certificate(directory, key, certificate):
    ca_key = directory / "ca.key"
    ca_cert = directory / "ca.pem"
    request = directory / "server.req"
    _botan("keygen", "--algo=ECDSA", "--params=secp384r1", f"--output={ca_key}")
    _botan("keygen", "--algo=ECDSA", "--params=secp384r1", f"--output={key}")
    _botan(
        "gen_self_signed",
        "--ca",
        "--country=DE",
        "--dns=localhost",
        "--hash=SHA-384",
        f"--output={ca_cert}",
        str(ca_key),
        "localhost",
    )
    _botan("gen_pkcs10", f"--output={request}", str(key), "localhost")
    _botan("sign_cert", f"--output={certificate}", str(ca_cert), str(ca_key), str(request))


def _botan(*args):
    completed = subprocess.run(["botan", *args], check=False, capture_output=True, text=True)
    if completed.returncode != 0:
        raise RuntimeError(f"botan {' '.join(args)} failed: {completed.stderr.strip() or completed.stdout.strip()}")


def _wait_until_accepting(port, process, log_path, timeout=30):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        if process.poll() is not None:
            raise RuntimeError(f"test server exited early:\n{_tail(log_path)}")
        try:
            with socket.create_connection(("127.0.0.1", port), timeout=0.2):
                return
        except OSError:
            time.sleep(0.1)
    raise RuntimeError(f"test server did not accept connections:\n{_tail(log_path)}")


def _tail(path):
    if not path.exists():
        return ""
    lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
    return "\n".join(lines[-40:])
