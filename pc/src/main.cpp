// ============================================================
//  SJRC 自動閱讀器 — 原生 Windows 版本（C++ / Win32）
//  - 心跳與翻頁在背景執行緒跑，關窗即停並收尾
//  - Token 由網頁版「複製 token」貼進來（Google 登入仍需在 localhost 完成）
//  - 單一 exe，無外部依賴、無設定檔（設定存登錄檔）
// ============================================================
#define UNICODE
#define _UNICODE

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <winhttp.h>
#include <commctrl.h>
#include <cctype>
#include <string>
#include <vector>
#include <deque>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cwchar>
#include <cstring>
#include <functional>
#include <gdiplus.h>

#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "ws2_32.lib")

// ---------------- 全域 ----------------
static const wchar_t* APP_TITLE = L"SJRC 自動閱讀器";
static const wchar_t* API_HOST = L"api.sjrc.club";
static const wchar_t* API_VER = L"2.9.0";

static HINSTANCE g_hInst = nullptr;
static HWND g_hWnd = nullptr;
static HICON g_hIconBig = nullptr;

static HFONT g_fontUI = nullptr;     // 一般文字
static HFONT g_fontUIB = nullptr;    // 粗體
static HFONT g_fontBig = nullptr;    // 大字
static HFONT g_fontMono = nullptr;   // 日誌
static HFONT g_fontSmall = nullptr;  // 小字

static HBRUSH g_brBg = nullptr;
static HBRUSH g_brCard = nullptr;
static HBRUSH g_brInput = nullptr;
static HBRUSH g_brLog = nullptr;

// 顏色（對齊網頁版）
static const COLORREF C_BG = RGB(0xF5, 0xF7, 0xF6);
static const COLORREF C_CARD = RGB(0xFF, 0xFF, 0xFF);
static const COLORREF C_LINE = RGB(0xE3, 0xEA, 0xE7);
static const COLORREF C_TXT = RGB(0x2B, 0x3A, 0x36);
static const COLORREF C_DIM = RGB(0x5F, 0x71, 0x6C);
static const COLORREF C_ACC = RGB(0x3D, 0xCA, 0xB1);
static const COLORREF C_ACCD = RGB(0x1F, 0x85, 0x74);
static const COLORREF C_WARN = RGB(0xE8, 0xA3, 0x3D);
static const COLORREF C_ERR = RGB(0xE0, 0x60, 0x5E);
static const COLORREF C_INPUT = RGB(0xFB, 0xFD, 0xFC);

// 控制項 ID
enum {
    ID_TOKEN = 1000, ID_BTN_IMPORT, ID_TOKEN_STATE,
    ID_TARGET, ID_HB, ID_TURN,
    ID_BTN_START, ID_BTN_STOP,
    ID_LOG, ID_TIMER = 1,
    ID_ST_BOOK, ID_ST_AUTHOR, ID_ST_BOOKID, ID_ST_CHAPTERS, ID_ST_PAGE,
    ID_ST_SESSION, ID_ST_LASTHB, ID_ST_TOKEN,
    ID_ST_HB, ID_ST_TURN, ID_ST_BOOKN, ID_ST_ERR,
    ID_ST_QUOTA, ID_ST_QUOTA_LEFT,
    // 帳號 / 閱讀數據卡（對齊網頁版 step-account）
    ID_ST_NAME, ID_ST_EMAIL, ID_ST_SCHOOL, ID_ST_IDENT,
    ID_ST_TOTALUSED, ID_ST_READCNT, ID_ST_SCHOOLQ,
    ID_ST_RUNLINE, ID_BTN_REFRESH,
    ID_OAUTH_URL = 1020, ID_BTN_OAUTH,
    WM_APP_STATE = WM_APP + 1, WM_APP_IMPORT = WM_APP + 2, WM_APP_IMPORT_CLIPBOARD = WM_APP + 3,
    WM_APP_GOOGLE_CRED = WM_APP + 4
};

// ---------------- 小工具 ----------------
static std::wstring widen(const std::string& s) {
    if (s.empty()) return L"";
    int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
    std::wstring w(n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), &w[0], n);
    return w;
}
static std::string narrow(const std::wstring& w) {
    if (w.empty()) return "";
    int n = WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), nullptr, 0, nullptr, nullptr);
    std::string s(n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), &s[0], n, nullptr, nullptr);
    return s;
}
static std::wstring fmt(const wchar_t* f, ...) {
    wchar_t buf[1024];
    va_list ap;
    va_start(ap, f);
    _vsnwprintf_s(buf, _countof(buf), _TRUNCATE, f, ap);
    va_end(ap);
    return buf;
}
static std::wstring nowTime() {
    SYSTEMTIME st;
    GetLocalTime(&st);
    return fmt(L"%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
}

// ---------------- 極簡 JSON ----------------
struct JVal {
    enum Type { NUL, BOOL, NUM, STR, ARR, OBJ } type = NUL;
    bool b = false;
    double num = 0;
    std::string str;
    std::vector<JVal> arr;
    std::vector<std::pair<std::string, JVal>> obj;

    const JVal* find(const char* k) const {
        if (type != OBJ) return nullptr;
        for (auto& kv : obj) if (kv.first == k) return &kv.second;
        return nullptr;
    }
    std::wstring wstr() const { return widen(str); }
    double numOr(double d) const { return type == NUM ? num : d; }
};

struct JParser {
    const char* p;
    const char* e;
    explicit JParser(const std::string& s) : p(s.data()), e(s.data() + s.size()) {}
    void ws() { while (p < e && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) ++p; }
    bool lit(const char* s) {
        size_t n = strlen(s);
        if ((size_t)(e - p) < n || strncmp(p, s, n) != 0) return false;
        p += n;
        return true;
    }
    static void putUtf8(std::string& out, unsigned cp) {
        if (cp < 0x80) out += (char)cp;
        else if (cp < 0x800) { out += (char)(0xC0 | (cp >> 6)); out += (char)(0x80 | (cp & 0x3F)); }
        else if (cp < 0x10000) {
            out += (char)(0xE0 | (cp >> 12)); out += (char)(0x80 | ((cp >> 6) & 0x3F)); out += (char)(0x80 | (cp & 0x3F));
        } else {
            out += (char)(0xF0 | (cp >> 18)); out += (char)(0x80 | ((cp >> 12) & 0x3F));
            out += (char)(0x80 | ((cp >> 6) & 0x3F)); out += (char)(0x80 | (cp & 0x3F));
        }
    }
    bool parseStr(std::string& out) {
        if (p >= e || *p != '"') return false;
        ++p;
        out.clear();
        while (p < e) {
            char c = *p++;
            if (c == '"') return true;
            if (c == '\\') {
                if (p >= e) return false;
                char d = *p++;
                switch (d) {
                case 'n': out += '\n'; break;
                case 't': out += '\t'; break;
                case 'r': out += '\r'; break;
                case 'b': out += '\b'; break;
                case 'f': out += '\f'; break;
                case 'u': {
                    if (e - p < 4) return false;
                    unsigned cp = 0;
                    for (int i = 0; i < 4; ++i) {
                        char h = *p++;
                        cp <<= 4;
                        if (h >= '0' && h <= '9') cp |= (h - '0');
                        else if (h >= 'a' && h <= 'f') cp |= (h - 'a' + 10);
                        else if (h >= 'A' && h <= 'F') cp |= (h - 'A' + 10);
                        else return false;
                    }
                    if (cp >= 0xD800 && cp <= 0xDBFF && e - p >= 6 && p[0] == '\\' && p[1] == 'u') {
                        p += 2;
                        unsigned lo = 0;
                        for (int i = 0; i < 4; ++i) {
                            char h = *p++;
                            lo <<= 4;
                            if (h >= '0' && h <= '9') lo |= (h - '0');
                            else if (h >= 'a' && h <= 'f') lo |= (h - 'a' + 10);
                            else if (h >= 'A' && h <= 'F') lo |= (h - 'A' + 10);
                        }
                        cp = 0x10000 + ((cp - 0xD800) << 10) + (lo - 0xDC00);
                    }
                    putUtf8(out, cp);
                    break;
                }
                default: out += d;
                }
            } else out += c;
        }
        return false;
    }
    bool value(JVal& v, int depth = 0) {
        if (depth > 64) return false;
        ws();
        if (p >= e) return false;
        char c = *p;
        if (c == '"') { v.type = JVal::STR; return parseStr(v.str); }
        if (c == '{') {
            ++p; v.type = JVal::OBJ; ws();
            if (p < e && *p == '}') { ++p; return true; }
            while (p < e) {
                ws();
                std::string k;
                if (!parseStr(k)) return false;
                ws();
                if (p >= e || *p != ':') return false;
                ++p;
                JVal child;
                if (!value(child, depth + 1)) return false;
                v.obj.emplace_back(std::move(k), std::move(child));
                ws();
                if (p < e && *p == ',') { ++p; continue; }
                if (p < e && *p == '}') { ++p; return true; }
                return false;
            }
            return false;
        }
        if (c == '[') {
            ++p; v.type = JVal::ARR; ws();
            if (p < e && *p == ']') { ++p; return true; }
            while (p < e) {
                JVal child;
                if (!value(child, depth + 1)) return false;
                v.arr.push_back(std::move(child));
                ws();
                if (p < e && *p == ',') { ++p; continue; }
                if (p < e && *p == ']') { ++p; return true; }
                return false;
            }
            return false;
        }
        if (lit("true")) { v.type = JVal::BOOL; v.b = true; return true; }
        if (lit("false")) { v.type = JVal::BOOL; v.b = false; return true; }
        if (lit("null")) { v.type = JVal::NUL; return true; }
        {
            char* end = nullptr;
            double d = strtod(p, &end);
            if (end == p) return false;
            p = end;
            v.type = JVal::NUM;
            v.num = d;
            return true;
        }
    }
};

static bool parseJson(const std::string& s, JVal& out) {
    JParser jp(s);
    if (!jp.value(out)) return false;
    return true;
}

// ---------------- HTTP（WinHTTP） ----------------
struct HttpResp {
    int status = 0;
    std::string body;
    std::wstring error;
    bool transportOk = false;
};

static std::string g_token;         // accessToken
static std::string g_refresh;       // refreshToken
static std::mutex g_mtx;

static HttpResp httpCall(const std::wstring& method, const std::wstring& path,
                         const std::string& body, bool auth, const wchar_t* schema) {
    HttpResp r;
    HINTERNET hs = WinHttpOpen(L"SJRC-AutoReader/2.9.0",
                               WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                               WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hs) { r.error = L"WinHttpOpen 失敗"; return r; }
    WinHttpSetTimeouts(hs, 8000, 8000, 10000, 15000);

    HINTERNET hc = WinHttpConnect(hs, API_HOST, INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hc) { WinHttpCloseHandle(hs); r.error = L"連線失敗"; return r; }

    HINTERNET hr = WinHttpOpenRequest(hc, method.c_str(), path.c_str(), nullptr,
                                      WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
                                      WINHTTP_FLAG_SECURE);
    if (!hr) { WinHttpCloseHandle(hc); WinHttpCloseHandle(hs); r.error = L"建立請求失敗"; return r; }

    std::wstring headers;
    headers += L"version: "; headers += API_VER; headers += L"\r\n";
    headers += L"x-current-platform: android\r\n";
    headers += L"x-current-locale: zh-HK\r\n";
    headers += L"x-auth-schema: ";
    headers += (schema ? schema : (auth ? L"CLIENT" : L"PUBLIC"));
    headers += L"\r\n";
    if (!body.empty()) headers += L"Content-Type: application/json\r\n";
    if (auth && !g_token.empty()) {
        headers += L"authorization: Bearer ";
        headers += widen(g_token);
        headers += L"\r\n";
    }

    BOOL ok = WinHttpSendRequest(hr, headers.c_str(), (DWORD)-1L,
                                 body.empty() ? WINHTTP_NO_REQUEST_DATA : (LPVOID)body.data(),
                                 (DWORD)body.size(), (DWORD)body.size(), 0);
    if (!ok) { r.error = L"送出失敗（" + fmt(L"%lu", GetLastError()) + L"）"; }
    else if (!WinHttpReceiveResponse(hr, nullptr)) {
        r.error = L"接收失敗（" + fmt(L"%lu", GetLastError()) + L"）";
    } else {
        DWORD code = 0, len = sizeof(code);
        WinHttpQueryHeaders(hr, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                            WINHTTP_HEADER_NAME_BY_INDEX, &code, &len, WINHTTP_NO_HEADER_INDEX);
        r.status = (int)code;
        DWORD avail = 0;
        do {
            avail = 0;
            if (!WinHttpQueryDataAvailable(hr, &avail)) break;
            if (!avail) break;
            std::string chunk(avail, '\0');
            DWORD read = 0;
            if (!WinHttpReadData(hr, &chunk[0], avail, &read)) break;
            chunk.resize(read);
            r.body += chunk;
        } while (avail > 0);
        r.transportOk = true;
    }

    WinHttpCloseHandle(hr);
    WinHttpCloseHandle(hc);
    WinHttpCloseHandle(hs);
    return r;
}

// ---------------- 狀態 ----------------
struct Snapshot {
    std::wstring book, author, session, lastHb, tokenState, user;
    long long bookId = 0, quotaTotal = 0, quotaUsed = 0, sentSec = 0;
    int chapters = 0, page = 0, totalPages = 0;
    int hbCount = 0, turnCount = 0, bookCount = 0, errorTime = 0;
    bool running = false;
    int targetX10 = 20;   // 目標小時 x10

    // ---- 帳號（GET /v1/client/users/detail） ----
    std::wstring email, name, school, acctType, serialNo, schoolRegion;
    std::wstring lastLogin;
    bool acctOk = false;

    // ---- 閱讀數據（GET /v1/client/reading-time） ----
    // 注意：伺服器全部以「秒」為單位，顯示時 /3600 轉小時
    long long totalUsedQuota = 0;      // 累計已讀（含歷史）
    long long schoolTotalQuota = 0;    // 全校購買額度
    long long schoolUsedQuota = 0;     // 全校已用
    int readCount = 0;                 // 已讀書本數
    int reflectionCount = 0;           // 閱讀心得數
    bool sharing = false;              // 是否分享中
    bool quotaOk = false;
};

static Snapshot g_snap;
static std::deque<std::wstring> g_pendingLog;
static std::atomic<bool> g_stop{false};
static std::thread g_worker;
static bool g_sessionAlive = false;

static void logLine(const std::wstring& s) {
    std::lock_guard<std::mutex> lk(g_mtx);
    g_pendingLog.push_back(L"[" + nowTime() + L"] " + s);
    if (g_pendingLog.size() > 600) g_pendingLog.pop_front();
}

static void setSnap(const std::function<void(Snapshot&)>& fn) {
    std::lock_guard<std::mutex> lk(g_mtx);
    fn(g_snap);
}

static void requestRepaint() {
    if (g_hWnd) PostMessageW(g_hWnd, WM_APP_STATE, 0, 0);
}

// ---------------- 設定持久化（登錄檔） ----------------
static const wchar_t* REG_PATH = L"Software\\SJRC\\AutoReader";

static void saveSetting(const wchar_t* name, const std::wstring& val) {
    HKEY k;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, REG_PATH, 0, nullptr, 0, KEY_WRITE, nullptr, &k, nullptr) == ERROR_SUCCESS) {
        RegSetValueExW(k, name, 0, REG_SZ, (const BYTE*)val.c_str(), (DWORD)((val.size() + 1) * sizeof(wchar_t)));
        RegCloseKey(k);
    }
}
static std::wstring loadSetting(const wchar_t* name, const wchar_t* def) {
    HKEY k;
    std::wstring out = def ? def : L"";
    if (RegOpenKeyExW(HKEY_CURRENT_USER, REG_PATH, 0, KEY_READ, &k) == ERROR_SUCCESS) {
        wchar_t buf[4096];
        DWORD type = 0, sz = sizeof(buf) - 2;
        if (RegQueryValueExW(k, name, nullptr, &type, (BYTE*)buf, &sz) == ERROR_SUCCESS && type == REG_SZ) {
            buf[sz / sizeof(wchar_t)] = 0;
            out = buf;
        }
        RegCloseKey(k);
    }
    return out;
}

