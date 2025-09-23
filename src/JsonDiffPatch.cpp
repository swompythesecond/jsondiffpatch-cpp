#include "../include/JsonDiffPatch/JsonDiffPatch.h"
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cctype>

namespace JsonDiffPatch {

// ItemMatch implementation
bool ItemMatch::Match(const json& obj1, const json& obj2) const {
    if (ObjectHash && obj1.is_object()) {
        std::string hash1 = ObjectHash(obj1);
        std::string hash2 = ObjectHash(obj2);
        return !hash1.empty() && !hash2.empty() && hash1 == hash2;
    }
    return obj1 == obj2;
}

bool ItemMatch::MatchArrayElement(const json& obj1, int index1, const json& obj2, int index2) const {
    if (ObjectHash) {
        return Match(obj1, obj2);
    }
    
    if (!obj1.is_object() && !obj1.is_array()) {
        return obj1 == obj2;
    }
    
    return index1 == index2;
}

// SimpleTextDiff implementation
std::vector<TextDiff> SimpleTextDiff::ComputeDiff(const std::string& text1, const std::string& text2) {
    std::vector<TextDiff> diffs;
    
    if (text1 == text2) {
        if (!text1.empty()) {
            diffs.emplace_back(DIFF_EQUAL, text1);
        }
        return diffs;
    }
    
    // Simple implementation - find common prefix and suffix
    size_t commonStart = 0;
    size_t minLen = (std::min)(text1.length(), text2.length());
    
    while (commonStart < minLen && text1[commonStart] == text2[commonStart]) {
        commonStart++;
    }
    
    size_t commonEnd = 0;
    while (commonEnd < minLen - commonStart && 
           text1[text1.length() - 1 - commonEnd] == text2[text2.length() - 1 - commonEnd]) {
        commonEnd++;
    }
    
    if (commonStart > 0) {
        diffs.emplace_back(DIFF_EQUAL, text1.substr(0, commonStart));
    }
    
    std::string middle1 = text1.substr(commonStart, text1.length() - commonStart - commonEnd);
    std::string middle2 = text2.substr(commonStart, text2.length() - commonStart - commonEnd);
    
    if (!middle1.empty()) {
        diffs.emplace_back(DIFF_DELETE, middle1);
    }
    if (!middle2.empty()) {
        diffs.emplace_back(DIFF_INSERT, middle2);
    }
    
    if (commonEnd > 0) {
        diffs.emplace_back(DIFF_EQUAL, text1.substr(text1.length() - commonEnd));
    }
    
    return diffs;
}

std::string SimpleTextDiff::Encode(const std::string& str) {
    std::ostringstream encoded;
    for (char c : str) {
        if (c == '%') {
            encoded << "%25";
        } else if (c == '\n') {
            encoded << "%0A";
        } else if (c == '\r') {
            encoded << "%0D";
        } else {
            encoded << c;
        }
    }
    return encoded.str();
}

std::string SimpleTextDiff::Decode(const std::string& str) {
    std::string decoded;
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '%' && i + 2 < str.length()) {
            std::string hex = str.substr(i + 1, 2);
            char ch = static_cast<char>(std::stoi(hex, nullptr, 16));
            decoded += ch;
            i += 2;
        } else {
            decoded += str[i];
        }
    }
    return decoded;
}

std::vector<TextPatch> SimpleTextDiff::CreatePatches(const std::string& text1, const std::string& text2) {
    std::vector<TextPatch> patches;
    auto diffs = ComputeDiff(text1, text2);
    
    if (diffs.empty()) return patches;
    
    TextPatch patch;
    patch.diffs = diffs;
    patch.start1 = 0;
    patch.start2 = 0;
    patch.length1 = static_cast<int>(text1.length());
    patch.length2 = static_cast<int>(text2.length());
    
    patches.push_back(patch);
    return patches;
}

// TextPatch ToString implementation
std::string TextPatch::ToString() const {
    std::ostringstream result;
    result << "@@ -" << (start1 + 1) << "," << length1 << " +" << (start2 + 1) << "," << length2 << " @@\n";
    
    for (const auto& diff : diffs) {
        char op;
        switch (diff.operation) {
            case DIFF_INSERT: op = '+'; break;
            case DIFF_DELETE: op = '-'; break;
            case DIFF_EQUAL: op = ' '; break;
            default: op = ' '; break;
        }
        result << op << SimpleTextDiff::Encode(diff.text) << "\n";
    }
    
    return result.str();
}

