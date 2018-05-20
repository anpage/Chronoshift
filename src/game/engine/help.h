/**
 * @file
 *
 * @author CCHyper
 * @author OmniBlade
 *
 * @brief Part of IOMap stack handling help tooltips.
 *
 * @copyright Redalert++ is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#ifndef HELP_H
#define HELP_H

#include "always.h"
#include "tab.h"

class HelpClass : public TabClass
{
public:
    HelpClass();

    virtual void Init_Clear() override;
    virtual void AI(KeyNumType &key, int mouse_x, int mouse_y) override;
    virtual void Draw_It(BOOL force_redraw) override;
    virtual void Help_Text(int str_id, int x = -1, int y = -1, int color = 14, BOOL on_wait = false) override;
    virtual BOOL Scroll_Map(DirType dir, int &distance, BOOL redraw = true) override;
    virtual void Set_Tactical_Position(uint32_t location) override;

    int16_t *const Overlap_List(void) const;
    void Set_Text(TextEnum string_id);
    void Set_Cost(int cost);
    void Set_Text_Color(unsigned char color);

#ifndef RAPP_STANDALONE
    static void Hook_Me();
#endif

protected:
    int field_1620;
    int field_1624;
    int field_1628;
#ifndef RAPP_NO_BITFIELDS
    // Union/Struct required to get correct packing when compiler packing set to 1.
    union
    {
        struct
        {
            bool HelpBit1 : 1; // 1
        };
        int Bitfield;
    };
#else
    bool HelpBit1;
#endif
    int HelpCost;
    int HelpMouseXPos;
    int HelpMouseYPos;
    int32_t HelpXPos;
    int32_t HelpYPos;
    int HelpWidth;
    int HelpTextID; // TextEnum is but setting it offsets alignment
    int HelpTextColor; // should just be unsigned chat but it offsets alignment
    TCountDownTimerClass<SystemTimerClass> CountDownTimer;

private:
    static int16_t OverlapList[60];
    static char *HelpText;
};

#ifndef RAPP_STANALONE
#include "hooker.h"
inline void HelpClass::Hook_Me()
{
#ifdef COMPILER_WATCOM
    // Hook_Function(0x, *HelpClass::Init_Clear);
#endif
}
#endif

#endif // HELP_H