// ---------------- 業務邏輯 ----------------
struct ApiRes {
    bool ok = false;
    int code = -1;
    std::wstring msg;
    JVal json;
};

static ApiRes api(const wchar_t* method, const std::wstring& path, const std::string& body,
                  bool auth = true, const wchar_t* schema = nullptr) {
    ApiRes a;
    HttpResp r = httpCall(method, path, body, auth, schema);
    if (!r.transportOk) {
        a.code = -1;
        a.msg = r.error.empty() ? L"網路錯誤" : r.error;
        return a;
    }
    if (!parseJson(r.body, a.json)) {
        a.code = -2;
        a.msg = L"回應不是 JSON（HTTP " + fmt(L"%d", r.status) + L"）";
        return a;
    }
    const JVal* c = a.json.find("code");
    a.code = c ? (int)c->numOr(-1) : -1;
    const JVal* m = a.json.find("message");
    a.msg = m ? m->wstr() : L"";
    a.ok = (a.code == 200 || a.code == 201);
    return a;
}

static bool refreshToken() {
    if (g_refresh.empty()) { logLine(L"沒有 refreshToken，無法續期"); return false; }
    logLine(L"→ token refresh");
    std::string body = "{\"refreshToken\":\"" + g_refresh + "\"}";
    // Swagger: 此端點 security=bearer，必須帶目前的 accessToken
    ApiRes a = api(L"POST", L"/v1/public/accounts/users/token/refresh", body, true, L"PUBLIC");
    if (a.ok) {
        const JVal* d = a.json.find("data");
        if (d) {
            const JVal* at = d->find("accessToken");
            const JVal* rt = d->find("refreshToken");
            if (at) g_token = at->str;
            if (rt) g_refresh = rt->str;
            saveSetting(L"token", widen(g_token));
            saveSetting(L"refresh", widen(g_refresh));
            logLine(L"← token refreshed");
            return true;
        }
    }
    logLine(L"refresh 失敗：" + fmt(L"%d", a.code) + L" " + a.msg);
    return false;
}

struct Book { long long id = 0; std::wstring title; };

static void collectBooks(const JVal& v, std::vector<Book>& out, int depth) {
    if (depth > 6) return;
    if (v.type == JVal::ARR) {
        for (auto& it : v.arr) collectBooks(it, out, depth + 1);
        return;
    }
    if (v.type == JVal::OBJ) {
        const JVal* id = v.find("bookId");
        if (!id || id->type != JVal::NUM) id = v.find("id");
        const JVal* nm = v.find("name");
        if (!nm) nm = v.find("bookName");
        // 只有「書」才算：作者／標籤節點同樣有 id + name（author 是 {id,name,authorId,profileImage}），
        // 舊寫法會把 "Stefan Al" 這種作者名當成書名收進清單，隨機選書就可能選到作者 id。
        bool looksBook = v.find("coverUrl") || v.find("cover_url") || v.find("isbn") ||
                         v.find("bookId") || v.find("authors") || v.find("price");
        if (looksBook && id && nm && id->type == JVal::NUM) {
            std::wstring title;
            if (nm->type == JVal::STR) title = nm->wstr();
            else if (nm->type == JVal::OBJ) {
                const char* keys[] = { "zh_HK", "zh_cn", "en_uk", "en_us" };
                for (auto k : keys) { const JVal* t = nm->find(k); if (t && t->type == JVal::STR) { title = t->wstr(); break; } }
            }
            if (!title.empty()) {
                out.push_back({ (long long)id->num, title });
                return;   // 是書就不往 authors / labels 裡遞迴
            }
        }
        for (auto& kv : v.obj) collectBooks(kv.second, out, depth + 1);
    }
}

static int countChapters(const JVal& arr, int depth) {
    if (arr.type != JVal::ARR || depth > 5) return 0;
    int n = 0;
    for (auto& c : arr.arr) {
        const JVal* kids = c.find("children");
        if (kids && kids->type == JVal::ARR && !kids->arr.empty()) n += countChapters(*kids, depth + 1);
        else ++n;
    }
    return n;
}

static bool startBook(const Book& b, int pagesPerChapter) {
    logLine(L"→ GET /v2/client/books/" + fmt(L"%lld", b.id));
    ApiRes d = api(L"GET", L"/v2/client/books/" + fmt(L"%lld", b.id), "", true);
    if (!d.ok) { logLine(L"書籍詳情失敗：" + fmt(L"%d", d.code) + L" " + d.msg); return false; }
    const JVal* data = d.json.find("data");
    if (!data) return false;

    std::wstring title = b.title;
    const JVal* nm = data->find("name");
    if (nm && nm->type == JVal::OBJ) {
        const char* keys[] = { "zh_HK", "zh_cn", "en_uk", "en_us" };
        for (auto k : keys) { const JVal* t = nm->find(k); if (t && t->type == JVal::STR && !t->str.empty()) { title = t->wstr(); break; } }
    }

    std::wstring author;
    const JVal* au = data->find("authors");
    if (au && au->type == JVal::ARR) {
        for (auto& a : au->arr) {
            const JVal* an = a.find("name");
            if (an && an->type == JVal::STR) { if (!author.empty()) author += L", "; author += an->wstr(); }
            else if (a.type == JVal::STR) { if (!author.empty()) author += L", "; author += a.wstr(); }
        }
    }

    int chapters = 1;
    const JVal* ch = data->find("chapters");
    if (ch) { int c = countChapters(*ch, 0); if (c > 0) chapters = c; }
    int totalPages = chapters * pagesPerChapter;

    logLine(L"→ POST reading-time/start {bookId:" + fmt(L"%lld", b.id) + L"}");
    ApiRes s = api(L"POST", L"/v1/client/reading-time/start", "{\"bookId\":" + std::to_string(b.id) + "}", true);
    if (!s.ok) {
        logLine(L"start 失敗：" + fmt(L"%d", s.code) + L" " + s.msg);
        return false;
    }
    const JVal* sd = s.json.find("data");
    std::string sid;
    long long qTotal = 0, qUsed = 0;
    if (sd) {
        const JVal* x = sd->find("sessionId");
        if (x && x->type == JVal::STR) sid = x->str;
        // 實測 start 回的是 {sessionId, total, usedQuota}，沒有 totalQuota！
        // 之前只讀 totalQuota → 0，把配額卡片覆蓋成 0.00 / 0.00。
        const JVal* t = sd->find("totalQuota");
        if (!t || t->type != JVal::NUM) t = sd->find("total");
        const JVal* u = sd->find("usedQuota");
        if (!u || u->type != JVal::NUM) u = sd->find("used");
        if (t && t->type == JVal::NUM) qTotal = (long long)t->num;
        if (u && u->type == JVal::NUM) qUsed = (long long)u->num;
    }
    if (sid.empty()) { logLine(L"start 回應沒有 sessionId"); return false; }

    setSnap([&](Snapshot& s2) {
        s2.book = title; s2.author = author.empty() ? L"—" : author;
        s2.bookId = b.id; s2.chapters = chapters;
        s2.page = 0; s2.totalPages = totalPages;
        s2.session = widen(sid);
        // 只有真的拿到值才覆蓋，避免又一次把配額歸零
        if (qTotal > 0) s2.quotaTotal = qTotal;
        if (qUsed > 0) s2.quotaUsed = qUsed;
        s2.bookCount++;
    });
    g_sessionAlive = true;
    logLine(L"← session=" + widen(sid).substr(0, 20) + L"… · 章節 " + fmt(L"%d", chapters) + L" · " + fmt(L"%d", totalPages) + L" 頁");
    return true;
}

static bool restartSession() {
    long long id = 0;
    {
        std::lock_guard<std::mutex> lk(g_mtx);
        id = g_snap.bookId;
    }
    if (!id) return false;
    ApiRes s = api(L"POST", L"/v1/client/reading-time/start", "{\"bookId\":" + std::to_string(id) + "}", true);
    if (s.ok) {
        const JVal* sd = s.json.find("data");
        const JVal* x = sd ? sd->find("sessionId") : nullptr;
        if (x && x->type == JVal::STR) {
            setSnap([&](Snapshot& s2) { s2.session = widen(x->str); });
            logLine(L"已重開讀書 session");
            return true;
        }
    }
    logLine(L"重開 session 失敗：" + fmt(L"%d", s.code) + L" " + s.msg);
    return false;
}

static bool sendReadPos() {
    long long id = 0; int page = 0, total = 0, ppc = 20;
    {
        std::lock_guard<std::mutex> lk(g_mtx);
        id = g_snap.bookId; page = g_snap.page; total = g_snap.totalPages;
        ppc = 20;
    }
    if (!id) return true;
    int ci = page / ppc;
    int pic = (page % ppc) + 1;
    double pos = (double)pic / ppc;
    char buf[512];
    snprintf(buf, sizeof(buf),
             "{\"bookId\":%lld,\"pos\":{\"chapterId\":\"%d\",\"chapterLocation\":%d,\"position\":%.4f}}",
             id, ci, pic, pos);
    ApiRes a = api(L"POST", L"/v1/client/reading-pos", buf, true);
    return a.ok;
}

static bool endSession() {
    std::wstring sid;
    {
        std::lock_guard<std::mutex> lk(g_mtx);
        sid = g_snap.session;
    }
    if (sid.empty()) return true;
    logLine(L"→ PATCH reading-time/end");
    ApiRes a = api(L"PATCH", L"/v1/client/reading-time/end",
                   "{\"sessionId\":\"" + narrow(sid) + "\"}", true);
    logLine(L"← " + fmt(L"%d", a.code) + L" " + a.msg);
    setSnap([](Snapshot& s) { s.session.clear(); });
    g_sessionAlive = false;
    return a.ok;
}

// ---- JSON 取值小工具（型別不符就回預設值，避免半途 crash）----
static std::wstring jstr(const JVal* o, const char* k) {
    if (!o) return L"";
    const JVal* v = o->find(k);
    if (!v || v->type != JVal::STR) return L"";
    return v->wstr();
}
static long long jnum(const JVal* o, const char* k) {
    if (!o) return 0;
    const JVal* v = o->find(k);
    if (!v || v->type != JVal::NUM) return 0;
    return (long long)v->num;
}
// 多語系名稱物件 → 取第一個有值的語系
static std::wstring localizedName(const JVal* nm) {
    if (!nm || nm->type != JVal::OBJ) return L"";
    const char* keys[] = { "zh_HK", "zh_hk", "zh_cn", "zh_CN", "en_uk", "en_us" };
    for (auto k : keys) {
        const JVal* t = nm->find(k);
        if (t && t->type == JVal::STR && !t->str.empty()) return t->wstr();
    }
    return L"";
}
static std::wstring fmtHours(long long sec) { return fmt(L"%.2f 小時", sec / 3600.0); }

