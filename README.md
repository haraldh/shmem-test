```shell
❯ docker build -t shmem-test .

❯ docker run --rm -it --init --privileged shmem-test shmem-test
Starting 100 tests with filename test.dat
[PASS] No errors detected.

❯ docker run --rm -it --init --privileged shmem-test "gramine-direct shmem-test"
[P1:T1:shmem-test] warning: Unsupported system call rseq
Starting 100 tests with filename test.dat
Detected 100 times (100.000%) when file contents were incorrect.
Detected 100 times (100.000%) when mapping was incorrect.

❯ docker run --rm -it --init --privileged shmem-test "gramine-sgx shmem-test"
Gramine is starting. Parsing TOML manifest file, this may take some time...
-----------------------------------------------------------------------------------------------------------------------
Gramine detected the following insecure configurations:

  - loader.log_level = warning|debug|trace|all (verbose log level, may leak information)
  - fs.insecure__keys.* = "..."                (keys hardcoded in manifest)

Gramine will continue application execution, but this configuration must not be used in production!
-----------------------------------------------------------------------------------------------------------------------

[P1:T1:shmem-test] warning: Unsupported system call rseq
Starting 100 tests with filename test.dat
Detected 100 times (100.000%) when file contents were incorrect.
Detected 100 times (100.000%) when mapping was incorrect.
```
