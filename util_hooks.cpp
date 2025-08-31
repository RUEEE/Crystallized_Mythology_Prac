#include "util_hooks.h"
#define NO_MIN_MAX
#include <Windows.h>
#include <vector>
#include <string>
#include <set>
#include "3rd/thp/thprac_hook.h"

HFONT(WINAPI* g_realCreateFontA)(int cHeight, int cWidth, int cEscapement, int cOrientation, int cWeight, DWORD bItalic, DWORD bUnderline, DWORD bStrikeOut, DWORD iCharSet, DWORD iOutPrecision, DWORD iClipPrecision, DWORD iQuality, DWORD iPitchAndFamily, LPCSTR pszFaceName);
HFONT(WINAPI* g_realCreateFontW)(int cHeight, int cWidth, int cEscapement, int cOrientation, int cWeight, DWORD bItalic, DWORD bUnderline, DWORD bStrikeOut, DWORD iCharSet, DWORD iOutPrecision, DWORD iClipPrecision, DWORD iQuality, DWORD iPitchAndFamily, LPCWSTR pszFaceName);

std::set<std::string> g_fonts; //make sure fonts are arranged

std::string g_customFont = "";
int g_charset = SHIFTJIS_CHARSET;

int CALLBACK EnumFontFamExProc(ENUMLOGFONTEXA* lpelfe, const TEXTMETRICA* lpntme, DWORD FontType, LPARAM lParam)
{
    if (lpelfe == NULL)
        return 0;
    if (lpelfe->elfFullName[0] != '@')
        g_fonts.insert(std::string((char*)lpelfe->elfFullName));
    return 1;
}
std::vector<std::string>& EnumAllFonts()
{
    static std::vector<std::string> res;
    if (res.size() != 0)
        return res;
    LOGFONTA logFont = { 0 };
    logFont.lfFaceName[0] = '\0';
    logFont.lfCharSet = SHIFTJIS_CHARSET;
    EnumFontFamiliesExA(GetDC(NULL), &logFont, (FONTENUMPROCA)EnumFontFamExProc, 0, 0);
    for (auto& i : g_fonts)
        res.push_back(i);
    return res;
}

HFONT WINAPI CreateFontA_Changed
(int cHeight, int cWidth, int cEscapement, int cOrientation, int cWeight, DWORD bItalic, DWORD bUnderline,
    DWORD bStrikeOut, DWORD iCharSet, DWORD iOutPrecision, DWORD iClipPrecision, DWORD iQuality, DWORD iPitchAndFamily, LPCSTR pszFaceName)
{
    if (g_charset != 0)
        iCharSet = g_charset;
    static std::vector<std::string> fonts = EnumAllFonts();
    unsigned char font_Gothic[] = { 0x82, 0x6C, 0x82, 0x72, 0x20, 0x83, 0x53, 0x83, 0x56, 0x83, 0x62, 0x83, 0x4E, 0x00 }; // ‚l‚r ƒSƒVƒbƒN, MS Gothic
    unsigned char font_Mincho[] = { 0x82, 0x6C, 0x82, 0x72, 0x20, 0x96, 0xBE, 0x92, 0xA9, 0x00 }; // ‚l‚r –¾’©, MS Mincho
    if (g_customFont!="")
        return g_realCreateFontA(cHeight, cWidth, cEscapement, cOrientation, cWeight, bItalic, bUnderline, bStrikeOut, iCharSet, iOutPrecision, iClipPrecision, iQuality, iPitchAndFamily, g_customFont.c_str());
    if (strcmp((char*)font_Gothic, pszFaceName) == 0) {
        return g_realCreateFontA(cHeight, cWidth, cEscapement, cOrientation, cWeight, bItalic, bUnderline, bStrikeOut, iCharSet, iOutPrecision, iClipPrecision, iQuality, iPitchAndFamily, "MS Gothic");
    } else if (strcmp((char*)font_Mincho, pszFaceName) == 0) {
        // some computer might not have mincho font
        if (std::find(fonts.begin(), fonts.end(), "MS Mincho") == fonts.end())
            return g_realCreateFontA(cHeight, cWidth, cEscapement, cOrientation, cWeight, bItalic, bUnderline, bStrikeOut, iCharSet, iOutPrecision, iClipPrecision, iQuality, iPitchAndFamily, "MS Gothic");
        return g_realCreateFontA(cHeight, cWidth, cEscapement, cOrientation, cWeight, bItalic, bUnderline, bStrikeOut, iCharSet, iOutPrecision, iClipPrecision, iQuality, iPitchAndFamily, "MS Mincho");
    }
    return g_realCreateFontA(cHeight, cWidth, cEscapement, cOrientation, cWeight, bItalic, bUnderline, bStrikeOut, iCharSet, iOutPrecision, iClipPrecision, iQuality, iPitchAndFamily, pszFaceName);
}