static std::wstring typeZh(const std::wstring& t) {
    if (t == L"student") return L"學生";
    if (t == L"teacher") return L"教師";
    if (t == L"parent")  return L"家長";
    if (t == L"admin" || t == L"school-admin") return L"學校管理員";
    return t.empty() ? L"—" : t;
}

// GET /v1/client/users/detail → 帳號資料（網頁版 step-account 那幾行）
// pre 有值時直接用已抓到的 data，省一次請求
static bool loadUserDetail(const JVal* pre = nullptr) {
    ApiRes a;                       // 必須活到函式結束：d 指向 a.json 內部
    const JVal* d = pre;
    if (!d) {
        a = api(L"GET", L"/v1/client/users/detail", "", true);
        if (!a.ok && (a.code == 10117 || a.code == 10119)) {
            // token 過期 → 續期後重試一次（跟網頁版 loadAccount 一致）
            if (refreshToken()) a = api(L"GET", L"/v1/client/users/detail", "", true);
        }
        if (!a.ok) {
            logLine(L"users/detail → " + fmt(L"%d", a.code) + L" " + a.msg);
            return false;
        }
        d = a.json.find("data");
    }
    if (!d) return false;

    std::wstring email  = jstr(d, "email");
    std::wstring name   = jstr(d, "displayName");
    std::wstring given  = jstr(d, "givenName");
    std::wstring family = jstr(d, "familyName");
    if (name.empty()) {                       // 網頁版：displayName || givenName + familyName
        name = given;
        if (!family.empty()) { if (!name.empty()) name += L" "; name += family; }
    }
    const JVal* sc = d->find("school");
    std::wstring school = localizedName(sc ? sc->find("name") : nullptr);
    if (school.empty()) school = localizedName(d->find("schoolName"));   // 兼容網頁版用的欄位
    std::wstring region = sc ? jstr(sc, "region") : L"";
    std::wstring type   = typeZh(jstr(d, "type"));
    std::wstring serial = jstr(d, "serialNo");
    std::wstring login  = jstr(d, "lastLoginAt");

    setSnap([&](Snapshot& s) {
        s.acctOk = true;
        s.user = email;
        s.email = email;
        s.name = name.empty() ? L"—" : name;
        s.school = school.empty() ? L"—" : school;
        s.schoolRegion = region;
        s.acctType = type;
        s.serialNo = serial;
        s.lastLogin = login;
    });
    logLine(L"帳號：" + (email.empty() ? std::wstring(L"?") : email) +
            L" · " + (name.empty() ? L"—" : name) +
            L" · " + (school.empty() ? L"—" : school));
    return true;
}

// GET /v1/client/reading-time → 配額與閱讀統計（單位全部是「秒」）
// 心跳每秒一次，若每次都打這支 API + 印日誌，畫面會被同一行洗版。
// 所以：force=false 時 20 秒節流，且只有數值真的變了才寫日誌。
static bool loadQuota(bool force = false) {
    static std::chrono::steady_clock::time_point lastAt{};
    static long long lastUsed = -1, lastTotal = -1;
    auto now = std::chrono::steady_clock::now();
    if (!force && lastAt.time_since_epoch().count() != 0 &&
        std::chrono::duration_cast<std::chrono::seconds>(now - lastAt).count() < 20)
        return false;
    lastAt = now;

    ApiRes a = api(L"GET", L"/v1/client/reading-time", "", true);
    if (!a.ok) {
        logLine(L"reading-time → " + fmt(L"%d", a.code) + L" " + a.msg);
        return false;
    }
    const JVal* d = a.json.find("data");
    if (!d) return false;

    long long total = jnum(d, "totalQuota");
    if (!total) total = jnum(d, "total_quota");
    long long used = jnum(d, "usedQuota");
    if (!used) used = jnum(d, "used_quota");
    long long totalUsed  = jnum(d, "totalUsedQuota");
    long long schoolBuy  = jnum(d, "schoolTotalBoughtQuota");
    long long schoolUsed = jnum(d, "schoolUsedQuota");
    int cnt  = (int)jnum(d, "count");
    int refl = (int)jnum(d, "readingReflectionCount");
    const JVal* sh = d->find("isSharingTime");
    bool sharing = (sh && sh->type == JVal::BOOL) ? sh->b : false;

    setSnap([&](Snapshot& s) {
        s.quotaOk = true;
        s.quotaTotal = total;
        s.quotaUsed = used;
        s.totalUsedQuota = totalUsed;
        s.schoolTotalQuota = schoolBuy;
        s.schoolUsedQuota = schoolUsed;
        s.readCount = cnt;
        s.reflectionCount = refl;
        s.sharing = sharing;
    });
    if (used != lastUsed || total != lastTotal) {
        lastUsed = used; lastTotal = total;
        logLine(L"配額更新：" + fmtHours(used) + L" / " + fmtHours(total) +
                L" · 已讀 " + fmt(L"%d", cnt) + L" 本");
    }
    return true;
}

// 帳號 + 閱讀數據一次抓齊（登入後、啟動時、按「重新整理」都用這個）
static void loadAccountInfo() {
    loadUserDetail();
    loadQuota(true);
    requestRepaint();
}

static bool fetchAndStartRandomBook(int ppc, long long avoidId) {
    std::vector<Book> books;
    ApiRes a = api(L"GET", L"/v1/client/bookshelf/books?page=1&limit=50", "", true);
    if (a.ok) { const JVal* d = a.json.find("data"); if (d) collectBooks(*d, books, 0); }
    logLine(L"書架：" + fmt(L"%d", (int)books.size()) + L" 本");
    if (books.size() < 3) {
        ApiRes h = api(L"GET", L"/v1/client/homepage", "", true);
        if (h.ok) { const JVal* d = h.json.find("data"); if (d) collectBooks(*d, books, 0); }
        logLine(L"書庫：累計 " + fmt(L"%d", (int)books.size()) + L" 本");
    }
    // 去重
    std::vector<Book> uniq;
    for (auto& b : books) {
        bool dup = false;
        for (auto& u : uniq) if (u.id == b.id) { dup = true; break; }
        if (!dup) uniq.push_back(b);
    }
    if (uniq.empty()) { logLine(L"找不到可讀書籍"); return false; }

    // 隨機（避免立刻重複）
    int idx = (int)(rand() % uniq.size());
    if (uniq.size() > 1 && uniq[idx].id == avoidId) idx = (idx + 1) % (int)uniq.size();
    logLine(L"隨機選書 → " + uniq[idx].title + L" (#" + fmt(L"%lld", uniq[idx].id) + L")");

    if (startBook(uniq[idx], ppc)) return true;
    // 失敗換一本
    for (auto& alt : uniq) {
        if (alt.id == uniq[idx].id) continue;
        logLine(L"改選 → " + alt.title);
        if (startBook(alt, ppc)) return true;
    }
    return false;
}

static void workerLoop(int targetX10, int hbSec, int turnSec) {
    logLine(L"=== 開始自動閱讀 · 目標 " + fmt(L"%.1f", targetX10 / 10.0) + L" 小時 ===");

    ApiRes me = api(L"GET", L"/v1/client/users/detail", "", true);
    if (!me.ok) {
        logLine(L"token 無效：" + fmt(L"%d", me.code) + L" " + me.msg);
        if (me.code == 10117 || me.code == 10119) {
            if (refreshToken()) me = api(L"GET", L"/v1/client/users/detail", "", true);
        }
        if (!me.ok) {
            logLine(L"無法取得帳號資料，請重新複製 token");
            setSnap([](Snapshot& s) { s.running = false; });
            requestRepaint();
            return;
        }
    }
    loadUserDetail(me.json.find("data"));
    loadQuota(true);
    requestRepaint();
    int ppc = 20;
    long long avoid = 0;
    if (!fetchAndStartRandomBook(ppc, avoid)) {
        logLine(L"開書失敗，停止");
        setSnap([](Snapshot& s) { s.running = false; });
        requestRepaint();
        return;
    }

    auto lastHb = std::chrono::steady_clock::now() - std::chrono::seconds(hbSec);
    auto lastTurn = std::chrono::steady_clock::now();
    auto lastRefresh = std::chrono::steady_clock::now();
    int authFail = 0;

    while (!g_stop.load()) {
        auto now = std::chrono::steady_clock::now();

        // 定期續期（伺服器端 session 約 4 分鐘就失效，靠 refresh 續命）
        if (std::chrono::duration_cast<std::chrono::seconds>(now - lastRefresh).count() >= 150) {
            if (refreshToken()) { lastRefresh = now; authFail = 0; }
            else { lastRefresh = now + std::chrono::seconds(-90); }
        }

        // 心跳
        if (std::chrono::duration_cast<std::chrono::seconds>(now - lastHb).count() >= hbSec) {
            lastHb = now;
            std::wstring sid;
            {
                std::lock_guard<std::mutex> lk(g_mtx);
                sid = g_snap.session;
            }
            if (sid.empty()) {
                logLine(L"session 為空，嘗試重開");
                if (!restartSession()) { logLine(L"重開失敗，停止"); break; }
                continue;
            }
            char buf[256];
            snprintf(buf, sizeof(buf), "{\"sessionId\":\"%s\",\"time\":%d}", narrow(sid).c_str(), hbSec);
            ApiRes a = api(L"PATCH", L"/v1/client/reading-time", buf, true);
            if (a.ok) {
                authFail = 0;
                setSnap([&](Snapshot& s) {
                    s.hbCount++;
                    s.sentSec += hbSec;
                    s.errorTime = 0;
                    s.lastHb = nowTime();
                });
                if ((g_snap.hbCount <= 3) || (g_snap.hbCount % 20 == 0))
                    logLine(L"心跳 #" + fmt(L"%d", g_snap.hbCount) + L" ok · time=" + fmt(L"%d", hbSec) + L"s");
                loadQuota();
                requestRepaint();
            } else {
                bool isAuth = (a.code == 10117 || a.code == 10119);
                bool isSession = (a.msg.find(L"session") != std::wstring::npos);
                logLine(L"心跳 " + fmt(L"%d", a.code) + L"：" + a.msg);
                if (isAuth && isSession) {
                    if (restartSession()) continue;
                }
                if (isAuth) {
                    bool ok = refreshToken();
                    if (!ok) {
                        if (++authFail >= 3) { logLine(L"token 續期失敗 3 次，停止"); break; }
                    }
                } else {
                    setSnap([&](Snapshot& s) { s.errorTime++; });
                    if (g_snap.errorTime > 3) { logLine(L"連續心跳失敗，停止"); break; }
                }
                requestRepaint();
            }
        }

        // 翻頁
        if (std::chrono::duration_cast<std::chrono::seconds>(now - lastTurn).count() >= turnSec) {
            lastTurn = now;
            int page = 0, total = 0;
            {
                std::lock_guard<std::mutex> lk(g_mtx);
                page = g_snap.page; total = g_snap.totalPages;
            }
            ++page;
            if (page >= total) {
                logLine(L"本書讀完，換下一本");
                endSession();
                long long prevId = 0;
                { std::lock_guard<std::mutex> lk(g_mtx); prevId = g_snap.bookId; }
                if (!fetchAndStartRandomBook(ppc, prevId)) { logLine(L"找不到下一本，停止"); break; }
            } else {
                setSnap([&](Snapshot& s) { s.page = page; s.turnCount++; });
                sendReadPos();
                requestRepaint();
            }
        }

        // 目標達成？
        long long doneSec = 0;
        {
            std::lock_guard<std::mutex> lk(g_mtx);
            doneSec = g_snap.sentSec;
        }
        if (targetX10 > 0 && doneSec >= (long long)targetX10 * 360) {
            logLine(L"已達成目標時長");
            break;
        }

        Sleep(200);
    }

    logLine(L"=== 停止，收尾 ===");
    endSession();
    setSnap([](Snapshot& s) { s.running = false; });
    requestRepaint();
}

// ---------------- 介面 ----------------
static HWND g_editToken = nullptr, g_log = nullptr;
static HWND g_target = nullptr, g_hb = nullptr, g_turn = nullptr;
static HWND g_btnStart = nullptr, g_btnStop = nullptr, g_btnImport = nullptr;
static HWND g_tokenState = nullptr;
static HWND g_comboOAuth = nullptr, g_btnOAuth = nullptr, g_authInfo = nullptr;
static HWND g_btnRefresh = nullptr;
static std::wstring g_lastClipboardHash;


static HWND mkStatic(const wchar_t* text, int x, int y, int w, int h, HFONT f, int id) {
    HWND hw = CreateWindowExW(0, L"STATIC", text, WS_CHILD | WS_VISIBLE | SS_LEFT,
                              x, y, w, h, g_hWnd, (HMENU)(INT_PTR)id, g_hInst, nullptr);
    SendMessageW(hw, WM_SETFONT, (WPARAM)(f ? f : g_fontUI), TRUE);
    return hw;
}

