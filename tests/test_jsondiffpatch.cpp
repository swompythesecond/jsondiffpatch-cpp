#include "test_framework.h"
#include "../include/JsonDiffPatch/JsonDiffPatch.h"

using json = nlohmann::json;

TestRunner globalTestRunner;

// Test basic object diff
TEST(BasicObjectDiff) {
    JsonDiffPatch::JsonDiffPatch jdp;
    
    json left = {{"x", 1}, {"y", 2}};
    json right = {{"x", 1}, {"y", 3}};
    
    json diff = jdp.Diff(left, right);
    
    ASSERT_FALSE(diff.is_null());
    ASSERT_TRUE(diff.contains("y"));
    ASSERT_TRUE(diff["y"].is_array());
    ASSERT_EQ(diff["y"][0], 2);
    ASSERT_EQ(diff["y"][1], 3);
}

// Test object addition
TEST(ObjectAddition) {
    JsonDiffPatch::JsonDiffPatch jdp;
    
    json left = {{"x", 1}};
    json right = {{"x", 1}, {"y", 2}};
    
    json diff = jdp.Diff(left, right);
    
    ASSERT_FALSE(diff.is_null());
    ASSERT_TRUE(diff.contains("y"));
    ASSERT_TRUE(diff["y"].is_array());
    ASSERT_EQ(diff["y"].size(), 1);
    ASSERT_EQ(diff["y"][0], 2);
}

// Test object deletion
TEST(ObjectDeletion) {
    JsonDiffPatch::JsonDiffPatch jdp;
    
    json left = {{"x", 1}, {"y", 2}};
    json right = {{"x", 1}};
    
    json diff = jdp.Diff(left, right);
    
    ASSERT_FALSE(diff.is_null());
    ASSERT_TRUE(diff.contains("y"));
    ASSERT_TRUE(diff["y"].is_array());
    ASSERT_EQ(diff["y"].size(), 3);
    ASSERT_EQ(diff["y"][0], 2);
    ASSERT_EQ(diff["y"][1], 0);
    ASSERT_EQ(diff["y"][2], JsonDiffPatch::OP_DELETED);
}

// Test no changes
TEST(NoChanges) {
    JsonDiffPatch::JsonDiffPatch jdp;
    
    json left = {{"x", 1}, {"y", 2}};
    json right = {{"x", 1}, {"y", 2}};
    
    json diff = jdp.Diff(left, right);
    
    ASSERT_TRUE(diff.is_null());
}

// Test patch application
TEST(BasicPatch) {
    JsonDiffPatch::JsonDiffPatch jdp;
    
    json left = {{"x", 1}, {"y", 2}};
    json right = {{"x", 1}, {"y", 3}};
    
    json diff = jdp.Diff(left, right);
    json patched = jdp.Patch(left, diff);
    
    ASSERT_EQ(patched, right);
}

// Test unpatch application
TEST(BasicUnpatch) {
    JsonDiffPatch::JsonDiffPatch jdp;
    
    json left = {{"x", 1}, {"y", 2}};
    json right = {{"x", 1}, {"y", 3}};
    
    json diff = jdp.Diff(left, right);
    json unpatched = jdp.Unpatch(right, diff);
    
    ASSERT_EQ(unpatched, left);
}

// Test array addition
TEST(ArrayAddition) {
    JsonDiffPatch::JsonDiffPatch jdp;
    
    json left = json::array({1, 2});
    json right = json::array({1, 2, 3});
    
    json diff = jdp.Diff(left, right);
    
    ASSERT_FALSE(diff.is_null());
    ASSERT_TRUE(diff.contains("_t"));
    ASSERT_EQ(diff["_t"], "a");
    ASSERT_TRUE(diff.contains("2"));
    ASSERT_TRUE(diff["2"].is_array());
    ASSERT_EQ(diff["2"].size(), 1);
    ASSERT_EQ(diff["2"][0], 3);
}

// Test array deletion
TEST(ArrayDeletion) {
    JsonDiffPatch::JsonDiffPatch jdp;
    
    json left = json::array({1, 2, 3});
    json right = json::array({1, 2});
    
    json diff = jdp.Diff(left, right);
    
    ASSERT_FALSE(diff.is_null());
    ASSERT_TRUE(diff.contains("_t"));
    ASSERT_EQ(diff["_t"], "a");
    ASSERT_TRUE(diff.contains("_2"));
    ASSERT_TRUE(diff["_2"].is_array());
    ASSERT_EQ(diff["_2"].size(), 3);
    ASSERT_EQ(diff["_2"][0], 3);
    ASSERT_EQ(diff["_2"][1], 0);
    ASSERT_EQ(diff["_2"][2], JsonDiffPatch::OP_DELETED);
}

// Test array patch
TEST(ArrayPatch) {
    JsonDiffPatch::JsonDiffPatch jdp;
    
    json left = json::array({1, 2, 3});
    json right = json::array({1, 2, 4});
    
    json diff = jdp.Diff(left, right);
    json patched = jdp.Patch(left, diff);
    
    ASSERT_EQ(patched, right);
}

// Test array unpatch
TEST(ArrayUnpatch) {
    JsonDiffPatch::JsonDiffPatch jdp;
    
    json left = json::array({1, 2, 3});
    json right = json::array({1, 2, 4});
    
    json diff = jdp.Diff(left, right);
    json unpatched = jdp.Unpatch(right, diff);
    
    ASSERT_EQ(unpatched, left);
}

