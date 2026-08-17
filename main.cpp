// Agri Life Simulator - Windows Launcher
// Compila con: g++ -o AgriLifeSimulator.exe main.cpp -mwindows -lshell32 -lshlwapi -municode -O2 -std=c++17
// Oppure usa build.bat

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "shlwapi.lib")

// ---------------------------------------------------------------------------
// Sistema di log: ad ogni avvio scrive in log.txt (stessa cartella dell'exe)
// i passaggi del caricamento, cosi da poter capire cosa sta caricando il
// gioco o, in caso di problemi, a che punto si è bloccato.
// ---------------------------------------------------------------------------
static std::string g_logPath;

std::string WStringToUtf8(const std::wstring& wstr)
{
    if (wstr.empty()) return std::string();
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
    std::string result(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &result[0], size, nullptr, nullptr);
    return result;
}

std::wstring GetTimeStamp()
{
    SYSTEMTIME st;
    GetLocalTime(&st);
    std::wstringstream ss;
    ss << std::setfill(L'0') << std::setw(2) << st.wHour   << L":"
       << std::setfill(L'0') << std::setw(2) << st.wMinute << L":"
       << std::setfill(L'0') << std::setw(2) << st.wSecond;
    return ss.str();
}

// Crea (o svuota) log.txt all'avvio del launcher
void LogInit(const std::wstring& exeDir)
{
    g_logPath = WStringToUtf8(exeDir + L"\\log.txt");
    std::ofstream file(g_logPath, std::ios::out | std::ios::trunc | std::ios::binary);
    if (file.is_open())
    {
        // BOM UTF-8: fa leggere correttamente le lettere accentate anche al Blocco Note
        const unsigned char bom[] = { 0xEF, 0xBB, 0xBF };
        file.write(reinterpret_cast<const char*>(bom), sizeof(bom));
    }
}

// Aggiunge una riga con orario al log
void Log(const std::wstring& message)
{
    std::ofstream file(g_logPath, std::ios::out | std::ios::app | std::ios::binary);
    if (file.is_open())
        file << WStringToUtf8(L"[" + GetTimeStamp() + L"] " + message) << "\r\n";
}

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
    LogInit(exeDir);
    Log(L"Avvio di Agri Life Simulator");
    Log(L"Cartella del programma: " + exeDir);

    std::wstring htmlFile = exeDir + L"\\agri-life-simulator-menu.html";

    // Verifica che il file HTML esista
    Log(L"Verifica presenza del menu principale...");
    if (GetFileAttributesW(htmlFile.c_str()) == INVALID_FILE_ATTRIBUTES)
    {
        Log(L"ERRORE: agri-life-simulator-menu.html non trovato.");
        MessageBoxW(nullptr,
            L"File non trovato:\nagri-life-simulator-menu.html\n\n"
            L"Assicurati che si trovi nella stessa cartella dell'eseguibile.",
            L"Agri Life Simulator — Errore",
            MB_ICONERROR | MB_OK);
        return 1;
    }
    Log(L"Menu principale trovato.");

    // Cerca browser compatibile (Edge prima, poi Chrome)
    Log(L"Ricerca di Microsoft Edge...");
    std::wstring browser = FindEdge();
    if (!browser.empty())
    {
        Log(L"Microsoft Edge trovato: " + browser);
    }
    else
    {
        Log(L"Microsoft Edge non trovato. Ricerca di Google Chrome...");
        browser = FindChrome();
        if (!browser.empty())
            Log(L"Google Chrome trovato: " + browser);
        else
            Log(L"Nessun browser compatibile (Edge/Chrome) trovato sul sistema.");
    }

    HANDLE hProc = nullptr;

    if (!browser.empty())
    {
        // Modalità app nativa (nessuna barra browser)
        Log(L"Avvio del gioco a schermo intero...");
        hProc = LaunchBrowserApp(browser, htmlFile);
        Log(hProc ? L"Gioco avviato correttamente." : L"Avvio a schermo intero non riuscito, provo un metodo alternativo...");
    }

    if (!hProc)
    {
        // Fallback: apri con il browser predefinito del sistema
        // (sarà in finestra normale, ma funzionerà sempre)
        Log(L"Apertura con il browser predefinito del sistema...");
        int result = (int)(INT_PTR)ShellExecuteW(
            nullptr, L"open", htmlFile.c_str(), nullptr, nullptr, SW_SHOWMAXIMIZED);

        if (result <= 32)
        {
            Log(L"ERRORE: impossibile aprire il file HTML (codice " + std::to_wstring(result) + L").");
            MessageBoxW(nullptr,
                L"Impossibile aprire il file HTML.\n"
                L"Installa Microsoft Edge o Google Chrome.",
                L"Agri Life Simulator — Errore",
                MB_ICONERROR | MB_OK);
            return 1;
        }
        Log(L"Gioco aperto con il browser predefinito.");
        return 0;
    }

    // Attendi che il browser venga chiuso (così l'app dura finché il gioco è aperto)
    Log(L"Gioco in esecuzione. In attesa della chiusura...");
    WaitForSingleObject(hProc, INFINITE);
    CloseHandle(hProc);
    Log(L"Gioco chiuso. Chiusura del launcher.");

    return 0;
}