static HWND mkEdit(const wchar_t* text, int x, int y, int w, int h, DWORD extra, int id, bool multiline) {
    DWORD style = WS_CHILD | WS_VISIBLE | WS_BORDER | (multiline ? (ES_MULTILINE | ES_AUTOVSCROLL) : ES_AUTOHSCROLL);
    if (id == ID_LOG) style |= WS_VSCROLL;
    HWND hw = CreateWindowExW(0, L"EDIT", text, style,
                              x, y, w, h, g_hWnd, (HMENU)(INT_PTR)id, g_hInst, nullptr);
    SendMessageW(hw, WM_SETFONT, (WPARAM)(multiline ? g_fontMono : g_fontUI), TRUE);
    return hw;
}

static HWND mkButton(const wchar_t* text, int x, int y, int w, int h, int id, bool primary) {
    HWND hw = CreateWindowExW(0, L"BUTTON", text, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                              x, y, w, h, g_hWnd, (HMENU)(INT_PTR)id, g_hInst, nullptr);
    SetWindowLongPtrW(hw, GWLP_USERDATA, primary ? 1 : 0);
    return hw;
}

static void drawCard(Gdiplus::Graphics& g, int x, int y, int w, int h) {
    using namespace Gdiplus;
    GraphicsPath path;
    int r = 12;
    path.AddArc(x, y, r * 2, r * 2, 180, 90);
    path.AddArc(x + w - r * 2, y, r * 2, r * 2, 270, 90);
    path.AddArc(x + w - r * 2, y + h - r * 2, r * 2, r * 2, 0, 90);
    path.AddArc(x, y + h - r * 2, r * 2, r * 2, 90, 90);
    path.CloseFigure();
    SolidBrush br(Color(255, 255, 255, 255));
    g.FillPath(&br, &path);
    Pen pen(Color(255, 0xE3, 0xEA, 0xE7), 1.0f);
    g.DrawPath(&pen, &path);
}

static void drawBar(Gdiplus::Graphics& g, int x, int y, int w, int h, double pct, bool alt) {
    using namespace Gdiplus;
    GraphicsPath bg;
    int r = h / 2;
    bg.AddArc(x, y, r * 2, r * 2, 180, 90);
    bg.AddArc(x + w - r * 2, y, r * 2, r * 2, 270, 90);
    bg.AddArc(x + w - r * 2, y + h - r * 2, r * 2, r * 2, 0, 90);
    bg.AddArc(x, y + h - r * 2, r * 2, r * 2, 90, 90);
    bg.CloseFigure();
    SolidBrush b1(Color(255, 0xEC, 0xF3, 0xF0));
    g.FillPath(&b1, &bg);
    if (pct > 0) {
        int fw = (int)(w * (pct > 1 ? 1 : pct));
        if (fw < r * 2) fw = r * 2;
        GraphicsPath fg;
        fg.AddArc(x, y, r * 2, r * 2, 180, 90);
        fg.AddArc(x + fw - r * 2, y, r * 2, r * 2, 270, 90);
        fg.AddArc(x + fw - r * 2, y + h - r * 2, r * 2, r * 2, 0, 90);
        fg.AddArc(x, y + h - r * 2, r * 2, r * 2, 90, 90);
        fg.CloseFigure();
        SolidBrush b2(alt ? Color(255, 0xE8, 0xA3, 0x3D) : Color(255, 0x3D, 0xCA, 0xB1));
        g.FillPath(&b2, &fg);
    }
}

static void text(HDC dc, int x, int y, const wchar_t* s) {
    TextOutW(dc, x, y, s, (int)wcslen(s));
}

static void paintWindow(HWND hwnd) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);
    RECT rc;
    GetClientRect(hwnd, &rc);
    int W = rc.right, H = rc.bottom;

    HDC mem = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, W, H);
    HBITMAP old = (HBITMAP)SelectObject(mem, bmp);

    RECT full{ 0, 0, W, H };
    HBRUSH bg = CreateSolidBrush(C_BG);
    FillRect(mem, &full, bg);
    DeleteObject(bg);

    {
        Gdiplus::Graphics g(mem);
        g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
        // 卡片
        drawCard(g, 20, 78, 580, 140);    // token
        drawCard(g, 20, 230, 580, 124);   // params
        drawCard(g, 20, 366, 580, 150);   // book
        drawCard(g, 616, 78, 364, 200);   // progress
        drawCard(g, 616, 290, 364, 226);  // account / reading data
        drawCard(g, 20, 528, 960, 190);   // log

        Snapshot s;
        {
            std::lock_guard<std::mutex> lk(g_mtx);
            s = g_snap;
        }

        double doneH = (double)s.sentSec / 3600.0;
        double targetH = s.targetX10 / 10.0;
        double pct = targetH > 0 ? doneH / targetH : 0;
        if (pct > 1) pct = 1;
        drawBar(g, 636, 196, 324, 10, pct, false);

        // 個人配額用量條（橘色，跟目標進度條區分）
        double qPct = s.quotaTotal > 0 ? (double)s.quotaUsed / (double)s.quotaTotal : 0;
        if (qPct > 1) qPct = 1;
        drawBar(g, 636, 234, 324, 8, qPct, true);
    }

    // 應用圖標
    if (!g_hIconBig) {
        g_hIconBig = (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(1), IMAGE_ICON, 40, 40, LR_DEFAULTCOLOR);
    }
    if (g_hIconBig) DrawIconEx(mem, 20, 16, g_hIconBig, 40, 40, 0, nullptr, DI_NORMAL);

    // 標題
    SetBkMode(mem, TRANSPARENT);
    SelectObject(mem, g_fontUIB);
    SetTextColor(mem, C_TXT);
    text(mem, 72, 18, L"金閱閱校園版 · 自動閱讀器");
    SelectObject(mem, g_fontSmall);
    SetTextColor(mem, C_DIM);
    text(mem, 72, 44, L"SJRC 2.9.0 · api.sjrc.club · 心跳與翻頁在背景執行 · 關閉視窗即停止服務");

    // 卡片標題
    SelectObject(mem, g_fontUIB);
    SetTextColor(mem, C_ACCD);
    text(mem, 36, 90, L"01 · TOKEN");
    text(mem, 36, 242, L"02 · 參數");
    text(mem, 36, 378, L"03 · 目前書本");
    text(mem, 636, 90, L"04 · 進度");
    text(mem, 636, 302, L"05 · 帳號與閱讀數據");
    text(mem, 36, 540, L"06 · 即時狀態與封包日誌");

    // 進度大字
    {
        Snapshot s;
        {
            std::lock_guard<std::mutex> lk(g_mtx);
            s = g_snap;
        }
        // 大字改成 H:MM:SS —— 之前用「小時」兩位小數，心跳 1 秒才 +0.0003，
        // 看起來像完全沒在動，會被誤認成功能沒寫。
        long long sec = s.sentSec;
        SelectObject(mem, g_fontBig);
        SetTextColor(mem, C_ACCD);
        std::wstring big = fmt(L"%lld:%02lld:%02lld", sec / 3600, (sec % 3600) / 60, sec % 60);
        TextOutW(mem, 636, 112, big.c_str(), (int)big.size());
        SIZE sz{};
        GetTextExtentPoint32W(mem, big.c_str(), (int)big.size(), &sz);
        SelectObject(mem, g_fontUI);
        SetTextColor(mem, C_DIM);
        std::wstring tail = fmt(L" / %.1f 小時", s.targetX10 / 10.0);
        TextOutW(mem, 636 + sz.cx + 6, 130, tail.c_str(), (int)tail.size());

        double remain = s.targetX10 / 10.0 - (double)s.sentSec / 3600.0;
        if (remain < 0) remain = 0;
        long long rs = (long long)(remain * 3600.0 + 0.5);
        std::wstring rt = fmt(L"剩餘 %lld:%02lld:%02lld", rs / 3600, (rs % 3600) / 60, rs % 60);
        TextOutW(mem, 636, 152, rt.c_str(), (int)rt.size());

        SelectObject(mem, g_fontSmall);
        SetTextColor(mem, C_DIM);
        std::wstring rep = fmt(L"已上報（伺服器）%lld 秒 · 心跳 %d 次", sec, s.hbCount);
        TextOutW(mem, 636, 176, rep.c_str(), (int)rep.size());

        SelectObject(mem, g_fontUI);
        SetTextColor(mem, C_ACCD);
        long long qLeft = s.quotaTotal - s.quotaUsed;
        if (qLeft < 0) qLeft = 0;
        std::wstring q = fmt(L"配額 %.2f / %.2f 小時 · 剩 %.2f", s.quotaUsed / 3600.0,
                             s.quotaTotal / 3600.0, qLeft / 3600.0);
        TextOutW(mem, 636, 214, q.c_str(), (int)q.size());

        SelectObject(mem, g_fontSmall);
        SetTextColor(mem, C_DIM);
        std::wstring st = s.running ? L"狀態：執行中" : L"狀態：待機";
        TextOutW(mem, 636, 250, st.c_str(), (int)st.size());
    }

    // 狀態列
    SelectObject(mem, g_fontSmall);
    SetTextColor(mem, C_DIM);
    text(mem, 22, 736, L"單一 exe · 無外部依賴 · 設定自動儲存");

    BitBlt(hdc, 0, 0, W, H, mem, 0, 0, SRCCOPY);
    SelectObject(mem, old);
    DeleteObject(bmp);
    DeleteDC(mem);
    EndPaint(hwnd, &ps);
}

static void refreshControls() {
    Snapshot s;
    {
        std::lock_guard<std::mutex> lk(g_mtx);
        s = g_snap;
    }
    auto setIfChanged = [&](int id, const std::wstring& text) {
        HWND h = GetDlgItem(g_hWnd, id);
        if (!h) return;
        int n = GetWindowTextLengthW(h);
        if (n < 0) n = 0;
        std::wstring cur;
        if (n > 0) {
            cur.resize(n + 1, L'\0');
            GetWindowTextW(h, &cur[0], n + 1);
            cur.resize(n);
        }
        if (cur != text) SetWindowTextW(h, text.c_str());
    };
    setIfChanged(ID_ST_BOOK,   s.book.empty() ? L"—" : s.book);
    setIfChanged(ID_ST_AUTHOR, s.author.empty() ? L"—" : s.author);
    setIfChanged(ID_ST_BOOKID, s.bookId ? fmt(L"%lld", s.bookId) : L"—");
    setIfChanged(ID_ST_CHAPTERS, s.chapters ? fmt(L"%d 章 · 共 %d 頁", s.chapters, s.totalPages) : L"—");
    setIfChanged(ID_ST_PAGE,   s.totalPages ? fmt(L"%d / %d 頁", s.page + 1, s.totalPages) : L"—");

    // 帳號卡
    setIfChanged(ID_ST_NAME,   s.name.empty() ? L"—" : s.name);
    setIfChanged(ID_ST_EMAIL,  s.email.empty() ? L"—" : s.email);
    setIfChanged(ID_ST_SCHOOL, s.school.empty() ? L"—" : s.school);
    {
        std::wstring ident = s.acctType;
        if (!s.serialNo.empty()) {
            if (!ident.empty()) ident += L" · ";
            ident += L"學號 " + s.serialNo;
        }
        setIfChanged(ID_ST_IDENT, ident.empty() ? L"—" : ident);
    }
    setIfChanged(ID_ST_TOTALUSED, s.quotaOk ? fmtHours(s.totalUsedQuota) : L"—");
    setIfChanged(ID_ST_READCNT, s.quotaOk
                    ? fmt(L"%d 本 · %d 篇心得", s.readCount, s.reflectionCount) : L"—");
    setIfChanged(ID_ST_SCHOOLQ, (s.quotaOk && s.schoolTotalQuota > 0)
                    ? fmt(L"%.2f / %.2f 小時", s.schoolUsedQuota / 3600.0, s.schoolTotalQuota / 3600.0)
                    : L"—");

    // 即時狀態一行（原本分散在「狀態」卡的六個欄位）
    {
        std::wstring run = fmt(L"心跳 %d · 翻頁 %d · 書本 %d · 錯誤 %d/3",
                               s.hbCount, s.turnCount, s.bookCount, s.errorTime);
        run += L" · Session ";
        run += s.session.empty() ? std::wstring(L"—") : s.session.substr(0, 12);
        run += L" · 最後心跳 ";
        run += s.lastHb.empty() ? std::wstring(L"—") : s.lastHb;
        if (s.sharing) run += L" · 分享中";
        setIfChanged(ID_ST_RUNLINE, run);
    }

    // 登入狀態列：抓到帳號資料就顯示已登入（不要一直卡在「尚未登入」）
    if (s.acctOk && g_tokenState) {
        wchar_t cur[160] = {0};
        GetWindowTextW(g_tokenState, cur, 160);
        std::wstring c = cur;
        if (c == L"尚未登入" || c.rfind(L"已登入：", 0) == 0 || c == L"已登入") {
            std::wstring t = s.email.empty() ? std::wstring(L"已登入") : (L"已登入：" + s.email);
            if (c != t) SetWindowTextW(g_tokenState, t.c_str());
        }
    }

    EnableWindow(g_btnStart, !s.running);
    EnableWindow(g_btnStop,  s.running);
}

