#include "game_init.h"
#include "ingame_ui.h"
#include "3rd/thp/thprac_hook.h"
#include "3rd/thp/thprac_gui_impl_win32.h"
#include "3rd/thp/thprac_gui_impl_dx9.h"
#include "3rd/thp/utils/utils.h"
#include "3rd/thp/utils/wininternal.h"
#include "game_locale.h"

#include <d3d9.h>

#include <format>
#include <vector>
#include <string>
#include <set>
#include "util_hooks.h"

#pragma warning(disable:4996)
#define MSG(str) MessageBoxA(NULL,std::string(str).c_str(),"",MB_OK);
#define MSGX(x) MessageBoxA(NULL,std::format("{:x}",x).c_str(),"",MB_OK);
#define DW_2(x) (*(DWORD*)(RVA2(x)))
#define I32_2(x) (*(int32_t*)(RVA2(x)))
#define VecD2_2(x) (*(vec2d*)(RVA2(x)))

#define TODO 0x114514
using namespace THPrac;

IDirect3DDevice9** g_gameGuiDevice;
HWND* g_gameGuiHwnd;
HIMC g_gameIMCCtx;

int GameGuiProgress = 0;
bool g_is_full_screen = false;
bool g_is_rep_version = false;

struct vec2d
{
    double x, y;
};

std::vector<const char*> stage_select;
std::vector<const char*> type_select_1;

std::vector<const char*> front_jmps[4];
std::vector<const char*> latter_jmps[4];
std::vector<const char*> boss_jmps[4];
std::vector<const char*> Mboss_jmps[4];

std::vector<int> boss_jmp_stages[4];
std::vector<int> boss_disable_lifebar[4];
std::vector<int> Mboss_jmp_stages[4];
std::vector<int> Mboss_disable_lifebar[4];

void AddBossJmp(int stage,const char* name,int boss_stage,int disableLifebar = 0)
{
    boss_jmps[stage].push_back(name);
    boss_jmp_stages[stage].push_back(boss_stage);
    boss_disable_lifebar[stage].push_back(disableLifebar);
}

void InitData()
{
    stage_select = { GetString(u8"stage 1"), GetString(u8"stage 2"),GetString(u8"stage 3"),GetString(u8"stage ex") };
    type_select_1 = { GetString(u8"前半"), GetString(u8"道中boss"),  GetString(u8"后半"),  GetString(u8"关底boss") };
    TODO;
    
    AddBossJmp(0, GetString(u8"一非"                                         ), 2, 2);
    AddBossJmp(0, GetString(u8"一符 嵐符 「ハードテンペスト」"               ), 3);
    AddBossJmp(0, GetString(u8"二非"                                         ), 4 ,1);
    AddBossJmp(0, GetString(u8"二符 幻想 「鏡花水月」"                       ), 5);
    AddBossJmp(0, GetString(u8"三符 外道 「魔縁の大禍」"                     ), 6, 2);

    AddBossJmp(1, GetString(u8"一非"                                         ), 2, 1);
    AddBossJmp(1, GetString(u8"一符 乾坤 「嚆矢濫觴の剣」"                   ), 3);
    AddBossJmp(1, GetString(u8"二非"                                         ), 4 ,1);
    AddBossJmp(1, GetString(u8"二符 摂理 「冥助冥加の剣」"                   ), 5);
    AddBossJmp(1, GetString(u8"三非"                                         ), 6, 1);
    AddBossJmp(1, GetString(u8"三符 天符 「空居の要石」"                     ), 7);
    AddBossJmp(1, GetString(u8"四符 「全存在の緋想天」"                      ), 8, 2);
    AddBossJmp(1, GetString(u8"五符 極光 「世界を彩る眩き光よ」"             ), 9, 2);

    AddBossJmp(2, GetString(u8"一非"                                         ), 2, 1);
    AddBossJmp(2, GetString(u8"一符 創符 「プログレッシブダート」"           ), 3);
    AddBossJmp(2, GetString(u8"二非"                                         ), 4);
    AddBossJmp(2, GetString(u8"二符 氾濫 「大太法師の足禊」"                 ), 5);
    AddBossJmp(2, GetString(u8"三非"                                         ), 6);
    AddBossJmp(2, GetString(u8"三符 幻影 「御来迎レーザー」"                 ), 7);
    AddBossJmp(2, GetString(u8"四非"                                         ), 8);
    AddBossJmp(2, GetString(u8"四符 虚像 「ブロッケンの大妖怪」"             ), 9);
    AddBossJmp(2, GetString(u8"五符 光輝 「ディライトフルレイン」"           ), 10);
    AddBossJmp(2, GetString(u8"六符 「プリズマティカルミソロジー」"          ), 11);
}

