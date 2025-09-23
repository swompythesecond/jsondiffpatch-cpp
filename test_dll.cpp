#include <windows.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>

// Function pointer types
typedef const char* (*JDP_DiffFunc)(const char*, const char*);
typedef const char* (*JDP_PatchFunc)(const char*, const char*);
typedef const char* (*JDP_UnpatchFunc)(const char*, const char*);
typedef void (*JDP_FreeStringFunc)(const char*);

int main() {
    std::cout << "Testing JsonDiffPatch DLL...\n" << std::endl;

    // Load the DLL
    HMODULE hModule = LoadLibraryA("JsonDiffPatch.dll");
    if (!hModule) {
        std::cerr << "Failed to load JsonDiffPatch.dll" << std::endl;
        std::cerr << "Make sure the DLL is in the same directory as this executable." << std::endl;
        system("pause");
        return 1;
    }

    // Get function addresses
    JDP_DiffFunc JDP_Diff = (JDP_DiffFunc)GetProcAddress(hModule, "JDP_Diff");
    JDP_PatchFunc JDP_Patch = (JDP_PatchFunc)GetProcAddress(hModule, "JDP_Patch");
    JDP_UnpatchFunc JDP_Unpatch = (JDP_UnpatchFunc)GetProcAddress(hModule, "JDP_Unpatch");
    JDP_FreeStringFunc JDP_FreeString = (JDP_FreeStringFunc)GetProcAddress(hModule, "JDP_FreeString");

    if (!JDP_Diff || !JDP_Patch || !JDP_Unpatch || !JDP_FreeString) {
        std::cerr << "Failed to get function addresses from DLL" << std::endl;
        FreeLibrary(hModule);
        system("pause");
        return 1;
    }

    std::cout << "DLL loaded successfully!" << std::endl;

    // Test data
    const char* json1 = R"({"name": "John", "age": 30, "skills": ["C++", "JavaScript"]})";
    const char* json2 = R"({"name": "John", "age": 31, "skills": ["C++", "JavaScript", "Python"], "city": "New York"})";

    std::cout << "\nOriginal JSON: " << json1 << std::endl;
    std::cout << "Modified JSON: " << json2 << std::endl;

    // Test diff
    std::cout << "\n=== Testing Diff ===" << std::endl;
    const char* diff = JDP_Diff(json1, json2);
    if (diff && strlen(diff) > 0) {
        std::cout << "Diff result: " << diff << std::endl;
    } else {
        std::cout << "Diff failed or returned empty result" << std::endl;
        FreeLibrary(hModule);
        system("pause");
        return 1;
    }

    // Test patch
    std::cout << "\n=== Testing Patch ===" << std::endl;
    const char* patched = JDP_Patch(json1, diff);
    if (patched && strlen(patched) > 0) {
        std::cout << "Patch result: " << patched << std::endl;
    } else {
        std::cout << "Patch failed or returned empty result" << std::endl;
    }

    // Test unpatch
    std::cout << "\n=== Testing Unpatch ===" << std::endl;
    const char* unpatched = JDP_Unpatch(json2, diff);
    if (unpatched && strlen(unpatched) > 0) {
        std::cout << "Unpatch result: " << unpatched << std::endl;
    } else {
        std::cout << "Unpatch failed or returned empty result" << std::endl;
    }

    // Verify round-trip
    std::cout << "\n=== Verification ===" << std::endl;
    if (patched && strlen(patched) > 0) {
        std::string patchedStr(patched);
        std::string originalJson2(json2);
        
        // Remove whitespace for comparison (JSON can have different formatting)
        patchedStr.erase(std::remove_if(patchedStr.begin(), patchedStr.end(), ::isspace), patchedStr.end());
        originalJson2.erase(std::remove_if(originalJson2.begin(), originalJson2.end(), ::isspace), originalJson2.end());
        
        if (patchedStr.find("\"age\":31") != std::string::npos &&
            patchedStr.find("\"city\":\"NewYork\"") != std::string::npos) {
            std::cout << "? Patch test passed!" << std::endl;
        } else {
            std::cout << "? Patch test failed - result doesn't match expected" << std::endl;
        }
    }

    if (unpatched && strlen(unpatched) > 0) {
        std::string unpatchedStr(unpatched);
        std::string originalJson1(json1);
        
        // Remove whitespace for comparison
        unpatchedStr.erase(std::remove_if(unpatchedStr.begin(), unpatchedStr.end(), ::isspace), unpatchedStr.end());
        originalJson1.erase(std::remove_if(originalJson1.begin(), originalJson1.end(), ::isspace), originalJson1.end());
        
        if (unpatchedStr.find("\"age\":30") != std::string::npos &&
            unpatchedStr.find("\"city\"") == std::string::npos) {
            std::cout << "? Unpatch test passed!" << std::endl;
        } else {
            std::cout << "? Unpatch test failed - result doesn't match expected" << std::endl;
        }
    }

    std::cout << "\nAll tests completed!" << std::endl;

    // Cleanup
    FreeLibrary(hModule);
    
    std::cout << "\nPress any key to exit..." << std::endl;
    system("pause");
    return 0;
}