std::string SimpleTextDiff::PatchesToText(const std::vector<TextPatch>& patches) {
    std::ostringstream result;
    for (const auto& patch : patches) {
        result << patch.ToString();
    }
    return result.str();
}

std::vector<TextPatch> SimpleTextDiff::PatchesFromText(const std::string& patchText) {
    std::vector<TextPatch> patches;
    std::istringstream stream(patchText);
    std::string line;
    
    TextPatch currentPatch;
    bool inPatch = false;
    
    while (std::getline(stream, line)) {
        if (line.substr(0, 2) == "@@") {
            if (inPatch) {
                patches.push_back(currentPatch);
            }
            currentPatch = TextPatch();
            inPatch = true;
            // Parse header - simplified
        } else if (inPatch && !line.empty()) {
            char op = line[0];
            std::string text = line.length() > 1 ? Decode(line.substr(1)) : "";
            
            int type;
            switch (op) {
                case '+': type = DIFF_INSERT; break;
                case '-': type = DIFF_DELETE; break;
                case ' ': type = DIFF_EQUAL; break;
                default: continue;
            }
            
            currentPatch.diffs.emplace_back(type, text);
        }
    }
    
    if (inPatch) {
        patches.push_back(currentPatch);
    }
    
    return patches;
}

std::pair<std::string, std::vector<bool>> SimpleTextDiff::ApplyPatches(const std::vector<TextPatch>& patches, const std::string& text) {
    if (patches.empty()) {
        return std::make_pair(text, std::vector<bool>());
    }
    
    std::string result = text;
    std::vector<bool> results;
    
    for (const auto& patch : patches) {
        // Simple application - reconstruct the target text
        std::string newText;
        for (const auto& diff : patch.diffs) {
            if (diff.operation == DIFF_INSERT || diff.operation == DIFF_EQUAL) {
                newText += diff.text;
            }
        }
        result = newText;
        results.push_back(true);
    }
    
    return std::make_pair(result, results);
}

// LCS implementation
LcsResult JsonDiffPatch::ComputeLcs(const std::vector<json>& left, const std::vector<json>& right, const ItemMatch& match) {
    size_t m = left.size();
    size_t n = right.size();
    
    // Create LCS matrix
    std::vector<std::vector<int>> matrix(m + 1, std::vector<int>(n + 1, 0));
    
    for (size_t i = 1; i <= m; ++i) {
        for (size_t j = 1; j <= n; ++j) {
            if (match.MatchArrayElement(left[i-1], static_cast<int>(i-1), right[j-1], static_cast<int>(j-1))) {
                matrix[i][j] = matrix[i-1][j-1] + 1;
            } else {
                matrix[i][j] = (std::max)(matrix[i-1][j], matrix[i][j-1]);
            }
        }
    }
    
    // Backtrack to find the LCS
    LcsResult result;
    int i = static_cast<int>(m), j = static_cast<int>(n);
    
    while (i > 0 && j > 0) {
        if (match.Match(left[i-1], right[j-1])) {
            result.Sequence.insert(result.Sequence.begin(), left[i-1]);
            result.Indices1.insert(result.Indices1.begin(), i-1);
            result.Indices2.insert(result.Indices2.begin(), j-1);
            --i;
            --j;
        } else if (matrix[i][j-1] > matrix[i-1][j]) {
            --j;
        } else {
            --i;
        }
    }
    
    return result;
}

// JsonDiffPatch main implementation
json JsonDiffPatch::Diff(const json& left, const json& right) {
    ItemMatch itemMatch(_options.ObjectHash);
    
    json leftValue = left.is_null() ? json("") : left;
    json rightValue = right.is_null() ? json("") : right;
    
    if (leftValue.is_object() && rightValue.is_object()) {
        return ObjectDiff(leftValue, rightValue);
    }
    
    if (_options.ArrayDiff == MODE_EFFICIENT && 
        leftValue.is_array() && rightValue.is_array()) {
        return ArrayDiff(leftValue, rightValue);
    }
    
    if (_options.TextDiff == TEXTDIFF_EFFICIENT &&
        leftValue.is_string() && rightValue.is_string()) {
        std::string leftStr = leftValue.get<std::string>();
        std::string rightStr = rightValue.get<std::string>();
        
        if (leftStr.length() > _options.MinEfficientTextDiffLength || 
            rightStr.length() > _options.MinEfficientTextDiffLength) {
            auto patches = SimpleTextDiff::CreatePatches(leftStr, rightStr);
            if (!patches.empty()) {
                json result = json::array();
                result.push_back(SimpleTextDiff::PatchesToText(patches));
                result.push_back(0);
                result.push_back(OP_TEXTDIFF);
                return result;
            }
        }
    }
    
    if (!itemMatch.Match(leftValue, rightValue)) {
        json result = json::array();
        result.push_back(leftValue);
        result.push_back(rightValue);
        return result;
    }
    
    return json(nullptr);
}

