#include <gtest/gtest.h>

class FilteringTestsFixture : public testing::Test
{
public:
    FilteringTestsFixture() { };
    ~FilteringTestsFixture() { };    

protected:
    void SetUp() override { }
    void TearDown() override { }
};
    
TEST_F(FilteringTestsFixture, Test01)
{
    EXPECT_EQ(1, 2);
}
