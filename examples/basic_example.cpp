#include <iostream>
#include "../include/JsonDiffPatch/JsonDiffPatch.h"

int main() {
    // Example 1: Using C++ API
    std::cout << "=== C++ API Example ===" << std::endl;
    
    JsonDiffPatch::JsonDiffPatch jdp;

    nlohmann::json left = { {"x", 1}, {"y", 2} };
    nlohmann::json right = { {"x", 1}, {"y", 3}, {"z", 4} };

    nlohmann::json diff = jdp.Diff(left, right);
    std::cout << "Original: " << left.dump() << std::endl;
    std::cout << "Modified: " << right.dump() << std::endl;
    std::cout << "Diff: " << diff.dump() << std::endl;

    nlohmann::json patched = jdp.Patch(left, diff);
    std::cout << "Patched: " << patched.dump() << std::endl;

    nlohmann::json unpatched = jdp.Unpatch(right, diff);
    std::cout << "Unpatched: " << unpatched.dump() << std::endl;

    std::cout << std::endl;

    // Example 2: Using C API
    std::cout << "=== C API Example ===" << std::endl;
    
    const char* a = "{\"name\":\"John\",\"age\":30}";
    const char* b = "{\"name\":\"John\",\"age\":31,\"city\":\"New York\"}";

    const char* diff_result = JDP_Diff(a, b);
    std::cout << "JSON A: " << a << std::endl;
    std::cout << "JSON B: " << b << std::endl;
    std::cout << "Diff: " << diff_result << std::endl;

    const char* patched_result = JDP_Patch(a, diff_result);
    std::cout << "Patched: " << patched_result << std::endl;

    const char* unpatched_result = JDP_Unpatch(b, diff_result);
    std::cout << "Unpatched: " << unpatched_result << std::endl;

    // Free memory (though in this implementation it's a no-op)
    JDP_FreeString(diff_result);
    JDP_FreeString(patched_result);
    JDP_FreeString(unpatched_result);

    return 0;
}