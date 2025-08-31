#include <cstdint>
#include <set>
#include <imgui.h>
#include "game_locale.h"

std::set<unsigned short> charSet;
char* GetString(const char8_t* u8char)
{
    auto getlen = [](const char8_t* str) {
        if ((*str & 0xF0) == 0xF0) return 4;
        if ((*str & 0xE0) == 0xE0) return 3;
        if ((*str & 0xC0) == 0xC0) return 2;
        if ((*str & 0x80) == 0x0) return 1;
        return -1;
        };
    auto getUChar = [](const char8_t* uch, int len) -> uint16_t
        {
            uint16_t s = 0;
            switch (len)
            {
            case 1:
                s = uch[0];
                break;
            case 2:
            {
                uint8_t a = uch[0] & 0x1F;
                uint8_t b = uch[1] & 0x3F;
                s = (a << 6) | b;
                break;
            }
            case 3:
            {
                uint8_t a = uch[0] & 0xF;
                uint8_t b = uch[1] & 0x3F;
                uint8_t c = uch[2] & 0x3F;
                s = (a << 12) | (b << 6) | c;
                break;
            }
            default:
                break;
            }
            return s;
        };
    auto strlength = strlen((char*)u8char);
    for (int i = 0; i < strlength && u8char[i] != 0;)
    {
        int len = getlen(u8char + i);
        if (len == -1)
        {
            break;
        }
        else
        {
            int u = getUChar(u8char + i, len);
            if (u != 0)
                charSet.insert(u);
        }
        i += len;
    }
RET:
    return (char*)u8char;
}


const ImWchar* GetGlyphRangesUsed()
{
    auto UnpackIntoRanges = [](std::set<uint16_t> charset, ImWchar* out_ranges)
        {
            for (auto& ch : charset)
            {
                out_ranges[0] = out_ranges[1] = ch;
                out_ranges += 2;
            }
            out_ranges[0] = 0;
        };
    static const ImWchar base_ranges[] = // not zero-terminated
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        // 0x2000, 0x206F, // General Punctuation
        // 0x3000, 0x30FF, // CJK Symbols and Punctuations, Hiragana, Katakana
        // 0x31F0, 0x31FF, // Katakana Phonetic Extensions
        // 0xFF00, 0xFFEF  // Half-width characters
    };
    static ImWchar* full_ranges = nullptr;
    if (full_ranges == nullptr)
    {
        int sz = IM_ARRAYSIZE(base_ranges) + charSet.size() * 2 + 1;
        full_ranges = new ImWchar[sz];
        memset(full_ranges, 0, sz * sizeof(ImWchar));
    }
    memcpy(full_ranges, base_ranges, sizeof(base_ranges));
    UnpackIntoRanges(charSet, full_ranges + IM_ARRAYSIZE(base_ranges));
    return &full_ranges[0];
}

void InitFont()
{
    auto& io = ImGui::GetIO();
    ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\SimHei.ttf", 18.0f, nullptr, GetGlyphRangesUsed());
}
