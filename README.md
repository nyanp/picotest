picotest : subset of googletest, single-header simple framework
========

picotest is a single header unit testing framework.

it has...
-----

**following macros**

- EXPECT_TRUE/ASSERT_TRUE
- EXPECT_FALSE/ASSERT_FALSE
- EXPECT_EQ/ASSERT_EQ
- EXPECT_NE/ASSERT_NE
- EXPECT_LT/ASSERT_LT
- EXPECT_GT/ASSERT_GT
- EXPECT_LE/ASSERT_LE
- EXPECT_GE/ASSERT_GE
- EXPECT_STREQ/ASSERT_STREQ
- EXPECT_STRNE/ASSERT_STRNE
- EXPECT_STRCASEEQ/ASSERT_STRCASEEQ
- EXPECT_STRCASENE/ASSERT_STRCASENE
- EXPECT_FLOAT_EQ/ASSERT_FLOAT_EQ
- EXPECT_DOUBLE_EQ/ASSERT_DOUBLE_EQ
- EXPECT_FLOAT_NE/ASSERT_FLOAT_EQ
- EXPECT_DOUBLE_NE/ASSERT_DOUBLE_NE
- EXPECT_NEAR

floating-point macros provides comparing in terms of ULPs (same to googletest).

**auto-registered-test, test-fixture**

- TEST(test_case_name, test_name)
- TEST_F(test_case_name, test_name)
- RUN_ALL_TESTS()

it hasn't...
----
- Windows HRESULT assertions
- Type Assertions
- Death Tests
- SCOPED_TRACE
- HasFatalFailure()
- RecordProperty()
- Global setup/teardown
- Value Parameterized Tests
- Typed Tests
- FRIEND_TEST()
- Failures Catching
- Controlling Test Output

sample
----

```cpp
#include "this/package/foo.h"
#include "picotest.h"

namespace {

// The fixture for testing class Foo.
class FooTest : public ::testing::Test {
 protected:
  // You can remove any or all of the following functions if its body
  // is empty.

  FooTest() {
    // You can do set-up work for each test here.
  }

  virtual ~FooTest() {
    // You can do clean-up work that doesn't throw exceptions here.
  }

  // If the constructor and destructor are not enough for setting up
  // and cleaning up each test, you can define the following methods:

  virtual void SetUp() {
    // Code here will be called immediately after the constructor (right
    // before each test).
  }

  virtual void TearDown() {
    // Code here will be called immediately after each test (right
    // before the destructor).
  }

  // Objects declared here can be used by all tests in the test case for Foo.
};

// Tests that the Foo::Bar() method does Abc.
TEST_F(FooTest, MethodBarDoesAbc) {
  const string input_filepath = "this/package/testdata/myinputfile.dat";
  const string output_filepath = "this/package/testdata/myoutputfile.dat";
  Foo f;
  EXPECT_EQ(0, f.Bar(input_filepath, output_filepath));
}

// Tests that Foo does Xyz.
TEST_F(FooTest, DoesXyz) {
  // Exercises the Xyz feature of Foo.
}

}  // namespace

int main(int argc, char **argv) {
  RUN_ALL_TESTS();
}
```
