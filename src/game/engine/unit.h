/**
 * @file
 *
 * @author CCHyper
 * @author OmniBlade
 *
 * @brief 
 *
 * @copyright Chronoshift is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#ifndef UNIT_H
#define UNIT_H

#include "always.h"
#include "drive.h"
#include "unittype.h"

class UnitClass : public DriveClass
{
public:
    UnitClass(UnitType type, HousesType house);
    UnitClass(const UnitClass &that);
    UnitClass(const NoInitClass &noinit);
    virtual ~UnitClass();

    // AbstractClass
    virtual MoveType Can_Enter_Cell(cell_t cellnum, FacingType facing = FACING_NONE) const override;

    // ObjectClass
    virtual ActionType What_Action(ObjectClass *object) const override;
    virtual ActionType What_Action(cell_t cellnum) const override;
    virtual const UnitTypeClass &Class_Of() const override { return *m_Class; }
    virtual coord_t Sort_Y() const override;
    virtual BOOL Limbo() override;
    virtual BOOL Unlimbo(coord_t coord, DirType dir = DIR_NORTH) override;
    virtual const int16_t *Overlap_List(BOOL a1 = false) const override;
    virtual void Draw_It(int x_pos, int y_pos, WindowNumberType window) const override;
    virtual void Active_Click_With(ActionType action, ObjectClass *object) override;
    virtual void Active_Click_With(ActionType action, cell_t cellnum) override;
    virtual DamageResultType Take_Damage(
        int &damage, int a2, WarheadType warhead, TechnoClass *object = nullptr, BOOL a5 = false) override;
    virtual void Scatter(coord_t coord, int a2, BOOL a3 = false) override;
    virtual void Per_Cell_Process(PCPType pcp) override;
    virtual RadioMessageType Receive_Message(RadioClass *radio, RadioMessageType message, target_t &target) override;

    // MissionClass
    virtual int Mission_Guard() override;
    virtual int Mission_Guard_Area() override;
    virtual int Mission_Harvest() override;
    virtual int Mission_Hunt() override;
    virtual int Mission_Move() override;
    virtual int Mission_Unload() override;
    virtual int Mission_Repair() override;

    // TechnoClass
    virtual DirType Turret_Facing() const override;
    virtual DirType Desired_Load_Dir(ObjectClass *object, cell_t &cellnum) const override;
    virtual DirType Fire_Direction() const override;
    virtual InfantryType Crew_Type() const override;
    virtual fixed_t Ore_Load() const override;
    virtual int Pip_Count() const override;
    virtual FireErrorType Can_Fire(target_t target, WeaponSlotType weapon = WEAPON_SLOT_PRIMARY) const override;
    virtual target_t Greatest_Threat(ThreatType threat) override;
    virtual BulletClass *Fire_At(target_t target, WeaponSlotType weapon = WEAPON_SLOT_PRIMARY) override;
    virtual void Assign_Destination(target_t dest) override;
    virtual void Enter_Idle_Mode(BOOL a1 = false) override;

    // FootClass
    virtual BOOL Start_Driver(coord_t &dest) override;
    virtual BOOL Offload_Ore_Bail() override;
    virtual void Approach_Target() override;

    // DriveClass
    virtual void Overrun_Cell(cell_t cell, int a2) override;
    virtual BOOL Ok_To_Move(DirType dir) override;

    BOOL Edge_Of_World_AI();
    BOOL Flag_Attach(HousesType house);
    BOOL Flag_Remove();
    void APC_Close_Door();
    void APC_Open_Door();
    int Credit_Load();
    BOOL Ore_Check(short &cellnum, int a2, int a3);
    BOOL Goto_Ore(int scan_radius);

    UnitType What_Type() const { return m_Class->What_Type(); }

    HousesType Flag_Owner() const { return m_FlagOwner; }

#ifdef GAME_DLL
    friend void Setup_Hooks();

public:
    void Hook_Active_Click_With_Obj(ActionType action, ObjectClass *object)
    {
        UnitClass::Active_Click_With(action, object);
    }
    void Hook_Active_Click_With_Cell(ActionType action, cell_t cellnum)
    {
        UnitClass::Active_Click_With(action, cellnum);
    }
    InfantryType Hook_Crew_Type()
    {
        return UnitClass::Crew_Type();
    }
#endif

private:
    GamePtr<UnitTypeClass> m_Class;
    HousesType m_FlagOwner;
#ifndef CHRONOSHIFT_NO_BITFIELDS
    BOOL m_IsDumping : 1; // 1
    int m_Gold : 5; // 2
    int m_Gems : 5;
    BOOL m_ToScatter : 1;
#else
    bool m_IsDumping;
    int m_Gold;
    int m_Gems;
    bool m_ToScatter;
#endif
    unsigned int m_BailCount;
    unsigned int m_GapGenCellTracker;
    cell_t m_GapGenCenterCell;
    TCountDownTimerClass<FrameTimerClass> m_V2RearmDelayTimer;
    FacingClass m_TurretFacing;
};

#ifdef GAME_DLL
extern TFixedIHeapClass<UnitClass> &g_Units;
#else
extern TFixedIHeapClass<UnitClass> g_Units;
#endif

#endif // UNIT_H