json JsonDiffPatch::ObjectDiff(const json& left, const json& right) {
    json diffPatch = json::object();
    
    // Find properties modified or deleted
    for (auto it = left.begin(); it != left.end(); ++it) {
        const std::string& key = it.key();
        const json& leftValue = it.value();
        
        if (right.contains(key)) {
            json d = Diff(leftValue, right[key]);
            if (!d.is_null()) {
                diffPatch[key] = d;
            }
        } else {
            // Property deleted
            json deleteArray = json::array();
            deleteArray.push_back(leftValue);
            deleteArray.push_back(0);
            deleteArray.push_back(OP_DELETED);
            diffPatch[key] = deleteArray;
        }
    }
    
    // Find properties that were added
    for (auto it = right.begin(); it != right.end(); ++it) {
        const std::string& key = it.key();
        const json& rightValue = it.value();
        
        if (!left.contains(key)) {
            json addArray = json::array();
            addArray.push_back(rightValue);
            diffPatch[key] = addArray;
        }
    }
    
    return diffPatch.empty() ? json(nullptr) : diffPatch;
}

json JsonDiffPatch::ArrayDiff(const json& left, const json& right) {
    ItemMatch itemMatch(_options.ObjectHash);
    json result = json::object();
    result["_t"] = "a";
    
    if (left == right) {
        return json(nullptr);
    }
    
    std::vector<json> leftVec = left.get<std::vector<json>>();
    std::vector<json> rightVec = right.get<std::vector<json>>();
    
    size_t commonHead = 0;
    size_t commonTail = 0;
    
    // Find common head
    while (commonHead < leftVec.size() && commonHead < rightVec.size() &&
           itemMatch.MatchArrayElement(leftVec[commonHead], static_cast<int>(commonHead), 
                                     rightVec[commonHead], static_cast<int>(commonHead))) {
        json child = Diff(leftVec[commonHead], rightVec[commonHead]);
        if (!child.is_null()) {
            result[std::to_string(commonHead)] = child;
        }
        commonHead++;
    }
    
    // Find common tail
    while (commonTail + commonHead < leftVec.size() && 
           commonTail + commonHead < rightVec.size() &&
           itemMatch.MatchArrayElement(leftVec[leftVec.size() - 1 - commonTail], 
                                     static_cast<int>(leftVec.size() - 1 - commonTail),
                                     rightVec[rightVec.size() - 1 - commonTail], 
                                     static_cast<int>(rightVec.size() - 1 - commonTail))) {
        size_t index1 = leftVec.size() - 1 - commonTail;
        size_t index2 = rightVec.size() - 1 - commonTail;
        json child = Diff(leftVec[index1], rightVec[index2]);
        if (!child.is_null()) {
            result[std::to_string(index2)] = child;
        }
        commonTail++;
    }
    
    // Handle simple cases
    if (commonHead + commonTail == leftVec.size()) {
        // Block was added
        for (size_t index = commonHead; index < rightVec.size() - commonTail; ++index) {
            json addArray = json::array();
            addArray.push_back(rightVec[index]);
            result[std::to_string(index)] = addArray;
        }
        return result;
    }
    
    if (commonHead + commonTail == rightVec.size()) {
        // Block was removed
        for (size_t index = commonHead; index < leftVec.size() - commonTail; ++index) {
            json deleteArray = json::array();
            deleteArray.push_back(leftVec[index]);
            deleteArray.push_back(0);
            deleteArray.push_back(OP_DELETED);
            result["_" + std::to_string(index)] = deleteArray;
        }
        return result;
    }
    
    // Complex diff using LCS
    std::vector<json> trimmedLeft(leftVec.begin() + commonHead, leftVec.end() - commonTail);
    std::vector<json> trimmedRight(rightVec.begin() + commonHead, rightVec.end() - commonTail);
    
    LcsResult lcs = ComputeLcs(trimmedLeft, trimmedRight, itemMatch);
    
    // Mark deletions
    for (size_t index = commonHead; index < leftVec.size() - commonTail; ++index) {
        bool found = false;
        for (int lcsIndex : lcs.Indices1) {
            if (lcsIndex == static_cast<int>(index - commonHead)) {
                found = true;
                break;
            }
        }
        
        if (!found) {
            json deleteArray = json::array();
            deleteArray.push_back(leftVec[index]);
            deleteArray.push_back(0);
            deleteArray.push_back(OP_DELETED);
            result["_" + std::to_string(index)] = deleteArray;
        }
    }
    
    // Mark additions and modifications
    for (size_t index = commonHead; index < rightVec.size() - commonTail; ++index) {
        auto it = std::find(lcs.Indices2.begin(), lcs.Indices2.end(), 
                           static_cast<int>(index - commonHead));
        
        if (it == lcs.Indices2.end()) {
            // Added
            json addArray = json::array();
            addArray.push_back(rightVec[index]);
            result[std::to_string(index)] = addArray;
        } else {
            // Potentially modified
            size_t lcsIdx = std::distance(lcs.Indices2.begin(), it);
            size_t leftIndex = lcs.Indices1[lcsIdx] + commonHead;
            
            json diff = Diff(leftVec[leftIndex], rightVec[index]);
            if (!diff.is_null()) {
                result[std::to_string(index)] = diff;
            }
        }
    }
    
    // Check if result is empty (only contains "_t")
    if (result.size() == 1 && result.contains("_t")) {
        return json(nullptr);
    }
    
    return result;
}

