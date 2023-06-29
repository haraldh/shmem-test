FROM gramineproject/gramine:v1.4

RUN env DEBIAN_FRONTEND=noninteractive apt-get update -y  \
    && env DEBIAN_FRONTEND=noninteractive apt-get install --no-install-recommends -y -q \
        build-essential \
        cmake ninja-build

WORKDIR /shmem-test
COPY . ./

RUN rm -fr cmake-build-debug && cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_MAKE_PROGRAM=ninja -G Ninja -S ./ -B ./cmake-build-debug \
    && cmake --build ./cmake-build-debug --target shmem-test \
    && cp ./cmake-build-debug/shmem-test /bin/shmem-test

WORKDIR /opt/shmem-test
COPY shmem-test.manifest.template ./

RUN gramine-sgx-gen-private-key \
    && gramine-manifest -Darch_libdir=/lib/x86_64-linux-gnu -Dexecdir=/usr/bin -Dlog_level=warning shmem-test.manifest.template shmem-test.manifest \
    && gramine-sgx-sign --manifest shmem-test.manifest --output shmem-test.manifest.sgx
