#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>

TEST_GROUP(Sample_TestGroup)
{
  void teardown()
  {
    mock().clear();
  }
};

void foo()
{
  mock().actualCall("foo");
}

TEST(Sample_TestGroup, SimpleTest)
{
  CHECK(2 == 2);
}

TEST(Sample_TestGroup, SimpleMock)
{
  mock().expectOneCall("foo");
  foo();
  mock().checkExpectations();
}
