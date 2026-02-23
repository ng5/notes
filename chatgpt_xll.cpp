// GRIDFX_core.cpp
//
// FRAMEWRK.H + XLCALL.H assumed.
// Implements:
//   - =GRIDFX([syms]) worksheet UDF (queues job)
//   - xlAutoOpen registers GRIDFX
//   - xlAutoClose shuts down pump window
//
// Registration notes:
//   - return type: Q  (LPXLOPER12)
//   - arg type:    P  (LPXLOPER12)  (optional; Excel can pass Missing)
//   - category:    "1" (Financial) or any category you prefer
//
// You still need: project exports/DEF or __declspec(dllexport) as you use elsewhere.

#define NOMINMAX
#include <windows.h>

#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "xlcall.h"
#include "framewrk.h"

// ----------------------------- Config -----------------------------

static const UINT WM_GRIDFX_PUMP = WM_APP + 0x4A;
static const wchar_t* kDefaultSyms[] = { L"EUR", L"GBP" };

// ----------------------------- State -----------------------------

static HINSTANCE g_hInst = nullptr;
static HWND g_hwndPump = nullptr;

struct GridJob {
    XLOPER12 anchorRef;                 // deep-copied xltypeRef
    std::vector<std::wstring> syms;
    uint64_t payload_hash;
};

static std::mutex g_jobsMtx;
static std::vector<GridJob> g_jobs;

static std::mutex g_dedupMtx;
static std::unordered_map<uint64_t, uint64_t> g_lastHash;

// ----------------------------- Hashing -----------------------------

static uint64_t fnv1a64_bytes(const void* data, size_t n) {
    const uint8_t* p = static_cast<const uint8_t*>(data);
    uint64_t h = 1469598103934665603ull;
    for (size_t i = 0; i < n; i++) {
        h ^= uint64_t(p[i]);
        h *= 1099511628211ull;
    }
    return h;
}

static uint64_t fnv1a64_wstr(const std::wstring& s) {
    return fnv1a64_bytes(s.data(), s.size() * sizeof(wchar_t));
}

static uint64_t hash_syms(const std::vector<std::wstring>& syms) {
    uint64_t h = 1469598103934665603ull;
    for (auto& w : syms) {
        uint64_t x = fnv1a64_wstr(w);
        h ^= x;
        h *= 1099511628211ull;
    }
    return h;
}

static uint64_t anchor_key(const XLOPER12& ref) {
    if (ref.xltype != xltypeRef || !ref.val.mref.lpmref) return 0;
    const XLMREF* mref = ref.val.mref.lpmref;
    if (mref->count < 1) return 0;
    const XLREF12& r = mref->reftbl[0];

    const uint64_t sid = uint64_t(uint32_t(ref.val.mref.idSheet));
    const uint64_t row = uint64_t(uint16_t(r.rwFirst));
    const uint64_t col = uint64_t(uint16_t(r.colFirst));
    return (sid << 32) ^ (row << 16) ^ col;
}

// ----------------------------- Ref deep copy -----------------------------

static bool deep_copy_ref(const XLOPER12& in, XLOPER12* out) {
    if (!out) return false;
    if (in.xltype != xltypeRef || !in.val.mref.lpmref) return false;

    const XLMREF* src = in.val.mref.lpmref;
    const size_t n = src->count;
    if (n < 1) return false;

    const size_t bytes = sizeof(XLMREF) + (n - 1) * sizeof(XLREF12);
    XLMREF* dst = static_cast<XLMREF*>(std::malloc(bytes));
    if (!dst) return false;

    std::memcpy(dst, src, bytes);

    out->xltype = xltypeRef;
    out->val.mref.idSheet = in.val.mref.idSheet;
    out->val.mref.lpmref = dst;
    return true;
}

static void free_deep_ref(XLOPER12* op) {
    if (!op) return;
    if (op->xltype == xltypeRef && op->val.mref.lpmref) {
        std::free(op->val.mref.lpmref);
        op->val.mref.lpmref = nullptr;
    }
    op->xltype = xltypeNil;
}

// ----------------------------- Excel safety -----------------------------

