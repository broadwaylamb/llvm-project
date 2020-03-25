# RUN: not %{lit} -j1 %{inputs}/setup-teardown | FileCheck %s

# CHECK: -- Testing: 2 tests, 1 workers --
# CHECK: Running setup code...
# CHECK: PASS: setup-teardown :: test1.txt
# CHECK: FAIL: setup-teardown :: test2.txt
# CHECK: Running teardown code...
# CHECK: Expected Passes    : 1
# CHECK: Unexpected Failures: 1
