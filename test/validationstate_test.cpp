#include <gtest/gtest.h>

#include <string>

#include <veriblock/validation_state.hpp>

TEST(ValidationState, BasicFormatting) {
  altintegration::ValidationState state{};
  bool ret = state.Invalid("test", "Description");
  ASSERT_FALSE(ret);
  EXPECT_EQ(state.GetDebugMessage(), "Description");
  EXPECT_EQ(state.GetPath(), "test");
  EXPECT_EQ(state.GetPathParts().size(), 1);
}

TEST(ValidationState, MultipleFormatting) {
  altintegration::ValidationState state{};
  bool ret = state.Invalid("test", "Description");
  ret = state.Invalid("test2", "Description2");
  ASSERT_FALSE(ret);
  EXPECT_EQ(state.GetDebugMessage(), "Description2");
  EXPECT_EQ(state.GetPath(), "test2+test");
  EXPECT_EQ(state.GetPathParts().size(), 2);
}

TEST(ValidationState, IndexFormatting) {
  altintegration::ValidationState state{};
  bool ret = state.addIndex(1).Invalid("test", "Description");
  ASSERT_FALSE(ret);
  EXPECT_EQ(state.GetDebugMessage(), "Description");
  EXPECT_EQ(state.GetPath(), "test+1");
  EXPECT_EQ(state.GetPathParts().size(), 2);
}