static bool is_safe_to_modify_sheet() {
    XLOPER12 ws;
    int rc = Excel12(xlfGetWorkspace, &ws, 1, TempInt12(44));
    if (rc != xlretSuccess) return true;

    bool inEdit = false;
    if (ws.xltype == xltypeBool) inEdit = (ws.val.xbool != 0);

    Excel12(xlFree, 0, 1, &ws);
    return !inEdit;
}

// ----------------------------- Symbol parsing -----------------------------

static std::wstring coerce_to_wstring(const XLOPER12& op) {
    if (op.xltype == xltypeStr && op.val.str) {
        const XCHAR* p = op.val.str;
        const int len = (int)p[0];
        return std::wstring(p + 1, p + 1 + len);
    }
    XLOPER12 txt;
    if (Excel12(xlfText, &txt, 1, const_cast<XLOPER12*>(&op)) == xlretSuccess &&
        txt.xltype == xltypeStr && txt.val.str) {
        const XCHAR* p = txt.val.str;
        const int len = (int)p[0];
        std::wstring w(p + 1, p + 1 + len);
        Excel12(xlFree, 0, 1, &txt);
        return w;
    }
    return L"";
}

static std::vector<std::wstring> parse_syms(LPXLOPER12 arg0) {
    std::vector<std::wstring> syms;

    if (!arg0 || (arg0->xltype & xltypeMissing) || (arg0->xltype & xltypeNil)) {
        for (auto* s : kDefaultSyms) syms.emplace_back(s);
        return syms;
    }

    if ((arg0->xltype & xltypeMulti) && arg0->val.array.lparray) {
        const int rows = arg0->val.array.rows;
        const int cols = arg0->val.array.columns;
        XLOPER12* arr = arg0->val.array.lparray;

        syms.reserve(size_t(rows) * size_t(cols));
        for (int i = 0; i < rows * cols; i++) {
            std::wstring w = coerce_to_wstring(arr[i]);
            if (!w.empty()) syms.push_back(std::move(w));
        }
        if (!syms.empty()) return syms;
    }

    // Scalar
    {
        std::wstring w = coerce_to_wstring(*arg0);
        if (!w.empty()) syms.push_back(std::move(w));
    }

    if (syms.empty()) {
        for (auto* s : kDefaultSyms) syms.emplace_back(s);
    }
    return syms;
}

// ----------------------------- Addressing cells (OFFSET) -----------------------------

static bool make_single_cell_ref(const XLOPER12& anchorRef, int dr, int dc, XLOPER12* outRef) {
    if (!outRef) return false;
    int rc = Excel12(
        xlfOffset, outRef, 5,
        const_cast<XLOPER12*>(&anchorRef),
        TempInt12(dr), TempInt12(dc),
        TempInt12(1), TempInt12(1)
    );
    return (rc == xlretSuccess) && (outRef->xltype == xltypeRef);
}

// ----------------------------- Injection -----------------------------

static void inject_one_job(GridJob& job) {
    if (!is_safe_to_modify_sheet())
        return;

    for (size_t i = 0; i < job.syms.size(); i++) {
        XLOPER12 dstRef;
        if (!make_single_cell_ref(job.anchorRef, int(i + 1), 0, &dstRef))
            continue;

        std::wstring f = L"=URTD(\"";
        f += job.syms[i];
        f += L"\")";

        Excel12(xlcFormula, 0, 2, TempStr12(f.c_str()), &dstRef);
        Excel12(xlFree, 0, 1, &dstRef);
    }
}

static void run_jobs_now() {
    std::vector<GridJob> local;
    {
        std::lock_guard<std::mutex> lk(g_jobsMtx);
        local.swap(g_jobs);
    }

    for (auto& job : local) {
        inject_one_job(job);
        free_deep_ref(&job.anchorRef);
    }
}

// ----------------------------- Hidden pump window -----------------------------

static LRESULT CALLBACK PumpWndProc(HWND hwnd, UINT msg, WPARAM, LPARAM) {
    if (msg == WM_GRIDFX_PUMP) {
        run_jobs_now();
        return 0;
    }
    return DefWindowProcW(hwnd, msg, 0, 0);
}

static bool ensure_pump_window() {
    if (g_hwndPump) return true;

    if (!g_hInst) g_hInst = (HINSTANCE)GetModuleHandleW(nullptr);

    WNDCLASSW wc{};
    wc.lpfnWndProc = PumpWndProc;
    wc.hInstance = g_hInst;
    wc.lpszClassName = L"GRIDFX_PUMP_WINDOW";
    RegisterClassW(&wc);

    g_hwndPump = CreateWindowExW(
        0, wc.lpszClassName, L"", 0,
        0, 0, 0, 0,
        HWND_MESSAGE, nullptr, g_hInst, nullptr
    );

    return g_hwndPump != nullptr;
}