json JsonDiffPatch::Patch(const json& left, const json& patch) {
    if (patch.is_null()) {
        return left;
    }
    
    if (patch.is_object()) {
        if (left.is_array() && patch.contains("_t") && 
            patch["_t"].is_string() && patch["_t"].get<std::string>() == "a") {
            return ArrayPatch(left, patch);
        }
        return ObjectPatch(left, patch);
    }
    
    if (patch.is_array()) {
        json patchArray = patch;
        
        if (patchArray.size() == 1) {
            // Add
            return patchArray[0];
        }
        
        if (patchArray.size() == 2) {
            // Replace
            return patchArray[1];
        }
        
        if (patchArray.size() == 3) {
            // Delete, Move or TextDiff
            if (!patchArray[2].is_number_integer()) {
                throw std::runtime_error("Invalid patch object");
            }
            
            int op = patchArray[2].get<int>();
            
            if (op == 0) {
                return json(nullptr);
            }
            
            if (op == OP_TEXTDIFF) {
                if (!left.is_string()) {
                    throw std::runtime_error("Invalid patch object");
                }
                
                std::string patchText = patchArray[0].get<std::string>();
                auto patches = SimpleTextDiff::PatchesFromText(patchText);
                
                if (patches.empty()) {
                    throw std::runtime_error("Invalid textline");
                }
                
                auto result = SimpleTextDiff::ApplyPatches(patches, left.get<std::string>());
                
                for (size_t i = 0; i < result.second.size(); ++i) {
                    bool success = result.second[i];
                    if (!success) {
                        throw std::runtime_error("Text patch failed");
                    }
                }
                
                return json(result.first);
            }
            
            throw std::runtime_error("Invalid patch object");
        }
        
        throw std::runtime_error("Invalid patch object");
    }
    
    return json(nullptr);
}

json JsonDiffPatch::ObjectPatch(const json& obj, const json& patch) {
    json target = obj.is_null() ? json::object() : obj;
    
    if (patch.is_null()) {
        return target;
    }
    
    for (auto it = patch.begin(); it != patch.end(); ++it) {
        const std::string& key = it.key();
        const json& patchValue = it.value();
        
        // Check for deletion
        if (patchValue.is_array() && patchValue.size() == 3 && 
            patchValue[2].is_number_integer() && patchValue[2].get<int>() == 0) {
            target.erase(key);
        } else {
            if (target.contains(key)) {
                target[key] = Patch(target[key], patchValue);
            } else {
                target[key] = Patch(json(nullptr), patchValue);
            }
        }
    }
    
    return target;
}