static void drainLog() {
    std::deque<std::wstring> lines;
    {
        std::lock_guard<std::mutex> lk(g_mtx);
        lines.swap(g_pendingLog);
    }
    if (lines.empty()) return;
    std::wstring add;
    for (auto& l : lines) { add += l; add += L"\r\n"; }
    int len = GetWindowTextLengthW(g_log);
    SendMessageW(g_log, EM_SETSEL, len, len);
    SendMessageW(g_log, EM_REPLACESEL, FALSE, (LPARAM)add.c_str());
    SendMessageW(g_log, EM_SCROLLCARET, 0, 0);
}

struct ImportResult {
    bool ok = false;
    int code = -1;
    std::wstring msg;
    std::wstring email;
};

static void tryImportText(const std::wstring& txt) {
    std::string raw = narrow(txt);
    std::string at, rt;
    size_t b = raw.find('{');
    if (b != std::string::npos) {
        JVal v;
        if (parseJson(raw.substr(b), v)) {
            const JVal* a = v.find("accessToken");
            if (!a) a = v.find("token");
            const JVal* r = v.find("refreshToken");
            if (!r) r = v.find("refresh");
            if (a && a->type == JVal::STR) at = a->str;
            if (r && r->type == JVal::STR) rt = r->str;
        }
    }
    if (at.empty()) at = raw;

    {
        std::lock_guard<std::mutex> lk(g_mtx);
        g_token = at;
        if (!rt.empty()) g_refresh = rt;
    }
    saveSetting(L"token", widen(at));
    if (!rt.empty()) saveSetting(L"refresh", widen(rt));

    SetWindowTextW(g_tokenState, L"驗證中…");
    HWND hw = g_hWnd;
    std::thread([hw]() {
        ImportResult* r = new ImportResult();
        ApiRes me = api(L"GET", L"/v1/client/users/detail", "", true);
        r->ok = me.ok;
        r->code = me.code;
        r->msg = me.msg;
        if (me.ok) {
            const JVal* d = me.json.find("data");
            if (d) { const JVal* e = d->find("email"); if (e) r->email = e->wstr(); }
            loadUserDetail(d);
            loadQuota(true);
        }
        PostMessageW(hw, WM_APP_IMPORT, 0, (LPARAM)r);
    }).detach();
}

static void doImport() {
    HWND hEdit = GetDlgItem(g_hWnd, ID_TOKEN);
    if (!hEdit) hEdit = g_editToken;
    int len = hEdit ? GetWindowTextLengthW(hEdit) : 0;
    if (len <= 0) { SetWindowTextW(g_tokenState, L"Please paste token first"); return; }
    std::wstring txt(len + 1, L'\0');
    GetWindowTextW(hEdit, &txt[0], len + 1);
    txt.resize(len);
    tryImportText(txt);
}

// ---------- Google credential → SJRC token ----------
// 正確流程（對齊官方 Web 版）：
//   POST /v1/public/accounts/users/google/pre-login  {providerId, type:"web"}
//     ├─ data.accessToken → 直接登入
//     └─ data.ticket      → 需要電郵驗證碼 → POST .../google/login {ticket, verificationCode}
struct LoginResult {
    bool ok = false;
    bool needCode = false;
    std::wstring ticket;
    std::wstring email;
    int code = -1;
    std::wstring msg;
};

static void requestShutdownSoon(DWORD delayMs);

static void adoptLoginData(const JVal& d, LoginResult& r) {
    const JVal* at = d.find("accessToken");
    if (!at) at = d.find("token");
    const JVal* rt = d.find("refreshToken");
    if (!rt) rt = d.find("refresh");
    if (!at || at->type != JVal::STR || at->str.empty()) return;

    std::string tok = at->str;
    std::string ref = (rt && rt->type == JVal::STR) ? rt->str : std::string();
    {
        std::lock_guard<std::mutex> lk(g_mtx);
        g_token = tok;
        g_refresh = ref;   // 新登入沒給 refreshToken 就清掉舊的，避免拿到殘留 token 去續期
    }
    saveSetting(L"token", widen(tok));
    saveSetting(L"refresh", widen(ref));

    ApiRes me = api(L"GET", L"/v1/client/users/detail", "", true);
    if (me.ok) {
        const JVal* dd = me.json.find("data");
        if (dd) {
            const JVal* e = dd->find("email");
            if (e && e->type == JVal::STR) r.email = e->wstr();
            loadUserDetail(dd);      // 順便把名稱／學校／學號填進介面
        }
        loadQuota(true);
    }
    r.ok = true;
    requestShutdownSoon(1500);   // 登入完成 → 本機服務自動關閉
    logLine(L"登入成功：" + (r.email.empty() ? std::wstring(L"(unknown)") : r.email) +
            (ref.empty() ? L" · 無 refreshToken（無法自動續期）" : L" · 已取得 refreshToken"));
    setSnap([&](Snapshot& s) { s.user = r.email; });
}

static LoginResult exchangeGoogleCredential(const std::string& cred) {
    LoginResult r;
    if (cred.empty()) { r.msg = L"credential 為空"; return r; }
    logLine(L"→ POST /v1/public/accounts/users/google/pre-login");
    std::string body = "{\"providerId\":\"" + cred + "\",\"type\":\"web\"}";
    ApiRes a = api(L"POST", L"/v1/public/accounts/users/google/pre-login", body, false, L"PUBLIC");
    r.code = a.code;
    r.msg = a.msg;
    logLine(L"← " + fmt(L"%d", a.code) + L" " + a.msg);
    if (!a.ok) {
        if (a.code == 20045) r.msg = L"此 Google 帳號未在 SJRC 註冊";
        else if (a.code == 20002) r.msg = L"id_token 驗證失敗（aud 不符 / 用了非 Web 類型 client）";
        return r;
    }
    const JVal* d = a.json.find("data");
    if (!d) { r.msg = L"回應缺少 data"; return r; }

    const JVal* at = d->find("accessToken");
    if (at && at->type == JVal::STR && !at->str.empty()) {
        adoptLoginData(*d, r);
        return r;
    }
    const JVal* tk = d->find("ticket");
    if (!tk) tk = d->find("code");
    if (!tk) tk = d->find("token");
    if (tk && tk->type == JVal::STR && !tk->str.empty()) {
        r.needCode = true;
        r.ticket = tk->wstr();
        r.msg = L"需要電郵驗證碼";
        logLine(L"pre-login 回傳 ticket，等待電郵驗證碼");
    } else {
        r.msg = L"pre-login 回應既無 accessToken 也無 ticket";
    }
    return r;
}

static LoginResult submitVerificationCode(const std::wstring& ticket, const std::wstring& code) {
    LoginResult r;
    if (ticket.empty()) { r.msg = L"缺少 ticket"; return r; }
    logLine(L"→ POST /v1/public/accounts/users/google/login");
    std::string body = "{\"ticket\":\"" + narrow(ticket) + "\",\"verificationCode\":\"" + narrow(code) + "\"}";
    ApiRes a = api(L"POST", L"/v1/public/accounts/users/google/login", body, false, L"PUBLIC");
    r.code = a.code;
    r.msg = a.msg;
    logLine(L"← " + fmt(L"%d", a.code) + L" " + a.msg);
    if (a.ok) {
        const JVal* d = a.json.find("data");
        if (d) adoptLoginData(*d, r);
    }
    return r;
}

static void handleGoogleCredential(const std::wstring& credW) {
    LoginResult r = exchangeGoogleCredential(narrow(credW));
    if (r.ok) {
        SetWindowTextW(g_tokenState, fmt(L"登入成功：%s", r.email.empty() ? L"(unknown)" : r.email.c_str()).c_str());
    } else if (r.needCode) {
        SetWindowTextW(g_tokenState, L"需要電郵驗證碼，請在登入頁輸入");
    } else {
        SetWindowTextW(g_tokenState, fmt(L"Google 換 token 失敗：%d %s", r.code, r.msg.c_str()).c_str());
    }
    requestRepaint();
}

static void startRun() {
    if (g_worker.joinable()) return;
    wchar_t buf[64];
    int targetX10 = 20, hb = 3, turn = 20;
    GetWindowTextW(g_target, buf, 64); targetX10 = (int)(_wtof(buf) * 10 + 0.5);
    GetWindowTextW(g_hb, buf, 64); hb = _wtoi(buf);
    GetWindowTextW(g_turn, buf, 64); turn = _wtoi(buf);
    if (hb < 1) hb = 1;
    if (turn < 2) turn = 2;
    if (targetX10 <= 0) { logLine(L"目標時長需大於 0"); return; }

    bool hasToken = false;
    {
        std::lock_guard<std::mutex> lk(g_mtx);
        hasToken = !g_token.empty();
    }
    if (!hasToken) { logLine(L"尚未匯入 token，請先貼上並按「導入並驗證」"); return; }
    {
        std::lock_guard<std::mutex> lk(g_mtx);
        g_snap.running = true;
        g_snap.targetX10 = targetX10;
        g_snap.sentSec = 0;
        g_snap.hbCount = 0;
        g_snap.turnCount = 0;
        g_snap.bookCount = 0;
        g_snap.errorTime = 0;
    }
    saveSetting(L"target", fmt(L"%.1f", targetX10 / 10.0));
    saveSetting(L"hb", fmt(L"%d", hb));
    saveSetting(L"turn", fmt(L"%d", turn));

    g_stop.store(false);
    g_worker = std::thread(workerLoop, targetX10, hb, turn);
    refreshControls();
}

static void stopRun() {
    if (!g_worker.joinable()) return;
    g_stop.store(true);
    if (g_worker.joinable()) g_worker.join();
    refreshControls();
}

// ---------- 本機登入服務（按需啟動 · 用完即關） ----------
// 專用埠 47821：只有按下按鈕才啟動，登入完成 / 閒置 2 分鐘 / 關閉程式就關掉。
// Google 一鍵登入受 SJRC 官方 client 白名單限制，只認 http://localhost:8080，
// 所以只有「按 Google 登入」時才臨時再開一個 8080 監聽，登入完成立刻關閉。
static const int   AUTH_PORT_MAIN     = 47821;
static const int   AUTH_PORT_GOOGLE   = 8080;
static const DWORD AUTH_IDLE_MS_MAIN  = 120000;   // 主頁閒置 2 分鐘
static const DWORD AUTH_IDLE_MS_GGL   = 180000;   // Google 埠閒置 3 分鐘

static unsigned long long nowMs() { return GetTickCount64(); }

struct Listener {
    std::atomic<int> state{0};          // 0 閒置 · 1 啟動中 · 2 運行中 · -1 失敗
    std::atomic<int> port{0};
    std::atomic<int> family{AF_INET};
    std::atomic<unsigned long long> lastHit{0};
    SOCKET sock = INVALID_SOCKET;
};
static Listener g_srvMain, g_srvGoogle;
static std::atomic<unsigned long long> g_shutdownAt{0};   // 0 = 不關
static std::wstring g_pendingTicket;            // pre-login 回傳的 ticket，等驗證碼
static std::atomic<bool> g_loginUiPending{false};
static std::wstring g_loginUiText;              // 受 g_mtx 保護

static std::string jesc(const std::string& s) {
    std::string o;
    o.reserve(s.size() + 8);
    for (char c : s) {
        switch (c) {
        case '"':  o += "\\\""; break;
        case '\\': o += "\\\\"; break;
        case '\n': o += "\\n"; break;
        case '\r': o += "\\r"; break;
        case '\t': o += "\\t"; break;
        default:
            if ((unsigned char)c < 0x20) {
                char b[8];
                snprintf(b, sizeof(b), "\\u%04x", (unsigned char)c);
                o += b;
            } else o += c;
        }
    }
    return o;
}

static std::string jsonStr(const std::string& body, const char* key) {
    std::string k = std::string("\"") + key + "\":\"";
    size_t p = body.find(k);
    if (p == std::string::npos) return "";
    p += k.size();
    std::string out;
    while (p < body.size()) {
        char c = body[p];
        if (c == '\\' && p + 1 < body.size()) { out += body[p + 1]; p += 2; continue; }
        if (c == '"') break;
        out += c;
        ++p;
    }
    return out;
}

static size_t contentLengthOf(const std::string& head) {
    std::string h;
    h.reserve(head.size());
    for (char c : head) h += (char)tolower((unsigned char)c);
    size_t p = h.find("content-length:");
    if (p == std::string::npos) return 0;
    p += 15;
    while (p < h.size() && (h[p] == ' ' || h[p] == '\t')) ++p;
    size_t v = 0;
    while (p < h.size() && h[p] >= '0' && h[p] <= '9') { v = v * 10 + (size_t)(h[p] - '0'); ++p; }
    return v;
}

static std::string requestPath(const std::string& head) {
    size_t a = head.find(' ');
    if (a == std::string::npos) return "/";
    size_t b = head.find(' ', a + 1);
    std::string p = head.substr(a + 1, (b == std::string::npos) ? std::string::npos : b - a - 1);
    size_t q = p.find('?');
    if (q != std::string::npos) p = p.substr(0, q);
    if (p.empty()) p = "/";
    return p;
}