static void request_pump() {
    if (!ensure_pump_window()) return;
    PostMessageW(g_hwndPump, WM_GRIDFX_PUMP, 0, 0);
}

// ----------------------------- UDF: =GRIDFX([syms]) -----------------------------

extern "C" __declspec(dllexport)
LPXLOPER12 WINAPI GRIDFX(LPXLOPER12 symsArg)
{
    static XLOPER12 retEmpty = *TempStr12(L""); // stable scalar

    XLOPER12 caller;
    if (Excel12(xlfCaller, &caller, 0) != xlretSuccess)
        return &retEmpty;

    if (caller.xltype != xltypeRef || !caller.val.mref.lpmref) {
        Excel12(xlFree, 0, 1, &caller);
        return &retEmpty;
    }

    XLOPER12 callerCopy;
    if (!deep_copy_ref(caller, &callerCopy)) {
        Excel12(xlFree, 0, 1, &caller);
        return &retEmpty;
    }

    auto syms = parse_syms(symsArg);
    const uint64_t ph = hash_syms(syms);
    const uint64_t akey = anchor_key(callerCopy);

    {
        std::lock_guard<std::mutex> lk(g_dedupMtx);
        auto it = g_lastHash.find(akey);
        if (it != g_lastHash.end() && it->second == ph) {
            free_deep_ref(&callerCopy);
            Excel12(xlFree, 0, 1, &caller);
            return &retEmpty;
        }
        g_lastHash[akey] = ph;
    }

    {
        std::lock_guard<std::mutex> lk(g_jobsMtx);
        g_jobs.push_back(GridJob{ callerCopy, std::move(syms), ph });
    }

    request_pump();

    Excel12(xlFree, 0, 1, &caller);
    return &retEmpty;
}

// ----------------------------- xlAutoOpen / xlAutoClose -----------------------------

static void register_GRIDFX(const XLOPER12& xDLL)
{
    // xlfRegister args (Excel 12):
    // 1) module_text (xDLL)
    // 2) procedure    (export name)
    // 3) type_text    (return+args types)
    // 4) function_text(display name)
    // 5) argument_text
    // 6) macro_type (0=worksheet func). Omit / empty for normal UDF.
    // 7) category
    // 8) shortcut_text
    // 9) help_topic
    // 10) function_help
    // 11) argument_help1 ...
    //
    // We'll provide up to 11 args total to include a helpful tooltip.
    Excel12(xlfRegister, 0, 11,
        const_cast<XLOPER12*>(&xDLL),
        TempStr12(L"GRIDFX"),            // export
        TempStr12(L"QP"),                // Q=LPXLOPER12 return, P=LPXLOPER12 arg
        TempStr12(L"GRIDFX"),            // worksheet name
        TempStr12(L"syms"),              // argument name
        TempStr12(L"1"),                 // category (1=Financial) - change as you like
        TempStr12(L""),                  // shortcut
        TempStr12(L""),                  // help topic
        TempStr12(L"Materialize URTD rows below this cell"), // function help
        TempStr12(L"Optional: symbols (range/array/scalar). Default: {\"EUR\",\"GBP\"}"),
        TempStr12(L"")                   // (extra arg help slot; keep count = 11)
    );
}

extern "C" __declspec(dllexport)
int WINAPI xlAutoOpen(void)
{
    // Capture module name for xlfRegister
    XLOPER12 xDLL;
    if (Excel12(xlGetName, &xDLL, 0) != xlretSuccess)
        return 0;

    // Init core infrastructure
    g_hInst = (HINSTANCE)GetModuleHandleW(nullptr);
    ensure_pump_window();

    // Register functions
    register_GRIDFX(xDLL);

    Excel12(xlFree, 0, 1, &xDLL);
    return 1;
}

extern "C" __declspec(dllexport)
int WINAPI xlAutoClose(void)
{
    if (g_hwndPump) {
        DestroyWindow(g_hwndPump);
        g_hwndPump = nullptr;
    }
    return 1;
}
