#include "test_framework.h"
#include "../include/JsonDiffPatch/JsonDiffPatch.h"

using json = nlohmann::json;

extern TestRunner globalTestRunner;

// Test text diff functionality
TEST(TextDiffBasic) {
    auto diffs = JsonDiffPatch::SimpleTextDiff::ComputeDiff("Hello World", "Hello Universe");
    
    ASSERT_FALSE(diffs.empty());
    ASSERT_EQ(diffs[0].operation, JsonDiffPatch::DIFF_EQUAL);
    ASSERT_EQ(diffs[0].text, "Hello ");
    ASSERT_EQ(diffs[1].operation, JsonDiffPatch::DIFF_DELETE);
    ASSERT_EQ(diffs[1].text, "World");
    ASSERT_EQ(diffs[2].operation, JsonDiffPatch::DIFF_INSERT);
    ASSERT_EQ(diffs[2].text, "Universe");
}

// Test text diff with identical strings
TEST(TextDiffIdentical) {
    auto diffs = JsonDiffPatch::SimpleTextDiff::ComputeDiff("Hello", "Hello");
    
    ASSERT_EQ(diffs.size(), 1);
    ASSERT_EQ(diffs[0].operation, JsonDiffPatch::DIFF_EQUAL);
    ASSERT_EQ(diffs[0].text, "Hello");
}

// Test text diff with empty strings
TEST(TextDiffEmpty) {
    auto diffs = JsonDiffPatch::SimpleTextDiff::ComputeDiff("", "");
    ASSERT_TRUE(diffs.empty());
    
    diffs = JsonDiffPatch::SimpleTextDiff::ComputeDiff("Hello", "");
    ASSERT_EQ(diffs.size(), 1);
    ASSERT_EQ(diffs[0].operation, JsonDiffPatch::DIFF_DELETE);
    ASSERT_EQ(diffs[0].text, "Hello");
    
    diffs = JsonDiffPatch::SimpleTextDiff::ComputeDiff("", "Hello");
    ASSERT_EQ(diffs.size(), 1);
    ASSERT_EQ(diffs[0].operation, JsonDiffPatch::DIFF_INSERT);
    ASSERT_EQ(diffs[0].text, "Hello");
}

// Test text encoding/decoding
TEST(TextEncoding) {
    std::string original = "Hello\nWorld\r\nWith%Percent";
    std::string encoded = JsonDiffPatch::SimpleTextDiff::Encode(original);
    std::string decoded = JsonDiffPatch::SimpleTextDiff::Decode(encoded);
    
    ASSERT_EQ(decoded, original);
    ASSERT_TRUE(encoded.find("%0A") != std::string::npos); // Contains encoded newline
    ASSERT_TRUE(encoded.find("%25") != std::string::npos); // Contains encoded percent
}

// Test text patches
TEST(TextPatches) {
    std::string text1 = "The quick brown fox";
    std::string text2 = "The quick red fox";
    
    auto patches = JsonDiffPatch::SimpleTextDiff::CreatePatches(text1, text2);
    ASSERT_FALSE(patches.empty());
    
    std::string patchText = JsonDiffPatch::SimpleTextDiff::PatchesToText(patches);
    ASSERT_FALSE(patchText.empty());
    
    auto parsedPatches = JsonDiffPatch::SimpleTextDiff::PatchesFromText(patchText);
    ASSERT_FALSE(parsedPatches.empty());
    
    auto result = JsonDiffPatch::SimpleTextDiff::ApplyPatches(parsedPatches, text1);
    ASSERT_EQ(result.first, text2);
    ASSERT_TRUE(result.second[0]); // Patch applied successfully
}

// Test large array operations
TEST(LargeArrayDiff) {
    JsonDiffPatch::JsonDiffPatch jdp;
    
    json left = json::array();
    json right = json::array();
    
    // Create large arrays
    for (int i = 0; i < 100; ++i) {
        left.push_back(i);
        right.push_back(i);
    }
    
    // Modify one element
    right[50] = 999;
    
    json diff = jdp.Diff(left, right);
    json patched = jdp.Patch(left, diff);
    
    ASSERT_EQ(patched, right);
}

// Test deeply nested objects
TEST(DeeplyNestedObject) {
    JsonDiffPatch::JsonDiffPatch jdp;
    
    json left = {
        {"level1", {
            {"level2", {
                {"level3", {
                    {"level4", {
                        {"value", 42}
                    }}
                }}
            }}
        }}
    };
    
    json right = left;
    right["level1"]["level2"]["level3"]["level4"]["value"] = 43;
    
    json diff = jdp.Diff(left, right);
    json patched = jdp.Patch(left, diff);
    
    ASSERT_EQ(patched, right);
}

