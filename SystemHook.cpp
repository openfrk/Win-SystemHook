#include <windows.h>
#include <gdiplus.h>
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>

#pragma comment (lib,"Gdiplus.lib")

#define APP_ICON 1

using namespace Gdiplus;
namespace fs = std::filesystem;

ULONG_PTR gdiplusToken;

// Initialize GDI+ for image processing
void InitGDIPlus() {
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
}

// Shutdown GDI+ to free resources
void ShutdownGDIPlus() {
    GdiplusShutdown(gdiplusToken);
}

// Save an extracted HICON to a PNG file
bool SaveIconToPng(HICON hIcon, const std::wstring& filePath) {
    Bitmap* bmp = Bitmap::FromHICON(hIcon);
    if (!bmp) return false;
    
    CLSID pngClsid;
    CLSIDFromString(L"{557CF406-1A04-11D3-9A73-0000F81EF32E}", &pngClsid); // PNG Format ID
    
    Status status = bmp->Save(filePath.c_str(), &pngClsid, NULL);
    delete bmp;
    return status == Ok;
}

// Extract all icons from a specific file (DLL, EXE, etc.)
void ExtractIconsFromFile(const std::wstring& filePath, const std::wstring& outputDir, int& totalExtracted) {
    int iconCount = ExtractIconExW(filePath.c_str(), -1, NULL, NULL, 0);
    if (iconCount <= 0) return;

    std::wstring baseName = fs::path(filePath).stem().wstring();
    
    for (int i = 0; i < iconCount; ++i) {
        HICON largeIcon = NULL;
        ExtractIconExW(filePath.c_str(), i, &largeIcon, NULL, 1);
        
        if (largeIcon) {
            std::wstring fileName = outputDir + L"\\" + baseName + L"_icon_" + std::to_wstring(i) + L".png";
            if (SaveIconToPng(largeIcon, fileName)) {
                totalExtracted++;
            }
            DestroyIcon(largeIcon);
        }
    }
}

// Extract the embedded PowerShell script from the EXE, run it, and delete it
void GenerateHtmlBase64(const std::wstring& imagePath, const std::wstring& txtPath) {
    // 1. Find the embedded resource
    HRSRC hRes = FindResourceW(NULL, L"POWERSHELL_SCRIPT", RT_RCDATA);
    if (!hRes) {
        std::wcout << L"[-] Embedded PowerShell script not found.\n";
        return;
    }

    // 2. Load the resource into memory
    HGLOBAL hResData = LoadResource(NULL, hRes);
    if (!hResData) return;

    DWORD size = SizeofResource(NULL, hRes);
    void* pData = LockResource(hResData);

    // 3. Create a temporary .ps1 file and write the embedded data into it
    wchar_t tempPath[MAX_PATH];
    GetTempPathW(MAX_PATH, tempPath);
    std::wstring tempScriptPath = std::wstring(tempPath) + L"SystemHook_temp.ps1";
    
    std::ofstream outFile(std::filesystem::path(tempScriptPath), std::ios::binary);
    outFile.write(static_cast<char*>(pData), size);
    outFile.close();

    // 4. Execute the temporary PowerShell script
    std::wstring psCommand = L"powershell -NoProfile -ExecutionPolicy Bypass -File \"" + tempScriptPath + L"\" -ImagePath \"" + imagePath + L"\" -TxtPath \"" + txtPath + L"\"";
    _wsystem(psCommand.c_str());

    // 5. Clean up the temporary script file (Leave no trace)
    DeleteFileW(tempScriptPath.c_str());
}

// Function to execute the embedded GodMode PowerShell script
void RunGodMode(const std::wstring& outputDir) {
    HRSRC hRes = FindResourceW(NULL, L"GODMODE_SCRIPT", RT_RCDATA);
    if (!hRes) {
        std::wcout << L"[-] GodMode script not found.\n";
        return;
    }
    
    HGLOBAL hResData = LoadResource(NULL, hRes);
    DWORD size = SizeofResource(NULL, hRes);
    void* pData = LockResource(hResData);

    wchar_t tempPath[MAX_PATH];
    GetTempPathW(MAX_PATH, tempPath);
    std::wstring tempScriptPath = std::wstring(tempPath) + L"GodMode_temp.ps1";
    
    std::ofstream outFile(std::filesystem::path(tempScriptPath), std::ios::binary);
    outFile.write(static_cast<char*>(pData), size);
    outFile.close();

    std::wstring psCommand = L"powershell -NoProfile -ExecutionPolicy Bypass -File \"" + tempScriptPath + L"\" -OutputDir \"" + outputDir + L"\" -VerboseOutput";
    
    std::wcout << L"[*] Extracting GodMode shortcuts...\n";
    _wsystem(psCommand.c_str());

    DeleteFileW(tempScriptPath.c_str());
}

int wmain(int argc, wchar_t* argv[]) {
    InitGDIPlus();

    if (argc < 2) {
        std::wcout << L"Usage:\n";
        std::wcout << L"  SystemHook run --OtherApp <exe_path>\n";
        std::wcout << L"  SystemHook run --GodMode <Output_Directory>\n";
        std::wcout << L"  SystemHook get --Icos <Output_Directory>\n";
        std::wcout << L"  SystemHook get --AppIcon <Output_Dir> <exe_name> [--HtmlLang <txt_path>]\n";
        ShutdownGDIPlus();
        return 1;
    }

    std::wstring command = argv[1];

    if (command == L"run") {
        if (argc >= 4 && std::wstring(argv[2]) == L"--OtherApp") {
            ShellExecuteW(NULL, L"open", argv[3], NULL, NULL, SW_SHOWNORMAL);
        }
        else if (argc >= 4 && std::wstring(argv[2]) == L"--GodMode") {
            RunGodMode(argv[3]);
        }
    }
    else if (command == L"get") {
        if (argc >= 4 && std::wstring(argv[2]) == L"--Icos") {
            // Scan all DLLs in the System32 directory
            std::wstring outDir = argv[3];
            fs::create_directories(outDir);
            std::wstring sys32Path = L"C:\\Windows\\System32";
            
            int total = 0;
            std::wcout << L"[*] Scanning DLLs in " << sys32Path << L". This may take a while...\n";
            
            for (const auto& entry : fs::directory_iterator(sys32Path)) {
                if (entry.path().extension() == L".dll") {
                    ExtractIconsFromFile(entry.path().wstring(), outDir, total);
                }
            }
            std::wcout << L"[+] Successfully extracted " << total << L" icons to: " << outDir << L"\n";
        }
        else if (argc >= 5 && std::wstring(argv[2]) == L"--AppIcon") {
            std::wstring outDir = argv[3];
            std::wstring exeName = argv[4];
            fs::create_directories(outDir);
            
            wchar_t exePath[MAX_PATH];
            if (SearchPathW(NULL, exeName.c_str(), L".exe", MAX_PATH, exePath, NULL)) {
                int total = 0;
                ExtractIconsFromFile(exePath, outDir, total);
                std::wcout << L"[+] Extracted icons for " << exeName << L"\n";
                
                // If HTML output is requested, convert the first icon to Base64
                if (argc >= 7 && std::wstring(argv[5]) == L"--HtmlLang") {
                    std::wstring firstIcon = outDir + L"\\" + fs::path(exePath).stem().wstring() + L"_icon_0.png";
                    GenerateHtmlBase64(firstIcon, argv[6]);
                }
            } else {
                std::wcout << L"[-] Application not found: " << exeName << L"\n";
            }
        }
    }

    ShutdownGDIPlus();
    return 0;
}