// 探測某個回環埠是否已經有程式在監聽（避免搶別人的埠）
// 非阻塞 connect + SO_ERROR 判定：連上才算「有人佔用」，被拒絕則視為空閒。
static bool probeOnce(int family, int port) {
    SOCKET c = socket(family, SOCK_STREAM, IPPROTO_TCP);
    if (c == INVALID_SOCKET) return false;
    u_long nb = 1;
    ioctlsocket(c, FIONBIO, &nb);
    int rc;
    if (family == AF_INET6) {
        sockaddr_in6 a{};
        a.sin6_family = AF_INET6;
        a.sin6_addr = in6addr_loopback;
        a.sin6_port = htons((u_short)port);
        rc = connect(c, (sockaddr*)&a, sizeof(a));
    } else {
        sockaddr_in a{};
        a.sin_family = AF_INET;
        a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        a.sin_port = htons((u_short)port);
        rc = connect(c, (sockaddr*)&a, sizeof(a));
    }
    bool used = false;
    if (rc == 0) {
        used = true;
    } else if (WSAGetLastError() == WSAEWOULDBLOCK) {
        fd_set wf, ef;
        FD_ZERO(&wf); FD_ZERO(&ef);
        FD_SET(c, &wf); FD_SET(c, &ef);
        timeval tv{0, 250000};
        int n = select(0, nullptr, &wf, &ef, &tv);
        if (n > 0) {
            int err = 0, len = sizeof(err);
            if (getsockopt(c, SOL_SOCKET, SO_ERROR, (char*)&err, &len) == 0 && err == 0)
                used = (FD_ISSET(c, &wf) != 0);
        }
    }
    closesocket(c);
    return used;
}

static bool portInUse(int family, int port) {
    // 連續兩次都判定佔用才算（單次結果在剛關閉連線時可能抖動）
    if (!probeOnce(family, port)) return false;
    Sleep(80);
    return probeOnce(family, port);
}

// 綁定回環位址。用 SO_REUSEADDR，服務關閉後才能立刻重新啟動
// （獨佔式綁定會卡在 TIME_WAIT，兩分鐘內無法重開）；
// 先用 portInUse 探測，避免搶佔別的程式正在用的埠。
static SOCKET listenLoopback(int family, int port) {
    if (port != 0 && portInUse(family, port)) return INVALID_SOCKET;
    SOCKET s = socket(family, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) return INVALID_SOCKET;
    int yes = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes));
    int rc;
    if (family == AF_INET6) {
        sockaddr_in6 a{};
        a.sin6_family = AF_INET6;
        a.sin6_addr = in6addr_loopback;
        a.sin6_port = htons((u_short)port);
        rc = bind(s, (sockaddr*)&a, sizeof(a));
    } else {
        sockaddr_in a{};
        a.sin_family = AF_INET;
        a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        a.sin_port = htons((u_short)port);
        rc = bind(s, (sockaddr*)&a, sizeof(a));
    }
    if (rc != 0 || listen(s, 8) != 0) { closesocket(s); return INVALID_SOCKET; }
    return s;
}

static int socketPort(SOCKET s, int family) {
    if (family == AF_INET6) {
        sockaddr_in6 a{};
        int l = sizeof(a);
        if (getsockname(s, (sockaddr*)&a, &l) == 0) return (int)ntohs(a.sin6_port);
    } else {
        sockaddr_in a{};
        int l = sizeof(a);
        if (getsockname(s, (sockaddr*)&a, &l) == 0) return (int)ntohs(a.sin_port);
    }
    return 0;
}