// Test string diff
TEST(StringDiff) {
    JsonDiffPatch::JsonDiffPatch jdp;
    
    json left = "Hello World";
    json right = "Hello Universe";
    
    json diff = jdp.Diff(left, right);
    
    ASSERT_FALSE(diff.is_null());
    ASSERT_TRUE(diff.is_array());
    ASSERT_EQ(diff.size(), 2);
    ASSERT_EQ(diff[0], "Hello World");
    ASSERT_EQ(diff[1], "Hello Universe");
}

// Test complex nested object
TEST(NestedObjectDiff) {
    JsonDiffPatch::JsonDiffPatch jdp;
    
    json left = {
        {"user", {
            {"name", "John"},
            {"age", 30},
            {"address", {
                {"street", "123 Main St"},
                {"city", "New York"}
            }}
        }}
    };
    
    json right = {
        {"user", {
            {"name", "John"},
            {"age", 31},
            {"address", {
                {"street", "456 Oak Ave"},
                {"city", "New York"}
            }}
        }}
    };
    
    json diff = jdp.Diff(left, right);
    
    ASSERT_FALSE(diff.is_null());
    ASSERT_TRUE(diff.contains("user"));
    ASSERT_TRUE(diff["user"].contains("age"));
    ASSERT_TRUE(diff["user"].contains("address"));
    ASSERT_TRUE(diff["user"]["address"].contains("street"));
    
    json patched = jdp.Patch(left, diff);
    ASSERT_EQ(patched, right);
    
    json unpatched = jdp.Unpatch(right, diff);
    ASSERT_EQ(unpatched, left);
}

// Test mixed array with objects
TEST(ArrayWithObjects) {
    JsonDiffPatch::JsonDiffPatch jdp;
    
    json left = json::array({
        {{"id", 1}, {"name", "Alice"}},
        {{"id", 2}, {"name", "Bob"}}
    });
    
    json right = json::array({
        {{"id", 1}, {"name", "Alice"}},
        {{"id", 2}, {"name", "Robert"}}
    });
    
    json diff = jdp.Diff(left, right);
    json patched = jdp.Patch(left, diff);
    
    ASSERT_EQ(patched, right);
}

// Test C API
TEST(C_API_Diff) {
    const char* left_str = "{\"x\":1,\"y\":2}";
    const char* right_str = "{\"x\":1,\"y\":3}";
    
    const char* diff_result = JDP_Diff(left_str, right_str);
    
    ASSERT_NE(diff_result, nullptr);
    
    std::string diff_string(diff_result);
    ASSERT_FALSE(diff_string.empty());
    
    // Parse the result to verify it's valid JSON
    json diff_json = json::parse(diff_string);
    ASSERT_TRUE(diff_json.contains("y"));
    
    JDP_FreeString(diff_result);
}

// Test C API Patch
TEST(C_API_Patch) {
    const char* left_str = "{\"x\":1,\"y\":2}";
    const char* right_str = "{\"x\":1,\"y\":3}";
    
    const char* diff_result = JDP_Diff(left_str, right_str);
    const char* patch_result = JDP_Patch(left_str, diff_result);
    
    ASSERT_NE(patch_result, nullptr);
    
    json patched = json::parse(patch_result);
    json expected = json::parse(right_str);
    
    ASSERT_EQ(patched, expected);
    
    JDP_FreeString(diff_result);
    JDP_FreeString(patch_result);
}

// Test C API Unpatch
TEST(C_API_Unpatch) {
    const char* left_str = "{\"x\":1,\"y\":2}";
    const char* right_str = "{\"x\":1,\"y\":3}";
    
    const char* diff_result = JDP_Diff(left_str, right_str);
    const char* unpatch_result = JDP_Unpatch(right_str, diff_result);
    
    ASSERT_NE(unpatch_result, nullptr);
    
    json unpatched = json::parse(unpatch_result);
    json expected = json::parse(left_str);
    
    ASSERT_EQ(unpatched, expected);
    
    JDP_FreeString(diff_result);
    JDP_FreeString(unpatch_result);
}

// Test empty objects
TEST(EmptyObjects) {
    JsonDiffPatch::JsonDiffPatch jdp;
    
    json left = json::object();
    json right = {{"x", 1}};
    
    json diff = jdp.Diff(left, right);
    json patched = jdp.Patch(left, diff);
    
    ASSERT_EQ(patched, right);
}

// Test null values
TEST(NullValues) {
    JsonDiffPatch::JsonDiffPatch jdp;
    
    json left = {{"x", nullptr}};
    json right = {{"x", 1}};
    
    json diff = jdp.Diff(left, right);
    json patched = jdp.Patch(left, diff);
    
    ASSERT_EQ(patched, right);
}

// Test boolean values
TEST(BooleanValues) {
    JsonDiffPatch::JsonDiffPatch jdp;
    
    json left = {{"flag", true}};
    json right = {{"flag", false}};
    
    json diff = jdp.Diff(left, right);
    json patched = jdp.Patch(left, diff);
    
    ASSERT_EQ(patched, right);
}

// Test number types
TEST(NumberTypes) {
    JsonDiffPatch::JsonDiffPatch jdp;
    
    json left = {{"int", 42}, {"float", 3.14}};
    json right = {{"int", 43}, {"float", 2.71}};
    
    json diff = jdp.Diff(left, right);
    json patched = jdp.Patch(left, diff);
    
    ASSERT_EQ(patched, right);
}

int main() {
    globalTestRunner.runAll();
    return globalTestRunner.getFailedCount();
}