void InitGui(IDirect3DDevice9** ppDevice, HWND* phwnd)
{
    ::ImGui::CreateContext();
    g_gameGuiDevice = ppDevice;
    g_gameGuiHwnd = phwnd;
    g_gameIMCCtx = ImmAssociateContext(*phwnd, 0);
    Gui::ImplDX9Init(*g_gameGuiDevice);
    Gui::ImplWin32Init(*phwnd);

    // Hooks
    Gui::ImplDX9HookReset();
    Gui::ImplWin32HookWndProc();
    Gui::ImplDX9AdjustDispSize();
}
void InitGameInput()
{
    SetGameInputFunc([](DWORD key)->bool
        {
            TODO;
            return KeyboardInputUpdate(key) == 1;
        }
    );
}

void GameGuiBegin()
{
    Gui::ImplDX9NewFrame();
    Gui::ImplWin32NewFrame();
    ::ImGui::NewFrame();
    GameGuiProgress = 1;
}

void GameGuiEnd(bool draw_cursor = true)
{
    if (GameGuiProgress != 1)
        return;
    if (draw_cursor && Gui::ImplWin32CheckFullScreen()) {
        auto& io = ::ImGui::GetIO();
        io.MouseDrawCursor = true;
    }
    ::ImGui::EndFrame();
    GameGuiProgress = 2;
}

void GameGuiRender()
{
    if (GameGuiProgress != 2)
        return;
    Gui::ImplDX9Check((IDirect3DDevice9*)*g_gameGuiDevice);
    ::ImGui::Render();
    Gui::ImplDX9RenderDrawData(::ImGui::GetDrawData());
    GameGuiProgress = 0;
}


enum Addrs
{
    Addrs = 0,

    GameState = 0xA4C240,
    NextMenu = 0x12C28DC,
    MenuConfirm         = 0xA2E270,
    MenuConfirmKeyTime  = 0xA2E27C,


    SelectedPlType   = 0x11118F8,
    SelectedDiff     = 0x11118F4,
    EnableGame       = 0x11118E8,
    GameType         = 0x11118F0,
    dword_11A9454    = 0x11A9454,
    dword_11A96E0    = 0x11A96E0,

    dword_12C28E0 = 0x12C28E0,
    dword_11A9458 = 0x11A9458,

    Stage           = 0x11A9710,
    Diff            = 0x11A9714,
    PlType          = 0x12C23DC,

    HitCountRemain  = 0x11A96F8,
    BombPeice       = 0x11A96FC,
    Power           = 0x11A9700,
    Faith           = 0x11A9708,
    Graze           = 0x11A9704,
    Score           = 0x11A97B8,

    GameTime = 0x11A96E4,


    MBossEnterTime = 0x11A91FC,
    BossEnterTime  = 0x11A9200,


    CurBossLife             = 0x11EA744,
    TotalLife               = 0x11EA748,
    BossType                = 0x11EA764,
    LifeBarFillTime         = 0x11EA754,
    TotLifeBarFillTimeArr   = 0x75D2D4,
    BossStage               = 0x11EA75C,
    BossMovement            = 0x11EA7F8,
    BossPos                 = 0x11EA798,
};

bool IsInPrac()
{
    return I32_2(GameType) == 2;
}

enum JmpType
{
    JFront, JMBoss, JLatter, JBoss
};

struct PracParam
{
    bool mode = 0;
    int32_t stage = 0;
    int32_t life = 0;
    int32_t bomb = 0;
    int32_t power = 0;
    int32_t faith = 0;
    int32_t graze = 0;
    int32_t score = 0;

    int32_t type;
    int32_t jmp;

}pracParam;




JmpType GetJmpType(int stage, int type)
{
    JmpType normal[] = { JFront,JMBoss,JLatter,JBoss };
    return normal[type];
}

// void InsertLatterJmpData(int stage, const char* name, int time)
// {
//     latter_jmps[stage].push_back((const char*)name);
//     latter_time[stage].push_back(time);
// }
// void InsertFrontJmpData(int stage, const char* name, int time)
// {
//     front_jmps[stage].push_back((const char*)name);
//     front_time[stage].push_back(time);
// }
// 
// void InsertBossJmpData(int stage, const char* name, int state, bool createbs = true)
// {
//     boss_jmps[stage].push_back((const char*)name);
//     boss_jmps_boss_state[stage].push_back(state);
//     boss_need_create[stage].push_back(createbs);
// }
// 
// void InsertMBossJmpData(int stage, const char* name, int state, bool createbs = true)
// {
//     Mboss_jmps[stage].push_back((const char*)name);
//     Mboss_jmps_boss_state[stage].push_back(state);
//     Mboss_need_create[stage].push_back(createbs);
// }