static const char* authPageHtml() {
    return R"HTML(<!DOCTYPE html>
<html lang="zh-Hant"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>SJRC 登入助手</title>
<script src="https://accounts.google.com/gsi/client" async defer></script>
<style>
*{box-sizing:border-box}
body{margin:0;padding:28px 16px;background:#F5F7F6;color:#2B3A36;
  font-family:"Segoe UI",system-ui,-apple-system,"Microsoft JhengHei",sans-serif;line-height:1.6}
.wrap{max-width:620px;margin:0 auto}
h1{font-size:22px;margin:0 0 6px}
h2{font-size:15px;margin:0 0 12px;color:#1F8574}
.sub{color:#5F716C;font-size:13px;margin-bottom:18px}
.card{background:#fff;border:1px solid #E3EAE7;border-radius:12px;padding:18px 20px;margin-bottom:14px}
.hint{font-size:13px;color:#5F716C;margin-top:10px}
.hint.ok{color:#1F8574}.hint.er{color:#E0605E}.hint.wn{color:#C4862A}
.box{font-size:13px;padding:10px 14px;border-radius:10px;background:#ECF3F0;border:1px solid #E3EAE7;margin-bottom:14px}
code{background:#EEF3F1;padding:1px 5px;border-radius:4px;font-size:12px}
textarea{width:100%;min-height:86px;padding:10px;border:1px solid #DCE6E2;border-radius:8px;
  font-family:Consolas,monospace;font-size:12px;resize:vertical;outline:none}
textarea:focus{border-color:#3DCAB1}
input{width:160px;padding:9px 10px;border:1px solid #DCE6E2;border-radius:8px;font-size:15px;
  letter-spacing:3px;outline:none}
input:focus{border-color:#3DCAB1}
button{padding:10px 18px;font-size:14px;border:none;border-radius:8px;background:#3DCAB1;
  color:#fff;cursor:pointer;font-weight:600}
button:hover{background:#34B49E}
button.ghost{background:#fff;color:#1F8574;border:1px solid #BFE6DD}
button.ghost:hover{background:#F0FAF7}
.row{display:flex;gap:10px;margin-top:12px}
.foot{font-size:12px;color:#7C8C87;margin-top:18px;padding-top:14px;border-top:1px solid #E3EAE7}
a{color:#1F8574}
#gbtn{margin-bottom:4px}
#okBox{border-color:#BFE6DD;background:#F2FBF8}
</style></head>
<body><div class="wrap">
<h1>SJRC 登入助手</h1>
<div class="sub">本頁由「SJRC 自動閱讀器」臨時提供，只綁定 127.0.0.1 / ::1；
  登入完成或閒置 2 分鐘後，程式會自動關閉這個服務。</div>

<div id="originBox" class="box">目前 origin：<code id="origin"></code></div>

<div class="card">
  <h2>① Google 登入（自動換 token）</h2>

  <div id="gArea">
    <div id="gbtn"></div>
    <div id="gstate" class="hint">正在載入 Google 登入…</div>
    <div id="codeBox" style="display:none;margin-top:14px">
      <p class="hint" style="margin-top:0">已寄出驗證碼到你的 Google 郵箱，請輸入驗證碼：</p>
      <input id="code" maxlength="8" placeholder="123456" inputmode="numeric">
      <button onclick="submitCode()" style="margin-left:8px">送出</button>
      <div id="cstate" class="hint"></div>
    </div>
  </div>

  <div id="upArea" style="display:none">
    <p class="hint" style="margin-top:0">Google 只認 <code>http://localhost:8080</code> 這個來源（SJRC 官方 client 的白名單）。
      按下面按鈕會<b>臨時</b>開啟 8080 埠完成 Google 登入，成功後立刻關閉，平時不會佔用。</p>
    <button onclick="upgrade()">臨時開啟 8080 並用 Google 登入</button>
    <div id="ustate" class="hint"></div>
  </div>
</div>

<div class="card">
  <h2>② 手動貼上 token</h2>
  <div class="hint" style="margin-top:0" id="m2hint">不想開 8080 時改用這裡：在
    <a href="https://sjrc.club" target="_blank" rel="noreferrer">官網</a> 登入後把 token 貼進來，
    或先複製再按「從剪貼簿讀取」。</div>
  <textarea id="ta" placeholder='{"accessToken":"...","refreshToken":"..."}  或只貼 accessToken'></textarea>
  <div class="row">
    <button onclick="sendToken()">送出 token</button>
    <button class="ghost" onclick="readClip()">從剪貼簿讀取</button>
  </div>
  <div id="tstate" class="hint"></div>
</div>

<div class="card" id="okBox" style="display:none">
  <h2>登入完成</h2>
  <p id="okMsg" style="margin:0"></p>
  <div class="hint">本機服務即將關閉，可以關掉這個分頁回到桌面程式。</div>
</div>

<div class="foot">本頁只綁定回環位址（127.0.0.1 / ::1），不對外網開放；
  服務在登入完成後自動關閉，閒置 2 分鐘也會自動關閉。</div>
</div>
<script>
var CID='37114007982-n7m1iscmrmeifjrdkf062amtaklr487q.apps.googleusercontent.com';
var isGooglePort=(location.port==='8080');

document.getElementById('origin').textContent=location.origin;

function gs(t,cls){var e=document.getElementById('gstate');e.textContent=t;e.className='hint '+(cls||'');}
function decodeJwt(t){
  try{
    var p=t.split('.')[1].replace(/-/g,'+').replace(/_/g,'/');
    var s=atob(p),o='';
    for(var i=0;i<s.length;i++){o+='%'+('00'+s.charCodeAt(i).toString(16)).slice(-2);}
    return JSON.parse(decodeURIComponent(o));
  }catch(e){return null;}
}

window.addEventListener('load',function(){
  if(!isGooglePort){
    document.getElementById('gArea').style.display='none';
    document.getElementById('upArea').style.display='block';
    return;
  }
  if(!window.google||!google.accounts||!google.accounts.id){
    gs('Google SDK 未載入（網路被擋或離線），請用下面的 ② 手動貼 token','er');return;
  }
  var m2=document.getElementById('m2hint');
  if(m2) m2.innerHTML='也可以在 <a href="https://sjrc.club" target="_blank" rel="noreferrer">官網</a> 登入後把 token 貼進來，或先複製再按「從剪貼簿讀取」。';
  google.accounts.id.initialize({client_id:CID,callback:onCred,auto_select:false,ux_mode:'popup'});
  google.accounts.id.renderButton(document.getElementById('gbtn'),
    {type:'standard',theme:'outline',size:'large',text:'signin_with',shape:'rectangular',width:300});
  gs('Google 登入就緒，點上方按鈕選擇帳號','ok');
});

async function upgrade(){
  var us=document.getElementById('ustate');
  us.textContent='正在啟動 8080…';
  try{
    var r=await fetch('/upgrade-google',{cache:'no-store'});
    var j=await r.json();
    if(j.ok){ us.textContent='已啟動，正在跳轉…'; location.href=j.url; }
    else{ us.textContent=j.msg||'啟動失敗，請改用 ② 手動貼 token'; }
  }catch(e){ us.textContent='啟動失敗：'+e; }
}

async function onCred(resp){
  if(!resp||!resp.credential){gs('未取得 credential（可能取消了選擇）','er');return;}
  var c=decodeJwt(resp.credential);
  var who=(c&&(c.email||c.sub))||'(unknown)';
  gs('已選擇帳號：'+who+'，正在向 SJRC 換 token…','');
  try{
    var r=await fetch('/google-credential',{method:'POST',headers:{'Content-Type':'application/json'},
      body:JSON.stringify({credential:resp.credential})});
    var j=await r.json();
    if(j.ok){done(j.email);}
    else if(j.needCode){
      document.getElementById('codeBox').style.display='block';
      document.getElementById('code').focus();
      gs('已送出 Google 帳號，請輸入寄到郵箱的驗證碼','wn');
    }else{
      gs('SJRC 拒絕：'+(j.code||'')+' '+(j.msg||''),'er');
    }
  }catch(e){gs('本機服務無回應：'+e,'er');}
}

async function submitCode(){
  var code=document.getElementById('code').value.trim();
  var cs=document.getElementById('cstate');
  if(!code){cs.textContent='請輸入驗證碼';return;}
  cs.textContent='驗證中…';
  try{
    var r=await fetch('/verify',{method:'POST',headers:{'Content-Type':'application/json'},
      body:JSON.stringify({code:code})});
    var j=await r.json();
    if(j.ok){done(j.email);}
    else{cs.textContent='驗證失敗：'+(j.code||'')+' '+(j.msg||'');}
  }catch(e){cs.textContent='本機服務無回應：'+e;}
}

function done(email){
  document.getElementById('okBox').style.display='block';
  document.getElementById('okMsg').textContent='已登入：'+(email||'(unknown)')+'，token 已寫入桌面程式。';
  gs('登入完成','ok');
  document.getElementById('codeBox').style.display='none';
  document.getElementById('okBox').scrollIntoView({behavior:'smooth',block:'center'});
}

async function sendToken(){
  var t=document.getElementById('ta').value.trim();
  var st=document.getElementById('tstate');
  if(!t){st.textContent='請先貼上 token';return;}
  try{
    var r=await fetch('/token',{method:'POST',headers:{'Content-Type':'text/plain'},body:t});
    st.textContent=r.ok?'已送出，桌面程式會自動驗證並匯入。':'送出失敗。';
  }catch(e){st.textContent='送出失敗：'+e;}
}

async function readClip(){
  var st=document.getElementById('tstate');
  try{
    var t=await navigator.clipboard.readText();
    document.getElementById('ta').value=t;
    st.textContent='已讀取剪貼簿，按「送出 token」即可。';
  }catch(e){
    st.textContent='讀不到剪貼簿（瀏覽器權限），請手動 Ctrl+V 貼上。';
  }
}
</script>
</body></html>)HTML";
}

static void httpSend(SOCKET s, const std::string& status, const std::string& contentType, const std::string& body) {
    std::string msg = "HTTP/1.1 " + status + "\r\n";
    msg += "Content-Type: " + contentType + "\r\n";
    msg += "Content-Length: " + std::to_string(body.size()) + "\r\n";
    msg += "Cache-Control: no-store\r\n";
    msg += "Connection: close\r\n";
    msg += "Access-Control-Allow-Origin: *\r\n\r\n";
    msg += body;
    size_t total = msg.size();
    size_t sent = 0;
    while (sent < total) {
        int n = send(s, msg.c_str() + sent, (int)(total - sent), 0);
        if (n <= 0) break;
        sent += n;
    }
    closesocket(s);
}

// 直接對自己綁定的回環位址打 /ping（不經 DNS，避免被同埠的其他程式誤導）
static bool selfCheckAuthServer(int port, int family) {
    SOCKET c = socket(family, SOCK_STREAM, IPPROTO_TCP);
    if (c == INVALID_SOCKET) return false;
    DWORD tmo = 2000;
    setsockopt(c, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tmo, sizeof(tmo));
    setsockopt(c, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tmo, sizeof(tmo));
    int rc;
    if (family == AF_INET6) {
        sockaddr_in6 a{};
        a.sin6_family = AF_INET6;
        a.sin6_addr = in6addr_loopback;
        a.sin6_port = htons((u_short)port);
        rc = connect(c, (sockaddr*)&a, sizeof(a));
    } else {
        sockaddr_in a{};
        a.sin_family = AF_INET;
        a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        a.sin_port = htons((u_short)port);
        rc = connect(c, (sockaddr*)&a, sizeof(a));
    }
    bool ok = false;
    if (rc == 0) {
        const char* req = "GET /ping HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
        if (send(c, req, (int)strlen(req), 0) > 0) {
            std::string body;
            char buf[2048];
            while (body.size() < 4096) {
                int n = recv(c, buf, sizeof(buf), 0);
                if (n <= 0) break;
                body.append(buf, n);
            }
            ok = body.find("sjrc-auth-helper") != std::string::npos;
        }
    }
    closesocket(c);
    return ok;
}

static void postLoginUi(const LoginResult& r) {
    if (!g_hWnd) return;
    std::wstring t;
    if (r.ok) {
        t = L"登入成功：" + (r.email.empty() ? std::wstring(L"(unknown)") : r.email);
    } else if (r.needCode) {
        t = L"需要電郵驗證碼，請在登入頁輸入";
    } else {
        t = fmt(L"登入失敗：%d %s", r.code, r.msg.c_str());
    }
    {
        std::lock_guard<std::mutex> lk(g_mtx);
        g_loginUiText = t;
    }
    g_loginUiPending.store(true);
    PostMessageW(g_hWnd, WM_APP_STATE, 0, 0);
}

static bool startServer(Listener& L, int wantPort, DWORD idleMs);
static void stopServer(Listener& L);
static void requestShutdownSoon(DWORD delayMs);

static void serverLoop(Listener& L, int wantPort, DWORD idleMs) {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) { L.state.store(-1); return; }

    int family = AF_INET;
    SOCKET s = listenLoopback(AF_INET, wantPort);
    if (s == INVALID_SOCKET) { s = listenLoopback(AF_INET6, wantPort); family = AF_INET6; }
    if (s == INVALID_SOCKET) { L.state.store(-1); WSACleanup(); return; }

    L.sock = s;
    L.family.store(family);
    L.port.store(socketPort(s, family));
    L.lastHit.store(nowMs());
    L.state.store(2);

    while (L.state.load() == 2) {
        unsigned long long now = nowMs();
        if (g_shutdownAt.load() != 0 && now >= g_shutdownAt.load()) break;
        if (now - L.lastHit.load() > idleMs) break;

        fd_set fds; FD_ZERO(&fds); FD_SET(s, &fds);
        timeval tv{0, 300000};   // 300ms
        if (select(0, &fds, nullptr, nullptr, &tv) <= 0) continue;
        SOCKET client = accept(s, nullptr, nullptr);
        if (client == INVALID_SOCKET) continue;
        L.lastHit.store(nowMs());

        DWORD tmo = 5000;
        setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tmo, sizeof(tmo));

        std::string req;
        char buf[8192];
        while (req.size() < 262144) {
            int n = recv(client, buf, sizeof(buf), 0);
            if (n <= 0) break;
            req.append(buf, n);
            size_t he = req.find("\r\n\r\n");
            if (he != std::string::npos) {
                size_t cl = contentLengthOf(req.substr(0, he));
                if (req.size() >= he + 4 + cl) break;
            }
        }
        if (req.empty()) { closesocket(client); continue; }

        size_t he = req.find("\r\n\r\n");
        std::string head = (he == std::string::npos) ? req : req.substr(0, he);
        std::string body = (he == std::string::npos) ? std::string() : req.substr(he + 4);
        std::string path = requestPath(head);
        bool isPost = (head.rfind("POST ", 0) == 0);

        if (path == "/ping") {
            httpSend(client, "200 OK", "text/plain", "sjrc-auth-helper");
        } else if (path == "/favicon.ico") {
            httpSend(client, "204 No Content", "text/plain", "");
        } else if (path == "/upgrade-google") {
            std::string js;
            if (wantPort != AUTH_PORT_MAIN) {
                js = "{\"ok\":true,\"url\":\"http://localhost:8080/\"}";
            } else if (startServer(g_srvGoogle, AUTH_PORT_GOOGLE, AUTH_IDLE_MS_GGL)) {
                js = "{\"ok\":true,\"url\":\"http://localhost:8080/\"}";
            } else {
                js = "{\"ok\":false,\"msg\":\"8080 埠被其他程式佔用（IPv4/IPv6 都試過了），請改用下面的 ② 手動貼 token\"}";
            }
            httpSend(client, "200 OK", "application/json; charset=utf-8", js);
        } else if (isPost && path == "/google-credential") {
            std::string cred = jsonStr(body, "credential");
            if (cred.empty()) cred = body;
            LoginResult r = exchangeGoogleCredential(cred);
            if (r.needCode) {
                std::lock_guard<std::mutex> lk(g_mtx);
                g_pendingTicket = r.ticket;
            }
            std::string js = "{\"ok\":" + std::string(r.ok ? "true" : "false") +
                ",\"needCode\":" + std::string(r.needCode ? "true" : "false") +
                ",\"email\":\"" + jesc(narrow(r.email)) + "\"" +
                ",\"code\":" + std::to_string(r.code) +
                ",\"msg\":\"" + jesc(narrow(r.msg)) + "\"}";
            httpSend(client, "200 OK", "application/json; charset=utf-8", js);
            postLoginUi(r);
        } else if (isPost && path == "/verify") {
            std::wstring ticket;
            {
                std::lock_guard<std::mutex> lk(g_mtx);
                ticket = g_pendingTicket;
            }
            std::string code = jsonStr(body, "code");
            LoginResult r = submitVerificationCode(ticket, widen(code));
            if (r.ok) {
                std::lock_guard<std::mutex> lk(g_mtx);
                g_pendingTicket.clear();
            }
            std::string js = "{\"ok\":" + std::string(r.ok ? "true" : "false") +
                ",\"needCode\":false" +
                ",\"email\":\"" + jesc(narrow(r.email)) + "\"" +
                ",\"code\":" + std::to_string(r.code) +
                ",\"msg\":\"" + jesc(narrow(r.msg)) + "\"}";
            httpSend(client, "200 OK", "application/json; charset=utf-8", js);
            postLoginUi(r);
        } else if (isPost && path == "/token") {
            if (!body.empty() && g_hWnd) {
                std::wstring* wb = new std::wstring(widen(body));
                PostMessageW(g_hWnd, WM_APP_IMPORT_CLIPBOARD, 0, (LPARAM)wb);
            }
            httpSend(client, "200 OK", "application/json; charset=utf-8", "{\"ok\":true}");
        } else if (isPost) {
            httpSend(client, "404 Not Found", "text/plain", "not found");
        } else {
            httpSend(client, "200 OK", "text/html; charset=utf-8", authPageHtml());
        }
    }

    closesocket(s);
    L.sock = INVALID_SOCKET;
    L.port.store(0);
    L.state.store(0);
    WSACleanup();
}

static bool startServer(Listener& L, int wantPort, DWORD idleMs) {
    int st = L.state.load();
    if (st == 2) return true;
    if (st == 1) {
        for (int i = 0; i < 80 && L.state.load() == 1; ++i) Sleep(25);
        return L.state.load() == 2;
    }
    L.state.store(1);
    g_shutdownAt.store(0);   // 重新啟動 → 取消上一次的關閉排程
    std::thread([&L, wantPort, idleMs]() { serverLoop(L, wantPort, idleMs); }).detach();
    for (int i = 0; i < 80; ++i) {
        int cur = L.state.load();
        if (cur == 2) return true;
        if (cur == -1) return false;
        Sleep(25);
    }
    return false;
}

static void stopServer(Listener& L) {
    if (L.state.load() == 2) L.state.store(0);
}

static void stopAllServers() {
    g_shutdownAt.store(0);
    stopServer(g_srvMain);
    stopServer(g_srvGoogle);
}

static void requestShutdownSoon(DWORD delayMs) {
    unsigned long long t = nowMs() + delayMs;
    unsigned long long cur = g_shutdownAt.load();
    if (cur == 0 || t < cur) g_shutdownAt.store(t);
}

static void checkClipboardForToken() {
    if (!OpenClipboard(g_hWnd)) return;
    HANDLE h = GetClipboardData(CF_UNICODETEXT);
    if (!h) { CloseClipboard(); return; }
    wchar_t* p = (wchar_t*)GlobalLock(h);
    if (!p) { CloseClipboard(); return; }
    std::wstring txt(p);
    GlobalUnlock(h);
    CloseClipboard();

    if (txt.size() < 30) return;
    if (txt.find(L"accessToken") == std::wstring::npos && txt.find(L"refreshToken") == std::wstring::npos) return;

    std::wstring hash = fmt(L"%zu", std::hash<std::wstring>{}(txt));
    if (hash == g_lastClipboardHash) return;
    g_lastClipboardHash = hash;

    logLine(L"偵測到剪貼簿含有 token，自動導入…");
    tryImportText(txt);
}

static void refreshAuthLabel() {
    if (!g_authInfo) return;
    std::wstring t;
    int st = g_srvMain.state.load();
    if (st == 2) {
        t = fmt(L"登入服務：localhost:%d · 登入完成即關", g_srvMain.port.load());
        if (g_srvGoogle.state.load() == 2) t += L" · 8080 已開";
    } else if (st == 1) {
        t = L"登入服務：啟動中…";
    } else {
        t = L"登入服務：未啟動";
    }
    wchar_t cur[512] = {0};
    GetWindowTextW(g_authInfo, cur, 512);
    if (t != cur) SetWindowTextW(g_authInfo, t.c_str());
}

static void openOAuthPage() {
    g_shutdownAt.store(0);

    if (g_srvMain.state.load() == 2) {
        std::wstring url = fmt(L"http://localhost:%d/", g_srvMain.port.load());
        ShellExecuteW(nullptr, L"open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
        SetWindowTextW(g_tokenState, L"登入頁已開啟（登入完成或閒置 2 分鐘會自動關閉）");
        return;
    }

    if (!startServer(g_srvMain, AUTH_PORT_MAIN, AUTH_IDLE_MS_MAIN)) {
        SetWindowTextW(g_authInfo, L"登入服務啟動失敗（埠被佔用）");
        logLine(fmt(L"本機登入服務啟動失敗：埠 %d 被佔用", AUTH_PORT_MAIN));
        SetWindowTextW(g_tokenState, L"登入服務啟動失敗，請用「手動導入」貼上 token");
        return;
    }

    int port = g_srvMain.port.load();
    bool selfOk = selfCheckAuthServer(port, g_srvMain.family.load());
    logLine(fmt(L"登入服務已啟動：http://localhost:%d/（登入完成或閒置 2 分鐘自動關閉）", port));
    if (!selfOk) logLine(L"警告：自我檢查失敗，瀏覽器可能連不到本機服務");

    std::wstring url = fmt(L"http://localhost:%d/", port);
    ShellExecuteW(nullptr, L"open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    SetWindowTextW(g_tokenState, L"已開啟登入頁（登入完成後自動關閉服務）");
    refreshAuthLabel();
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE: {
        g_hWnd = hwnd;

        // Token
        mkStatic(L"登入方式：本機登入頁（Google 一鍵登入）或直接貼上 token", 36, 106, 548, 18, g_fontSmall, -1);
        g_authInfo = mkStatic(L"登入服務：未啟動", 36, 128, 344, 18, g_fontSmall, -1);
        g_btnOAuth = mkButton(L"開啟 Google 登入頁", 388, 120, 196, 28, ID_BTN_OAUTH, true);
        g_tokenState = mkStatic(L"尚未登入", 36, 152, 548, 18, g_fontSmall, ID_TOKEN_STATE);
        g_editToken = mkEdit(L"", 36, 176, 424, 24, 0, ID_TOKEN, false);
        g_btnImport = mkButton(L"手動導入", 468, 175, 116, 26, ID_BTN_IMPORT, false);

        // 參數
        mkStatic(L"目標時長（小時）", 36, 258, 120, 18, g_fontSmall, -1);
        g_target = mkEdit(L"2", 36, 278, 100, 26, 0, ID_TARGET, false);
        mkStatic(L"心跳（秒）", 150, 258, 80, 18, g_fontSmall, -1);
        g_hb = mkEdit(L"3", 150, 278, 70, 26, 0, ID_HB, false);
        mkStatic(L"翻頁（秒）", 234, 258, 80, 18, g_fontSmall, -1);
        g_turn = mkEdit(L"20", 234, 278, 70, 26, 0, ID_TURN, false);
        g_btnStart = mkButton(L"開始自動閱讀", 36, 314, 130, 30, ID_BTN_START, true);
        g_btnStop = mkButton(L"停止", 176, 314, 90, 30, ID_BTN_STOP, false);
        mkStatic(L"關閉視窗會自動停止並收尾", 278, 320, 300, 18, g_fontSmall, -1);

        // 書本
        int y = 400;
        const wchar_t* labels[] = { L"書名", L"作者", L"Book ID", L"章節", L"頁碼" };
        int ids[] = { ID_ST_BOOK, ID_ST_AUTHOR, ID_ST_BOOKID, ID_ST_CHAPTERS, ID_ST_PAGE };
        for (int i = 0; i < 5; ++i) {
            mkStatic(labels[i], 36, y + i * 22, 70, 18, g_fontSmall, -1);
            mkStatic(L"—", 108, y + i * 22, 460, 18, g_fontUI, ids[i]);
        }

        // 帳號與閱讀數據卡（右欄下方）
        g_btnRefresh = mkButton(L"重新整理", 872, 298, 92, 24, ID_BTN_REFRESH, false);
        {
            struct Row { const wchar_t* label; int id; int ly, lw, vx, vw; };
            const Row rows[] = {
                { L"姓名",     ID_ST_NAME,      332, 60, 700, 264 },
                { L"電郵",     ID_ST_EMAIL,     355, 60, 700, 264 },
                { L"學校",     ID_ST_SCHOOL,    378, 60, 700, 264 },
                { L"身份",     ID_ST_IDENT,     401, 60, 700, 264 },
                { L"累計已讀", ID_ST_TOTALUSED, 430, 60, 700, 264 },
                { L"已讀本數", ID_ST_READCNT,   455, 60, 700, 264 },
                { L"全校用量", ID_ST_SCHOOLQ,   480, 60, 700, 264 },
            };
            for (auto& r : rows) {
                mkStatic(r.label, 636, r.ly, r.lw, 18, g_fontSmall, -1);
                mkStatic(L"—", r.vx, r.ly, r.vw, 18, g_fontUI, r.id);
            }
        }

        // 即時狀態（日誌卡標題列右側）
        mkStatic(L"—", 250, 540, 714, 18, g_fontSmall, ID_ST_RUNLINE);

        // 日誌
        g_log = mkEdit(L"", 36, 562, 928, 142, 0, ID_LOG, true);
        SendMessageW(g_log, EM_SETREADONLY, TRUE, 0);

        SetTimer(hwnd, ID_TIMER, 500, nullptr);
        AddClipboardFormatListener(hwnd);

        // 還原設定
        SetWindowTextW(g_target, loadSetting(L"target", L"2").c_str());
        SetWindowTextW(g_hb, loadSetting(L"hb", L"3").c_str());
        SetWindowTextW(g_turn, loadSetting(L"turn", L"20").c_str());
        // 讓「04 · 進度」卡片的目標時長跟輸入框一致（否則會顯示預設 2.0，跟框裡的值對不上）
        {
            wchar_t tb[64] = {0};
            GetWindowTextW(g_target, tb, 64);
            double v = _wtof(tb);
            if (v > 0) setSnap([&](Snapshot& s) { s.targetX10 = (int)(v * 10 + 0.5); });
        }
        {
            std::wstring tk = loadSetting(L"token", L"");
            std::wstring rf = loadSetting(L"refresh", L"");
            std::lock_guard<std::mutex> lk(g_mtx);
            g_token = narrow(tk);
            g_refresh = narrow(rf);
        }
        logLine(L"SJRC 自動閱讀器（原生版）已啟動");
        if (!g_token.empty()) {
            logLine(L"已載入上次的 token，可直接按「開始自動閱讀」");
            // 開機就先抓一次帳號／閱讀數據，介面不用等按鈕
            std::thread([]() { loadAccountInfo(); }).detach();
        }
        return 0;
    }

    case WM_APP_STATE: {
        if (g_loginUiPending.exchange(false) && g_tokenState) {
            std::wstring t;
            {
                std::lock_guard<std::mutex> lk(g_mtx);
                t = g_loginUiText;
            }
            SetWindowTextW(g_tokenState, t.c_str());
        }
        drainLog();
        refreshControls();
        // 進度大字 / 進度條 / 配額條都畫在視窗上（不是控件），不重畫就永遠是舊值
        InvalidateRect(hwnd, nullptr, FALSE);
        return 0;
    }

    case WM_ERASEBKGND:
        return 1;

    case WM_TIMER:
        if (wp == ID_TIMER) { drainLog(); refreshControls(); refreshAuthLabel(); }
        return 0;

    case WM_CLIPBOARDUPDATE:
        checkClipboardForToken();
        return 0;

    case WM_APP_IMPORT_CLIPBOARD: {
        std::wstring* txt = (std::wstring*)lp;
        if (txt) {
            logLine(L"Auth helper sent token, auto importing...");
            tryImportText(*txt);
            delete txt;
        }
        return 0;
    }

    case WM_APP_GOOGLE_CRED: {
        std::wstring* cred = (std::wstring*)lp;
        if (cred) {
            handleGoogleCredential(*cred);
            delete cred;
        }
        return 0;
    }

    case WM_APP_IMPORT: {
        ImportResult* r = (ImportResult*)lp;
        if (r) {
            if (r->ok) {
                SetWindowTextW(g_tokenState, fmt(L"已登入：%s", r->email.c_str()).c_str());
                logLine(L"token 可用 · " + (r->email.empty() ? L"(未知)" : r->email));
                setSnap([&](Snapshot& s2) { s2.user = r->email; });
                requestShutdownSoon(2000);   // 已匯入 → 本機服務自動關閉
            } else {
                SetWindowTextW(g_tokenState, fmt(L"失敗：%d %s", r->code, r->msg.c_str()).c_str());
                logLine(L"token 驗證失敗：" + fmt(L"%d", r->code) + L" " + r->msg);
            }
            delete r;
        }
        refreshControls();
        return 0;
    }

    case WM_CTLCOLORSTATIC: {
        // 必須用實色背景 + OPAQUE：之前回 NULL_BRUSH + TRANSPARENT，
        // 文字更新時舊字不會被擦掉，狀態行改幾次就疊成一團。
        HDC dc = (HDC)wp;
        SetBkMode(dc, OPAQUE);
        SetBkColor(dc, C_CARD);
        SetTextColor(dc, C_TXT);
        return (LRESULT)g_brCard;
    }

    case WM_CTLCOLOREDIT: {
        HDC dc = (HDC)wp;
        HWND h = (HWND)lp;
        SetTextColor(dc, C_TXT);
        if (h == g_log) {
            SetBkColor(dc, RGB(0xFB, 0xFD, 0xFC));
            return (LRESULT)g_brLog;
        }
        SetBkColor(dc, C_INPUT);
        return (LRESULT)g_brInput;
    }

    case WM_DRAWITEM: {
        LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lp;
        bool primary = GetWindowLongPtrW(dis->hwndItem, GWLP_USERDATA) != 0;
        bool disabled = (dis->itemState & ODS_DISABLED) != 0;
        RECT r = dis->rcItem;

        Gdiplus::Graphics g(dis->hDC);
        g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
        Gdiplus::GraphicsPath path;
        int rad = 6;
        path.AddArc(r.left, r.top, rad * 2, rad * 2, 180, 90);
        path.AddArc(r.right - rad * 2, r.top, rad * 2, rad * 2, 270, 90);
        path.AddArc(r.right - rad * 2, r.bottom - rad * 2, rad * 2, rad * 2, 0, 90);
        path.AddArc(r.left, r.bottom - rad * 2, rad * 2, rad * 2, 90, 90);
        path.CloseFigure();

        COLORREF fill = primary ? C_ACCD : RGB(0xF1, 0xF6, 0xF4);
        if (disabled) fill = primary ? RGB(0x9F, 0xC7, 0xBE) : RGB(0xF5, 0xF7, 0xF6);
        Gdiplus::SolidBrush br(Gdiplus::Color(255, GetRValue(fill), GetGValue(fill), GetBValue(fill)));
        g.FillPath(&br, &path);
        Gdiplus::Pen pen(Gdiplus::Color(255, 0xE3, 0xEA, 0xE7), 1.0f);
        if (!primary) g.DrawPath(&pen, &path);

        wchar_t text[128];
        GetWindowTextW(dis->hwndItem, text, 128);
        SetBkMode(dis->hDC, TRANSPARENT);
        SetTextColor(dis->hDC, primary ? RGB(255, 255, 255) : C_TXT);
        SelectObject(dis->hDC, g_fontUI);
        DrawTextW(dis->hDC, text, -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        return TRUE;
    }

    case WM_COMMAND: {
        int id = LOWORD(wp);
        if (id == ID_BTN_IMPORT) { doImport(); return 0; }
        if (id == ID_BTN_REFRESH) {
            if (g_token.empty()) { SetWindowTextW(g_tokenState, L"尚未登入，無法讀取帳號資料"); return 0; }
            logLine(L"重新整理帳號資料…");
            std::thread([]() { loadAccountInfo(); }).detach();
            return 0;
        }
        if (id == ID_BTN_OAUTH) { openOAuthPage(); return 0; }
        if (id == ID_BTN_START) { startRun(); return 0; }
        if (id == ID_BTN_STOP) { stopRun(); return 0; }
        return 0;
    }

    case WM_PAINT:
        paintWindow(hwnd);
        return 0;

    case WM_CLOSE: {
        if (g_worker.joinable()) {
            g_stop.store(true);
            g_worker.join();
        }
        KillTimer(hwnd, ID_TIMER);
        DestroyWindow(hwnd);
        return 0;
    }

    case WM_DESTROY:
        stopAllServers();
        RemoveClipboardFormatListener(hwnd);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR, int) {
    g_hInst = hInst;
    SetProcessDPIAware();

    INITCOMMONCONTROLSEX icc{ sizeof(icc), ICC_STANDARD_CLASSES };
    InitCommonControlsEx(&icc);

    Gdiplus::GdiplusStartupInput gsi;
    ULONG_PTR gdipToken = 0;
    Gdiplus::GdiplusStartup(&gdipToken, &gsi, nullptr);

    g_fontUI = CreateFontW(-15, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
                           OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                           DEFAULT_PITCH, L"Microsoft JhengHei UI");
    g_fontUIB = CreateFontW(-15, 0, 0, 0, FW_SEMIBOLD, 0, 0, 0, DEFAULT_CHARSET,
                            OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                            DEFAULT_PITCH, L"Microsoft JhengHei UI");
    g_fontBig = CreateFontW(-32, 0, 0, 0, FW_SEMIBOLD, 0, 0, 0, DEFAULT_CHARSET,
                            OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                            DEFAULT_PITCH, L"Microsoft JhengHei UI");
    g_fontSmall = CreateFontW(-12, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
                              OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                              DEFAULT_PITCH, L"Microsoft JhengHei UI");
    g_fontMono = CreateFontW(-13, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
                             OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                             DEFAULT_PITCH, L"Consolas");

    g_brBg = CreateSolidBrush(C_BG);
    g_brCard = CreateSolidBrush(C_CARD);
    g_brInput = CreateSolidBrush(C_INPUT);
    g_brLog = CreateSolidBrush(RGB(0xFB, 0xFD, 0xFC));

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = g_brBg;
    wc.lpszClassName = L"SJRCAutoReaderWnd";
    wc.hIcon = LoadIconW(hInst, MAKEINTRESOURCEW(1));
    wc.hIconSm = (HICON)LoadImageW(hInst, MAKEINTRESOURCEW(1), IMAGE_ICON,
                                   GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
    RegisterClassExW(&wc);

    // 以「客戶區 1000x760」為準，補上標題列與邊框
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN;
    RECT wr{ 0, 0, 1000, 760 };
    AdjustWindowRect(&wr, style, FALSE);
    int W = wr.right - wr.left, H = wr.bottom - wr.top;
    int sx = GetSystemMetrics(SM_CXSCREEN), sy = GetSystemMetrics(SM_CYSCREEN);
    g_hWnd = CreateWindowExW(0, wc.lpszClassName, APP_TITLE, style,
                             (sx - W) / 2, (sy - H) / 2, W, H,
                             nullptr, nullptr, hInst, nullptr);
    if (!g_hWnd) return 1;

    SendMessageW(g_hWnd, WM_SETICON, ICON_BIG, (LPARAM)wc.hIcon);
    SendMessageW(g_hWnd, WM_SETICON, ICON_SMALL, (LPARAM)wc.hIconSm);

    ShowWindow(g_hWnd, SW_SHOW);
    UpdateWindow(g_hWnd);
    refreshControls();

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    if (g_worker.joinable()) { g_stop.store(true); g_worker.join(); }
    Gdiplus::GdiplusShutdown(gdipToken);
    return 0;
}