json JsonDiffPatch::ArrayPatch(const json& left, const json& patch) {
    // Expect: patch has "_t":"a"
    std::vector<json> arr = left.get<std::vector<json>>();

    struct Removal { size_t index; bool isMove; size_t moveTarget; };
    struct Modification { size_t index; json value; };
    struct Insertion { size_t index; json value; bool isMove; };

    std::vector<Removal> removals;
    std::vector<Modification> modifications;
    std::vector<Insertion> insertions;

    // Classify ops
    for (auto it = patch.begin(); it != patch.end(); ++it) {
        const std::string& key = it.key();
        if (key == "_t") continue;

        const json& v = it.value();
        if (!key.empty() && key[0] == '_') {
            // deletion or move-out
            size_t idx = std::stoul(key.substr(1));
            if (v.is_array() && v.size() == 3) {
                int op = v[2].get<int>();
                if (op == OP_DELETED) {
                    removals.push_back({ idx, false, 0 });
                }
                else if (op == OP_ARRAYMOVE) {
                    // jsondiffpatch encodes move as ["<val>", toIndex, 3]
                    size_t to = v[1].get<size_t>();
                    removals.push_back({ idx, true, to });
                }
            }
        }
        else {
            // addition or modification
            size_t idx = std::stoul(key);
            if (v.is_array() && v.size() == 1) {
                insertions.push_back({ idx, v[0], false });
            }
            else if (v.is_array() && v.size() == 3 && v[2].is_number_integer()
                && v[2].get<int>() == OP_ARRAYMOVE) {
                // (rare form) move encoded on positive key
                size_t to = v[1].get<size_t>();
                // treat as: remove from '_' + fromIndex and insert at to
                // if you ever generate this form, you’d need the "from"; most diffs use the '_' key form.
                insertions.push_back({ to, v[0], true });
            }
            else {
                modifications.push_back({ idx, v });
            }
        }
    }

    // 1) Apply removals (including move extraction) in DESC order
    std::sort(removals.begin(), removals.end(),
        [](const Removal& a, const Removal& b) { return a.index > b.index; });

    // Keep a temporary store for values we move so we can reinsert later using fresh indices
    struct PendingMove { size_t target; json value; };
    std::vector<PendingMove> pendingMoves;

    for (const auto& r : removals) {
        if (arr.empty()) continue;
        size_t idx = (r.index < arr.size()) ? r.index : (arr.size() - 1);
        json taken = arr[idx];
        arr.erase(arr.begin() + idx);
        if (r.isMove) {
            pendingMoves.push_back({ r.moveTarget, taken });
        }
    }

    // 2) Apply modifications in ASC order (only if index still exists)
    std::sort(modifications.begin(), modifications.end(),
        [](const Modification& a, const Modification& b) { return a.index < b.index; });

    for (const auto& m : modifications) {
        if (m.index < arr.size()) {
            arr[m.index] = Patch(arr[m.index], m.value);
        }
    }

    // 3) Apply move insertions first (ASC), then regular insertions (ASC)
    std::sort(pendingMoves.begin(), pendingMoves.end(),
        [](const PendingMove& a, const PendingMove& b) { return a.target < b.target; });

    for (const auto& mv : pendingMoves) {
        size_t pos = (mv.target <= arr.size()) ? mv.target : arr.size();
        arr.insert(arr.begin() + pos, mv.value);
    }

    std::sort(insertions.begin(), insertions.end(),
        [](const Insertion& a, const Insertion& b) { return a.index < b.index; });

    for (const auto& ins : insertions) {
        size_t pos = (ins.index <= arr.size()) ? ins.index : arr.size();
        arr.insert(arr.begin() + pos, ins.value);
    }

    return json(arr);
}