class THOverlay : public GameGuiWnd {
    THOverlay() noexcept
    {
        SetTitle("Mod Menu");
        SetFade(0.1f, 0.5f);
        SetPos(10.0f, 10.0f);
        SetSize(0.0f, 0.0f);
        SetWndFlag(
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | 0);
        OnLocaleChange();
    }
    SINGLETON(THOverlay);
public:

protected:
    virtual void OnLocaleChange() override
    {
        float x_offset_1 = 0.0f;
        float x_offset_2 = 0.0f;
        x_offset_1 = 0.1f;
        x_offset_2 = 0.14f;

        mMenu.SetTextOffsetRel(x_offset_1, x_offset_2);
        mMuteki.SetTextOffsetRel(x_offset_1, x_offset_2);
        mDisableX.SetTextOffsetRel(x_offset_1, x_offset_2);
    }
    virtual void OnContentUpdate() override
    {
        ImGui::Text("gametime: %d", I32_2(0x11A96E4));
        mMuteki();
        mDisableX();
    }
    virtual void OnPreUpdate() override
    {
        if (mMenu(false) && !ImGui::IsAnyItemActive()) {
            if (*mMenu) {
                Open();
            }
            else {
                Close();
            }
        }
    }

    GuiHotKey mMenu{ "ModMenuToggle", "BACKSPACE", VK_BACK };

    GuiHotKey mDisableX{ GetString(u8"禁用X键"), "F2", VK_F2 };

    HOTKEY_DEFINE(mMuteki, GetString(u8"无敌"), "F1", VK_F1)
    PATCH_HK(0x357A0, "00")
    HOTKEY_ENDDEF();
    // HOTKEY_DEFINE(mDisableX, GetString(u8"禁用X键"), "F2", VK_F2)
    // EHOOK_HK(0x0, 1, {
    // 
    //     })
    // HOTKEY_ENDDEF();
    
public:
};



class PracUI : public GameGuiWnd
{
    PracUI() noexcept
    {
        SetFade(0.8f, 0.1f);
        SetStyle(ImGuiStyleVar_WindowRounding, 0.0f);
        SetStyle(ImGuiStyleVar_WindowBorderSize, 0.0f);
        SetTitle("prac");
        SetSizeRel(0.65f, 0.7f);
        SetPosRel(0.2f, 0.18f);
        // SetSizeRel(0.65f, 0.81f);
        // SetPosRel(0.20f, 0.1f);
        SetItemWidthRel(-0.1f);
        SetAutoSpacing(true);

        *mStage = 0;
        *mLife = 8;
        *mBomb = 16;
        *mPower = 400;
        *mGraze = 0;
        *mScore = 0;
        *mJmpSelect = 0;
        *mType = 0;
        *mFaith = 50000;

        *mNavFocus = 0;
    }
    SINGLETON(PracUI)
private:
    GuiCombo mStage{ "stage",stage_select };
    GuiCombo mType{ "type",type_select_1 };
    GuiCombo mJmpSelect{ "jmp",std::vector<const char*>{""} };


    GuiDrag<int32_t, ImGuiDataType_S32> mLife{ "life",0,8 };
    GuiDrag<int32_t, ImGuiDataType_S32> mBomb{ "bomb(peice)",0,16 };
    GuiDrag<int32_t, ImGuiDataType_S32> mPower{ "power",0,400 };
    GuiDrag<int32_t, ImGuiDataType_S32> mGraze{ "graze",0,2147483647 };
    GuiDrag<int32_t, ImGuiDataType_S32> mScore{ "score",0,2147483647 };
    GuiDrag<int32_t, ImGuiDataType_S32> mFaith{ "faith",0,9999990 };

    GuiNavFocus mNavFocus{ "stage","type","jmp","life","bomb","power","faith","graze","score" };

public:
    void FillParam()
    {
        pracParam.mode = true;
        pracParam.life = *mLife;
        pracParam.bomb = *mBomb;
        pracParam.power = *mPower;
        pracParam.faith = *mFaith;

        pracParam.graze = *mGraze;
        pracParam.score = *mScore;

        pracParam.stage = *mStage;
        pracParam.type = *mType;
        pracParam.jmp = *mJmpSelect;
    }

