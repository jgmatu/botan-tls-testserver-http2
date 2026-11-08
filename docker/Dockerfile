# One container for the end-to-end suite: the server and headless Chromium
# share the network namespace and talk over localhost.
FROM debian:bookworm

ENV DEBIAN_FRONTEND=noninteractive \
    PATH="/opt/e2e-venv/bin:/usr/local/bin:${PATH}"

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        cmake \
        curl \
        pkg-config \
        python3 \
        python3-venv \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src

# Dependencies change far less often than the suite, so install them first.
COPY 3rdparty/install.sh 3rdparty/install.sh
RUN ./3rdparty/install.sh

COPY . .
RUN cmake -B build -S . -DBUILD_TESTING=OFF \
    && cmake --build build -j"$(nproc)"

# Debian packages Chromium and a matching chromedriver. Ubuntu's chromium
# package is a snap stub and does not start in a container.
RUN apt-get update && apt-get install -y --no-install-recommends \
        chromium \
        chromium-driver \
    && rm -rf /var/lib/apt/lists/* \
    && python3 -m venv /opt/e2e-venv \
    && /opt/e2e-venv/bin/pip install --no-cache-dir -r e2e/requirements.txt

# The pqc tag needs Chromium 131 or newer, which bookworm may not ship.
CMD ["robot", "--exclude", "pqc", "--outputdir", "/tmp/e2e-report", "e2e/tests"]