json JsonDiffPatch::Unpatch(const json& right, const json& patch) {
    if (patch.is_null()) {
        return right;
    }
    
    if (patch.is_object()) {
        if (right.is_array() && patch.contains("_t") && 
            patch["_t"].is_string() && patch["_t"].get<std::string>() == "a") {
            return ArrayUnpatch(right, patch);
        }
        return ObjectUnpatch(right, patch);
    }
    
    if (patch.is_array()) {
        json patchArray = patch;
        
        if (patchArray.size() == 1) {
            // Add (we need to remove)
            return json(nullptr);
        }
        
        if (patchArray.size() == 2) {
            // Replace
            return patchArray[0];
        }
        
        if (patchArray.size() == 3) {
            if (!patchArray[2].is_number_integer()) {
                throw std::runtime_error("Invalid patch object");
            }
            
            int op = patchArray[2].get<int>();
            
            if (op == 0) {
                return patchArray[0];
            }
            
            if (op == OP_TEXTDIFF) {
                if (!right.is_string()) {
                    throw std::runtime_error("Invalid patch object");
                }
                
                // For unpatch, we need to reverse the text diff
                std::string patchText = patchArray[0].get<std::string>();
                auto patches = SimpleTextDiff::PatchesFromText(patchText);
                
                // Create reverse patches
                std::vector<TextPatch> reversePatches;
                for (size_t i = 0; i < patches.size(); ++i) {
                    TextPatch& patchItem = patches[i];
                    TextPatch reversePatch = patchItem;
                    reversePatch.diffs.clear();
                    
                    for (const auto& diff : patchItem.diffs) {
                        if (diff.operation == DIFF_DELETE) {
                            reversePatch.diffs.emplace_back(DIFF_INSERT, diff.text);
                        } else if (diff.operation == DIFF_INSERT) {
                            reversePatch.diffs.emplace_back(DIFF_DELETE, diff.text);
                        } else {
                            reversePatch.diffs.push_back(diff);
                        }
                    }
                    reversePatches.push_back(reversePatch);
                }
                
                auto result = SimpleTextDiff::ApplyPatches(reversePatches, right.get<std::string>());
                
                for (size_t i = 0; i < result.second.size(); ++i) {
                    bool success = result.second[i];
                    if (!success) {
                        throw std::runtime_error("Text patch failed");
                    }
                }
                
                return json(result.first);
            }
            
            throw std::runtime_error("Invalid patch object");
        }
        
        throw std::runtime_error("Invalid patch object");
    }
    
    return json(nullptr);
}

json JsonDiffPatch::ObjectUnpatch(const json& obj, const json& patch) {
    json target = obj.is_null() ? json::object() : obj;
    
    if (patch.is_null()) {
        return target;
    }
    
    for (auto it = patch.begin(); it != patch.end(); ++it) {
        const std::string& key = it.key();
        const json& patchValue = it.value();
        
        // Check for addition (which we need to undo by removing)
        if (patchValue.is_array() && patchValue.size() == 1) {
            target.erase(key);
        } else {
            if (target.contains(key)) {
                target[key] = Unpatch(target[key], patchValue);
            } else {
                target[key] = Unpatch(json(nullptr), patchValue);
            }
        }
    }
    
    return target;
}