    void PracticeMenu()
    {
        bool sel_changed = false;
        sel_changed = mStage();
        if (sel_changed) {
            mType.SetItems(type_select_1);
        }
        sel_changed |= mType();

        if (sel_changed)
        {
            switch (GetJmpType(*mStage, *mType))
            {
            default:
            case JFront:
                mJmpSelect.SetItems(front_jmps[*mStage]);
                break;
            case JLatter:
                mJmpSelect.SetItems(latter_jmps[*mStage]);
                break;
            case JMBoss:
                mJmpSelect.SetItems(Mboss_jmps[*mStage]);
                break;
            case JBoss://boss
                mJmpSelect.SetItems(boss_jmps[*mStage]);
                break;
            }
        }
        mJmpSelect();

        mLife();
        mBomb();
        mPower();
        mFaith();
        mGraze();
        mScore();

        mNavFocus();
    }
    virtual void OnContentUpdate() override
    {
        PracticeMenu();
    }
};

void PracEnterStage()
{
    I32_2(dword_11A9454) = 1;
    I32_2(EnableGame)   = 1;
    memset((void*)(RVA2(dword_11A96E0)), 0, 0x218);
    if (pracParam.stage == 3) {
        I32_2(Diff) = 4;
    } else {
        I32_2(Diff) = I32_2(SelectedDiff);
    }
    I32_2(PlType) = I32_2(SelectedPlType);

    I32_2(Stage)     = pracParam.stage;
    
}

void RenderUpdate()
{
    PracUI::singleton().Update();
    THOverlay::singleton().Update();
}