HFONT WINAPI CreateFontW_Changed(int cHeight, int cWidth, int cEscapement, int cOrientation, int cWeight, DWORD bItalic, DWORD bUnderline,
    DWORD bStrikeOut, DWORD iCharSet, DWORD iOutPrecision, DWORD iClipPrecision, DWORD iQuality, DWORD iPitchAndFamily, LPCWSTR pszFaceName)
{

    if (g_charset != 0)
        iCharSet = g_charset;
    static std::vector<std::string> fonts = EnumAllFonts();
    wchar_t font_YuGothic[] = { 0x6E38, 0x30B4, 0x30B7, 0x30C3, 0x30AF, 0 };//Yu Gothic...
    wchar_t font_YuMincho[] = L"Yu Mincho";
    wchar_t font_YuMeiryo[] = { 0x30E1, 0x30A4, 0x30EA, 0x30AA, 0 }; // Meiryo
    wchar_t font_MsGothic[] = { 0xFF2D,0xFF33,0x0020,0x30B4,0x30B7,0x30C3,0x30AF ,0x0 }; // MS Gothic
    if (g_customFont != "") {
        // fall back to CreateFontA
        return CreateFontA(cHeight, cWidth, cEscapement, cOrientation, cWeight, bItalic, bUnderline, bStrikeOut, iCharSet, iOutPrecision, iClipPrecision, iQuality, iPitchAndFamily, g_customFont.c_str());
    }
    // Yu Gothic
    if (wcscmp(font_YuGothic, pszFaceName) == 0)
    {
        if (std::find(fonts.begin(), fonts.end(), "Yu Gothic") != fonts.end())
            return g_realCreateFontW(cHeight, cWidth, cEscapement, cOrientation, cWeight, bItalic, bUnderline, bStrikeOut, iCharSet, iOutPrecision, iClipPrecision, iQuality, iPitchAndFamily, L"Yu Gothic");
        if (std::find(fonts.begin(), fonts.end(), "MS Gothic") != fonts.end())
            return g_realCreateFontW(cHeight, cWidth, cEscapement, cOrientation, cWeight, bItalic, bUnderline, bStrikeOut, iCharSet, iOutPrecision, iClipPrecision, iQuality, iPitchAndFamily, L"MS Gothic");
        if (std::find(fonts.begin(), fonts.end(), "MS Mincho") != fonts.end())
            return g_realCreateFontW(cHeight, cWidth, cEscapement, cOrientation, cWeight, bItalic, bUnderline, bStrikeOut, iCharSet, iOutPrecision, iClipPrecision, iQuality, iPitchAndFamily, L"MS Mincho");
    }
    // Yu Mincho
    if (wcscmp(font_YuMincho, pszFaceName) == 0)
    {
        if (std::find(fonts.begin(), fonts.end(), "Yu Mincho") != fonts.end())
            return g_realCreateFontW(cHeight, cWidth, cEscapement, cOrientation, cWeight, bItalic, bUnderline, bStrikeOut, iCharSet, iOutPrecision, iClipPrecision, iQuality, iPitchAndFamily, L"Yu Mincho");
        if (std::find(fonts.begin(), fonts.end(), "MS Mincho") != fonts.end())
            return g_realCreateFontW(cHeight, cWidth, cEscapement, cOrientation, cWeight, bItalic, bUnderline, bStrikeOut, iCharSet, iOutPrecision, iClipPrecision, iQuality, iPitchAndFamily, L"MS Mincho");
    }
    // Meiryo
    if (wcscmp(font_YuMeiryo, pszFaceName) == 0)
    {
        if (std::find(fonts.begin(), fonts.end(), "Meiryo") != fonts.end())
            return g_realCreateFontW(cHeight, cWidth, cEscapement, cOrientation, cWeight, bItalic, bUnderline, bStrikeOut, iCharSet, iOutPrecision, iClipPrecision, iQuality, iPitchAndFamily, L"Meiryo");
    }
    // MS Gothic
    if (wcscmp(font_MsGothic, pszFaceName) == 0) {
        if (std::find(fonts.begin(), fonts.end(), "MS Gothic") != fonts.end())
            return g_realCreateFontW(cHeight, cWidth, cEscapement, cOrientation, cWeight, bItalic, bUnderline, bStrikeOut, iCharSet, iOutPrecision, iClipPrecision, iQuality, iPitchAndFamily, L"MS Gothic");
        if (std::find(fonts.begin(), fonts.end(), "MS Mincho") != fonts.end())
            return g_realCreateFontW(cHeight, cWidth, cEscapement, cOrientation, cWeight, bItalic, bUnderline, bStrikeOut, iCharSet, iOutPrecision, iClipPrecision, iQuality, iPitchAndFamily, L"MS Mincho");
    }
    return g_realCreateFontW(cHeight, cWidth, cEscapement, cOrientation, cWeight, bItalic, bUnderline, bStrikeOut, iCharSet, iOutPrecision, iClipPrecision, iQuality, iPitchAndFamily, pszFaceName);
}


void HookCreateFont()
{
    THPrac::HookIAT(GetModuleHandle(NULL), "GDI32.dll", "CreateFontA", CreateFontA_Changed, (void**)&g_realCreateFontA);
    THPrac::HookIAT(GetModuleHandle(NULL), "GDI32.dll", "CreateFontW", CreateFontW_Changed, (void**)&g_realCreateFontW);
}