json JsonDiffPatch::ArrayUnpatch(const json& right, const json& patch) {
    std::vector<json> arr = right.get<std::vector<json>>();

    struct AddWas { size_t index; };                  // positive key, size==1 → remove
    struct DelWas { size_t index; json value; };      // "_i": [value,0,0] → insert back
    struct ModWas { size_t index; json value; };      // positive key with object → unpatch
    struct MoveBack { size_t from; size_t to; };      // moved to "to" from "from" → move back

    std::vector<AddWas> adds;         // will remove these
    std::vector<DelWas> dels;         // will reinsert these
    std::vector<ModWas> mods;         // will unpatch these
    std::vector<MoveBack> moves;      // will move back

    for (auto it = patch.begin(); it != patch.end(); ++it) {
        const std::string& key = it.key();
        if (key == "_t") continue;
        const json& v = it.value();

        if (!key.empty() && key[0] == '_') {
            size_t idx = std::stoul(key.substr(1));
            if (v.is_array() && v.size() == 3) {
                int op = v[2].get<int>();
                if (op == OP_DELETED) {
                    dels.push_back({ idx, v[0] });
                }
                else if (op == OP_ARRAYMOVE) {
                    // original: moved from idx to v[1]
                    size_t to = v[1].get<size_t>();
                    moves.push_back({ to, idx }); // move back from "to" to "idx"
                }
            }
        }
        else {
            size_t idx = std::stoul(key);
            if (v.is_array() && v.size() == 1) {
                adds.push_back({ idx });
            }
            else {
                mods.push_back({ idx, v });
            }
        }
    }

    // 1) Undo additions: remove at index (DESC to keep indices stable)
    std::sort(adds.begin(), adds.end(),
        [](const AddWas& a, const AddWas& b) { return a.index > b.index; });
    for (const auto& a : adds) {
        if (arr.empty()) continue;
        if (a.index < arr.size()) {
            arr.erase(arr.begin() + a.index);
        }
        else {
            // if out of range, remove last (best-effort)
            arr.pop_back();
        }
    }

    // 2) Undo moves: move from 'from' back to 'to' (ASC by target)
    std::sort(moves.begin(), moves.end(),
        [](const MoveBack& x, const MoveBack& y) { return x.to < y.to; });
    for (const auto& mv : moves) {
        if (arr.empty()) continue;
        size_t from = (mv.from < arr.size()) ? mv.from : (arr.size() - 1);
        json val = arr[from];
        arr.erase(arr.begin() + from);
        size_t to = (mv.to <= arr.size()) ? mv.to : arr.size();
        arr.insert(arr.begin() + to, val);
    }

    // 3) Undo modifications (ASC)
    std::sort(mods.begin(), mods.end(),
        [](const ModWas& a, const ModWas& b) { return a.index < b.index; });
    for (const auto& m : mods) {
        if (m.index < arr.size()) {
            arr[m.index] = Unpatch(arr[m.index], m.value);
        }
    }

    // 4) Reinsert deletions (ASC)
    std::sort(dels.begin(), dels.end(),
        [](const DelWas& a, const DelWas& b) { return a.index < b.index; });
    for (const auto& d : dels) {
        size_t pos = (d.index <= arr.size()) ? d.index : arr.size();
        arr.insert(arr.begin() + pos, d.value);
    }

    return json(arr);
}

std::string JsonDiffPatch::Diff(const std::string& left, const std::string& right) {
    try {
        json leftJson = left.empty() ? json("") : json::parse(left);
        json rightJson = right.empty() ? json("") : json::parse(right);
        json result = Diff(leftJson, rightJson);
        return result.is_null() ? "" : result.dump();
    } catch (const std::exception&) {
        return "";
    }
}

std::string JsonDiffPatch::Patch(const std::string& left, const std::string& patch) {
    try {
        json leftJson = left.empty() ? json("") : json::parse(left);
        json patchJson = patch.empty() ? json(nullptr) : json::parse(patch);
        json result = Patch(leftJson, patchJson);
        return result.is_null() ? "" : result.dump();
    } catch (const std::exception&) {
        return "";
    }
}

std::string JsonDiffPatch::Unpatch(const std::string& right, const std::string& patch) {
    try {
        json rightJson = right.empty() ? json("") : json::parse(right);
        json patchJson = patch.empty() ? json(nullptr) : json::parse(patch);
        json result = Unpatch(rightJson, patchJson);
        return result.is_null() ? "" : result.dump();
    } catch (const std::exception&) {
        return "";
    }
}

} // namespace JsonDiffPatch

// C API implementation
extern "C" {

    static JsonDiffPatch::JsonDiffPatch g_diffPatch;
    static thread_local std::string g_lastResult;

    const char* JDP_Diff(const char* json_left, const char* json_right)
    {
        try {
            g_lastResult = g_diffPatch.Diff(
                json_left ? std::string(json_left) : std::string(),
                json_right ? std::string(json_right) : std::string());
            // always return at least ""
            return g_lastResult.c_str();
        }
        catch (...) {
            g_lastResult.clear();
            return g_lastResult.c_str();
        }
    }

    const char* JDP_Patch(const char* json_left, const char* patch_json)
    {
        try {
            g_lastResult = g_diffPatch.Patch(
                json_left ? std::string(json_left) : std::string(),
                patch_json ? std::string(patch_json) : std::string());
            return g_lastResult.c_str();
        }
        catch (...) {
            g_lastResult.clear();
            return g_lastResult.c_str();
        }
    }

    const char* JDP_Unpatch(const char* json_right, const char* patch_json)
    {
        try {
            g_lastResult = g_diffPatch.Unpatch(
                json_right ? std::string(json_right) : std::string(),
                patch_json ? std::string(patch_json) : std::string());
            return g_lastResult.c_str();
        }
        catch (...) {
            g_lastResult.clear();
            return g_lastResult.c_str();
        }
    }

    void JDP_FreeString(const char*) {}
}