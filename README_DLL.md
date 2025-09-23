# JsonDiffPatch DLL für GameMaker

Diese Anleitung zeigt, wie Sie die JsonDiffPatch-Bibliothek als DLL kompilieren und in GameMaker verwenden.

## Voraussetzungen

- Visual Studio 2019 oder neuer (mit C++ Workload)
- CMake 3.10 oder neuer
- Git

## DLL erstellen

### Option 1: Automatisches Build-Script (Empfohlen)

1. Führen Sie `build_dll.bat` aus
2. Die DLL wird in `build/Release/JsonDiffPatch.dll` erstellt

### Option 2: Manuell mit CMake

```batch
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config Release --target JsonDiffPatchDLL
```

## DLL in GameMaker verwenden

### Schritt 1: DLL in GameMaker-Projekt kopieren

Kopieren Sie `JsonDiffPatch.dll` in das Hauptverzeichnis Ihres GameMaker-Projekts.

### Schritt 2: External Functions definieren

Erstellen Sie ein Script in GameMaker mit folgendem Code:

```gml
// DLL-Funktionen einmalig beim Spielstart laden
if (!variable_global_exists("jsondiffpatch_loaded")) {
    global.jsondiffpatch_loaded = true;
    
    global.JDP_Diff = external_define("JsonDiffPatch.dll", "JDP_Diff", dll_cdecl, ty_string, 2, ty_string, ty_string);
    global.JDP_Patch = external_define("JsonDiffPatch.dll", "JDP_Patch", dll_cdecl, ty_string, 2, ty_string, ty_string);
    global.JDP_Unpatch = external_define("JsonDiffPatch.dll", "JDP_Unpatch", dll_cdecl, ty_string, 2, ty_string, ty_string);
}
```

### Schritt 3: Wrapper-Funktionen erstellen

```gml
function jsondiffpatch_diff(left_json, right_json) {
    return external_call(global.JDP_Diff, left_json, right_json);
}

function jsondiffpatch_patch(original_json, patch_json) {
    return external_call(global.JDP_Patch, original_json, patch_json);
}

function jsondiffpatch_unpatch(modified_json, patch_json) {
    return external_call(global.JDP_Unpatch, modified_json, patch_json);
}
```

## Beispiel-Verwendung

```gml
// JSON-Strings definieren
var json1 = "{\"name\": \"John\", \"age\": 30}";
var json2 = "{\"name\": \"John\", \"age\": 31, \"city\": \"New York\"}";

// Unterschiede berechnen
var diff = jsondiffpatch_diff(json1, json2);
show_debug_message("Diff: " + diff);

// Patch anwenden
var patched = jsondiffpatch_patch(json1, diff);
show_debug_message("Patched: " + patched);

// Patch rückgängig machen
var original = jsondiffpatch_unpatch(patched, diff);
show_debug_message("Original: " + original);
```

## Verfügbare Funktionen

### JDP_Diff(json_left, json_right)
- **Beschreibung**: Berechnet die Unterschiede zwischen zwei JSON-Strings
- **Parameter**: 
  - `json_left`: Original JSON-String
  - `json_right`: Modifizierter JSON-String
- **Rückgabe**: Diff-Patch als JSON-String

### JDP_Patch(json_left, patch_json)
- **Beschreibung**: Wendet einen Patch auf einen JSON-String an
- **Parameter**: 
  - `json_left`: Original JSON-String
  - `patch_json`: Patch-JSON-String
- **Rückgabe**: Gepatchter JSON-String

### JDP_Unpatch(json_right, patch_json)
- **Beschreibung**: Macht einen Patch rückgängig
- **Parameter**: 
  - `json_right`: Modifizierter JSON-String
  - `patch_json`: Patch-JSON-String
- **Rückgabe**: Original JSON-String

## Fehlerbehandlung

Alle Funktionen geben einen leeren String zurück, wenn ein Fehler auftritt. Es wird empfohlen, die Rückgabewerte zu überprüfen:

```gml
var result = jsondiffpatch_diff(json1, json2);
if (result == "") {
    show_debug_message("Fehler beim Erstellen des Diffs!");
} else {
    show_debug_message("Diff erstellt: " + result);
}
```

## Debugging

- Verwenden Sie `show_debug_message()` um die JSON-Strings und Ergebnisse zu überprüfen
- Stellen Sie sicher, dass die DLL im gleichen Verzeichnis wie die .exe liegt
- Bei Problemen überprüfen Sie die Output-Konsole von GameMaker auf Fehlermeldungen

## Anmerkungen

- Die DLL verwendet UTF-8 Encoding für Strings
- Sehr große JSON-Dateien können die Performance beeinträchtigen
- Die DLL ist thread-safe, aber GameMaker selbst ist single-threaded