HOOKSET_DEFINE(Prac)
EHOOK_DY(Boss_Jmp2, 0x458D6, 8,
    {
         if (IsInPrac()) {
           if ((GetJmpType(pracParam.stage, pracParam.type) == JBoss
               && I32_2(BossStage) == 2 && pracParam.stage != 3
               && boss_jmp_stages[pracParam.stage][pracParam.jmp] != 2)
               ||
               (GetJmpType(pracParam.stage, pracParam.type) == JBoss
                   && I32_2(BossStage) == 4
                   && boss_jmp_stages[pracParam.stage][pracParam.jmp] != 4) // ExBoss
               )
           {
                I32_2(BossStage) = pCtx->Ecx = boss_jmp_stages[pracParam.stage][pracParam.jmp];
                pCtx->Eip = RVA2(0x445933);
            }
            if (GetJmpType(pracParam.stage, pracParam.type) == JMBoss
                && I32_2(BossStage) == 0 && Mboss_jmp_stages[pracParam.stage][pracParam.jmp] != 0
            ) {
                I32_2(BossStage) = pCtx->Ecx = Mboss_jmp_stages[pracParam.stage][pracParam.jmp];
                pCtx->Eip = RVA2(0x445933);
            }
        }
    }
)
EHOOK_DY(Boss_Bgm, 0x356F1, 1,
    {
        if (IsInPrac()) {
            if (GetJmpType(pracParam.stage, pracParam.type) == JBoss )
            {
                pCtx->Edi = 1;
            }
        }
    }
)
EHOOK_DY(Boss_Jmp, 0x45627, 1,
    {
        if (IsInPrac()) {
            if ((GetJmpType(pracParam.stage, pracParam.type) == JBoss
                && I32_2(BossStage) == 2 && pracParam.stage != 3
                && boss_jmp_stages[pracParam.stage][pracParam.jmp] != 2)
                || 
                (GetJmpType(pracParam.stage, pracParam.type) == JBoss
                 && I32_2(BossStage) == 4 
                    && boss_jmp_stages[pracParam.stage][pracParam.jmp] != 4) // ExBoss
                )
            {
                I32_2(BossMovement) = 0;
                VecD2_2(BossPos) = {192.0,119.95832824707};
                asm_call_rel<0x457E0, Stdcall>();
            }

            if (GetJmpType(pracParam.stage, pracParam.type) == JMBoss
                && I32_2(BossStage) == 0 && Mboss_jmp_stages[pracParam.stage][pracParam.jmp]!=0
            )
            {
                I32_2(BossMovement) = 0;
                VecD2_2(BossPos) = { 192.0,119.95832824707 };
                asm_call_rel<0x457E0, Stdcall>();
            }
        }
    }
)
EHOOK_DY(Boss_LifeBar_Disable, 0x45683, 7,
    {
         if (IsInPrac()) {
            if ((GetJmpType(pracParam.stage, pracParam.type) == JBoss
                && boss_disable_lifebar[pracParam.stage][pracParam.jmp] == 1) ||
                (GetJmpType(pracParam.stage, pracParam.type) == JMBoss
               && Mboss_disable_lifebar[pracParam.stage][pracParam.jmp] == 1))
            {
                I32_2(BossMovement) = 0;
                VecD2_2(BossPos) = { 192.0,119.95832824707 };
                int32_t tot_time = I32_2(TotLifeBarFillTimeArr + 4 * I32_2(BossType));
                I32_2(LifeBarFillTime) = tot_time;
                I32_2(CurBossLife) = I32_2(TotalLife);
            }
            else if ((GetJmpType(pracParam.stage, pracParam.type) == JBoss
                && boss_disable_lifebar[pracParam.stage][pracParam.jmp] == 2) ||
                (GetJmpType(pracParam.stage, pracParam.type) == JMBoss
                    && Mboss_disable_lifebar[pracParam.stage][pracParam.jmp] == 2))
            {
                I32_2(BossMovement) = 0;
                VecD2_2(BossPos) = { 192.0,119.95832824707 };
                int32_t cur_time = I32_2(LifeBarFillTime);
                int32_t tot_time = I32_2(TotLifeBarFillTimeArr + 4 * I32_2(BossType));
                if (cur_time < tot_time - 30){
                    I32_2(LifeBarFillTime) = tot_time - 30;
                }
            }
    }
    }
)
EHOOK_DY(UI_Prac_Render, 0x50E11, 5,
    {
        pCtx->Eip = RVA(0x5101D);
    }
)
EHOOK_DY(Prac_Retry, 0x539A9, 5,
{
    if (IsInPrac())
    {
        PracEnterStage();
        pCtx->Eip= RVA2(0x453971);
    }
})
EHOOK_DY(UI_Prac_Logic, 0x550B7, 1,
    {
        if (I32_2(GameState) == 23)
        {
            PracUI::singleton().Open();
            if (I32_2(MenuConfirmKeyTime) == 1)
            {
                if (I32_2(MenuConfirm) == 1)
                {
                    PracUI::singleton().FillParam();
                    PracEnterStage();
                    I32_2(GameState) = 49;
                    PracUI::singleton().Close();
                }
                else if (I32_2(MenuConfirm) == -1)// last menu
                {
                    DW_2(dword_11A9458) = 1;
                    I32_2(GameState) = 22;
                    PracUI::singleton().Close();
                    pracParam.mode = false;//disable
                }
            }
            DW_2(dword_12C28E0) = 0;
            pCtx->Eip = RVA(0x551DB);
        }
    }
)
EHOOK_DY(Prac_Param_Set, 0x32C2E, 7,
{
    I32_2(HitCountRemain) = pracParam.life + 1;
    I32_2(BombPeice) = pracParam.bomb;
    I32_2(Power) = pracParam.power;
    I32_2(Faith) = pracParam.faith;
    I32_2(Graze) = pracParam.graze;
    I32_2(Score) = pracParam.score;

    switch (GetJmpType(pracParam.stage,pracParam.type)) {
    case JBoss:
        I32_2(GameTime) = I32_2(BossEnterTime) - 10;
        break;
    case JMBoss:
        I32_2(GameTime) = I32_2(MBossEnterTime) - 10;
        break;
    }
})
HOOKSET_ENDDEF()

HOOKSET_DEFINE(Hook_Update)
EHOOK_DY(GameGuiRenderUpdate, 0xAA71D, 6, {
    GameGuiBegin();
    RenderUpdate();
    GameGuiEnd();
    GameGuiRender();
    })
HOOKSET_ENDDEF()


void InitAll()
{
    InitGui((IDirect3DDevice9**)(RVA2(0x918794)), (HWND*)RVA2(0x76F964));

    InitData();
    PracUI::singleton();
    THOverlay::singleton();

    InitFont();
    // InitGameInput();
}

HOOKSET_DEFINE(Hook_Init)
// EHOOK_DY(WindowSz, TODO, 1, {
//     if (pCtx->Edx == '4')
//     {
//         *(double*)(pCtx->Ebp - 0x30) = 3.0;
//         pCtx->Eip = 0x65307D;
//     }
// })
 EHOOK_DY(init_1, 0xAA49F, 1, {
     self->Disable();
     InitAll();
     EnableAllHooks(Hook_Update);
     EnableAllHooks(Prac);
})
HOOKSET_ENDDEF()

void InitGame()
{
    ingame_image_base = CurrentImageBase;
    HookCreateFont();
    EnableAllHooks(Hook_Init);
}