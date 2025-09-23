#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include "../../thirdparty/nlohmann/json.hpp"

using json = nlohmann::json;

namespace JsonDiffPatch {

    // Use constants instead of enums to avoid compiler issues
    const int OP_DELETED = 0;
    const int OP_TEXTDIFF = 2;
    const int OP_ARRAYMOVE = 3;

    const int MODE_SIMPLE = 0;
    const int MODE_EFFICIENT = 1;

    const int TEXTDIFF_SIMPLE = 0;
    const int TEXTDIFF_EFFICIENT = 1;

    const int DIFF_DELETE = 0;
    const int DIFF_INSERT = 1;
    const int DIFF_EQUAL = 2;

    struct TextDiff {
        int operation;
        std::string text;
        
        TextDiff(int op, const std::string& txt) : operation(op), text(txt) {}
    };

    struct TextPatch {
        std::vector<TextDiff> diffs;
        int start1 = 0, start2 = 0;
        int length1 = 0, length2 = 0;
        
        std::string ToString() const;
    };

    // Options class to configure diff behavior
    struct ArrayOptions {
        bool DetectMove = false;
        bool IncludeValueOnMove = false;
    };

    struct Options {
        int ArrayDiff = MODE_EFFICIENT;
        int TextDiff = TEXTDIFF_EFFICIENT;
        size_t MinEfficientTextDiffLength = 50;
        ArrayOptions DiffArrayOptions;
        std::function<std::string(const json&)> ObjectHash = nullptr;
    };

    // LCS (Longest Common Subsequence) implementation
    struct LcsResult {
        std::vector<json> Sequence;
        std::vector<int> Indices1;
        std::vector<int> Indices2;
    };

    class ItemMatch {
    public:
        std::function<std::string(const json&)> ObjectHash;
        
        ItemMatch(std::function<std::string(const json&)> objectHash = nullptr)
            : ObjectHash(objectHash) {}
        
        bool Match(const json& obj1, const json& obj2) const;
        bool MatchArrayElement(const json& obj1, int index1, const json& obj2, int index2) const;
    };

    // Simple text diff engine (basic version of DiffMatchPatch)
    class SimpleTextDiff {
    public:
        static std::vector<TextDiff> ComputeDiff(const std::string& text1, const std::string& text2);
        static std::string Encode(const std::string& str);
        static std::string Decode(const std::string& str);

        static std::vector<TextPatch> CreatePatches(const std::string& text1, const std::string& text2);
        static std::string PatchesToText(const std::vector<TextPatch>& patches);
        static std::vector<TextPatch> PatchesFromText(const std::string& patchText);
        static std::pair<std::string, std::vector<bool>> ApplyPatches(const std::vector<TextPatch>& patches, const std::string& text);
    };

    // Main JsonDiffPatch class
    class JsonDiffPatch {
    private:
        Options _options;
        
        json ObjectDiff(const json& left, const json& right);
        json ArrayDiff(const json& left, const json& right);
        json ObjectPatch(const json& obj, const json& patch);
        json ArrayPatch(const json& left, const json& patch);
        json ObjectUnpatch(const json& obj, const json& patch);
        json ArrayUnpatch(const json& right, const json& patch);
        
        LcsResult ComputeLcs(const std::vector<json>& left, const std::vector<json>& right, const ItemMatch& match);
        
    public:
        JsonDiffPatch() = default;
        JsonDiffPatch(const Options& options) : _options(options) {}
        
        json Diff(const json& left, const json& right);
        json Patch(const json& left, const json& patch);
        json Unpatch(const json& right, const json& patch);
        
        std::string Diff(const std::string& left, const std::string& right);
        std::string Patch(const std::string& left, const std::string& patch);
        std::string Unpatch(const std::string& right, const std::string& patch);
    };

} // namespace JsonDiffPatch

// C API for GameMaker Studio 2
extern "C" {
    __declspec(dllexport) const char* JDP_Diff(const char* json_left, const char* json_right);
    __declspec(dllexport) const char* JDP_Patch(const char* json_left, const char* patch_json);
    __declspec(dllexport) const char* JDP_Unpatch(const char* json_right, const char* patch_json);
    __declspec(dllexport) void JDP_FreeString(const char* s);
}
