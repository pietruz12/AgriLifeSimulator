// Farm Life Simulator - Windows Launcher
// Compila con: g++ -o FarmLifeSimulator.exe main.cpp -mwindows -lshell32 -lshlwapi -municode -O2 -std=c++17
// Oppure usa build.bat

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <string>
#include <vector>

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "shlwapi.lib")

// ---------------------------------------------------------------------------
// Utility: trova il percorso dell'exe corrente
// ---------------------------------------------------------------------------
std::wstring GetExeDir()
{
    wchar_t buf[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, buf, MAX_PATH);
    std::wstring path(buf);
    size_t pos = path.rfind(L'\\');
    if (pos != std::wstring::npos)
        path = path.substr(0, pos);
    return path;
}

// ---------------------------------------------------------------------------
// Cerca Edge (msedge.exe) nei percorsi standard
// ---------------------------------------------------------------------------
std::wstring FindEdge()
{
    const wchar_t* candidates[] = {
        L"C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe",
        L"C:\\Program Files\\Microsoft\\Edge\\Application\\msedge.exe",
        nullptr
    };
    for (int i = 0; candidates[i]; ++i)
        if (GetFileAttributesW(candidates[i]) != INVALID_FILE_ATTRIBUTES)
            return candidates[i];

    // prova via PATH
    wchar_t found[MAX_PATH] = {};
    if (SearchPathW(nullptr, L"msedge.exe", nullptr, MAX_PATH, found, nullptr))
        return found;

    return L"";
}

// ---------------------------------------------------------------------------
// Cerca Chrome
// ---------------------------------------------------------------------------
std::wstring FindChrome()
{
    const wchar_t* candidates[] = {
        L"C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe",
        L"C:\\Program Files (x86)\\Google\\Chrome\\Application\\chrome.exe",
        nullptr
    };
    for (int i = 0; candidates[i]; ++i)
        if (GetFileAttributesW(candidates[i]) != INVALID_FILE_ATTRIBUTES)
            return candidates[i];
    return L"";
}

// ---------------------------------------------------------------------------
// Apre il browser in modalità app (kiosk-like, nessuna barra browser)
// Restituisce il HANDLE del processo lanciato
// ---------------------------------------------------------------------------
HANDLE LaunchBrowserApp(const std::wstring& browserPath, const std::wstring& htmlPath)
{
    // Crea URL file:/// con slash forward
    std::wstring url = L"file:///" + htmlPath;
    for (auto& c : url)
        if (c == L'\\') c = L'/';

    // Argomenti: --app apre senza chrome UI, --start-fullscreen = schermo intero
    std::wstring args = L"\"" + browserPath + L"\" "
        L"--app=\"" + url + L"\" "
        L"--start-fullscreen "
        L"--disable-infobars "
        L"--no-first-run "
        L"--no-default-browser-check";

    STARTUPINFOW si = {};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOW;

    PROCESS_INFORMATION pi = {};

    // Copia args in buffer modificabile
    std::vector<wchar_t> argBuf(args.begin(), args.end());
    argBuf.push_back(L'\0');

    if (CreateProcessW(
            browserPath.c_str(),
            argBuf.data(),
            nullptr, nullptr,
            FALSE,
            0,
            nullptr,
            nullptr,
            &si, &pi))
    {
        CloseHandle(pi.hThread);
        return pi.hProcess;
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// WinMain
// ---------------------------------------------------------------------------
int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    std::wstring exeDir  = GetExeDir();
    std::wstring htmlFile = exeDir + L"\\farm-life-simulator-menu.html";

    // Verifica che il file HTML esista
    if (GetFileAttributesW(htmlFile.c_str()) == INVALID_FILE_ATTRIBUTES)
    {
        MessageBoxW(nullptr,
            L"File non trovato:\nfarm-life-simulator-menu.html\n\n"
            L"Assicurati che si trovi nella stessa cartella dell'eseguibile.",
            L"Farm Life Simulator — Errore",
            MB_ICONERROR | MB_OK);
        return 1;
    }

    // Cerca browser compatibile (Edge prima, poi Chrome)
    std::wstring browser = FindEdge();
    if (browser.empty())
        browser = FindChrome();

    HANDLE hProc = nullptr;

    if (!browser.empty())
    {
        // Modalità app nativa (nessuna barra browser)
        hProc = LaunchBrowserApp(browser, htmlFile);
    }

    if (!hProc)
    {
        // Fallback: apri con il browser predefinito del sistema
        // (sarà in finestra normale, ma funzionerà sempre)
        int result = (int)(INT_PTR)ShellExecuteW(
            nullptr, L"open", htmlFile.c_str(), nullptr, nullptr, SW_SHOWMAXIMIZED);

        if (result <= 32)
        {
            MessageBoxW(nullptr,
                L"Impossibile aprire il file HTML.\n"
                L"Installa Microsoft Edge o Google Chrome.",
                L"Farm Life Simulator — Errore",
                MB_ICONERROR | MB_OK);
            return 1;
        }
        return 0;
    }

    // Attendi che il browser venga chiuso (così l'app dura finché il gioco è aperto)
    WaitForSingleObject(hProc, INFINITE);
    CloseHandle(hProc);

    return 0;
}