// Test invalid JSON in C API
TEST(C_API_InvalidJSON) {
    const char* invalid_json = "{invalid json}";
    const char* valid_json = "{\"x\":1}";
    
    const char* result = JDP_Diff(invalid_json, valid_json);
    
    // Should handle gracefully and return empty string
    std::string result_str(result);
    ASSERT_TRUE(result_str.empty());
    
    JDP_FreeString(result);
}

// Test C API with null pointers
TEST(C_API_NullPointers) {
    const char* result1 = JDP_Diff(nullptr, nullptr);
    ASSERT_NE(result1, nullptr);
    
    const char* result2 = JDP_Patch(nullptr, nullptr);
    ASSERT_NE(result2, nullptr);
    
    const char* result3 = JDP_Unpatch(nullptr, nullptr);
    ASSERT_NE(result3, nullptr);
    
    JDP_FreeString(result1);
    JDP_FreeString(result2);
    JDP_FreeString(result3);
}

// Test LCS (Longest Common Subsequence) with complex arrays
TEST(ComplexArrayLCS) {
    JsonDiffPatch::JsonDiffPatch jdp;
    
    json left = json::array({
        {{"id", 1}, {"name", "Alice"}},
        {{"id", 2}, {"name", "Bob"}},
        {{"id", 3}, {"name", "Charlie"}}
    });
    
    json right = json::array({
        {{"id", 1}, {"name", "Alice"}},
        {{"id", 4}, {"name", "David"}},
        {{"id", 3}, {"name", "Charlie"}},
        {{"id", 5}, {"name", "Eve"}}
    });
    
    json diff = jdp.Diff(left, right);
    json patched = jdp.Patch(left, diff);
    
    ASSERT_EQ(patched, right);
}

// Test array reordering
TEST(ArrayReordering) {
    JsonDiffPatch::JsonDiffPatch jdp;
    
    json left = json::array({1, 2, 3, 4, 5});
    json right = json::array({5, 4, 3, 2, 1});
    
    json diff = jdp.Diff(left, right);
    json patched = jdp.Patch(left, diff);
    
    ASSERT_EQ(patched, right);
}

// Test object with array values
TEST(ObjectWithArrays) {
    JsonDiffPatch::JsonDiffPatch jdp;
    
    json left = {
        {"numbers", json::array({1, 2, 3})},
        {"strings", json::array({"a", "b", "c"})}
    };
    
    json right = {
        {"numbers", json::array({1, 2, 4})},
        {"strings", json::array({"a", "b", "d"})}
    };
    
    json diff = jdp.Diff(left, right);
    json patched = jdp.Patch(left, diff);
    
    ASSERT_EQ(patched, right);
}

// Test performance with large objects
TEST(LargeObjectPerformance) {
    JsonDiffPatch::JsonDiffPatch jdp;
    
    json left = json::object();
    json right = json::object();
    
    // Create large objects
    for (int i = 0; i < 1000; ++i) {
        std::string key = "key" + std::to_string(i);
        left[key] = i;
        right[key] = i;
    }
    
    // Modify one value
    right["key500"] = 999;
    
    json diff = jdp.Diff(left, right);
    json patched = jdp.Patch(left, diff);
    
    ASSERT_EQ(patched, right);
    ASSERT_TRUE(diff.contains("key500"));
    ASSERT_FALSE(diff.contains("key499")); // Should not contain unchanged keys
}

// Test special characters in strings
TEST(SpecialCharactersInStrings) {
    JsonDiffPatch::JsonDiffPatch jdp;
    
    json left = {{"text", "Hello \"World\" with 'quotes' and \n newlines"}};
    json right = {{"text", "Hello \"Universe\" with 'quotes' and \n newlines"}};
    
    json diff = jdp.Diff(left, right);
    json patched = jdp.Patch(left, diff);
    
    ASSERT_EQ(patched, right);
}

// Test options configuration
TEST(OptionsConfiguration) {
    JsonDiffPatch::Options opts;
    opts.ArrayDiff = JsonDiffPatch::MODE_SIMPLE;
    opts.TextDiff = JsonDiffPatch::TEXTDIFF_SIMPLE;
    
    JsonDiffPatch::JsonDiffPatch jdp(opts);
    
    json left = json::array({1, 2, 3});
    json right = json::array({1, 2, 4});
    
    json diff = jdp.Diff(left, right);
    json patched = jdp.Patch(left, diff);
    
    ASSERT_EQ(patched, right);
}