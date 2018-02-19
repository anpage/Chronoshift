/**
 * @file
 *
 * @author OmniBlade
 * @author CCHyper
 *
 * @brief Class containing information pertaining to a single map cell.
 *
 * @copyright Redalert++ is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "cell.h"
#include "ccfileclass.h"
#include "coord.h"
#include "drawshape.h"
#include "globals.h"
#include "iomap.h"
#include "mixfile.h"
#include "rules.h"
#include "scenario.h"
#include "session.h"
#include "target.h"
#include "theater.h"
#include "tileset.h"
#include "tracker.h"

CellClass::CellClass() :
    CellNumber(Map.Cell_Number(this)),
    Bit1(false),
    PlacementCheck(false),
    Bit4(false),
    Bit8(false),
    Bit16(false),
    Bit32(false),
    HasFlag(false),
    Bit128(false),
    field_A(HOUSES_COUNT),
    CellTag(),
    Template(TEMPLATE_NONE),
    Icon(0),
    Overlay(OVERLAY_NONE),
    OverlayFrame(-1),
    Smudge(SMUDGE_NONE),
    SmudgeFrame(-1),
    OwnerHouse(HOUSES_NONE),
    field_18(HOUSES_NONE),
    OccupierPtr(nullptr),
    OccupantBit(0),
    Land(LAND_CLEAR)
{
    for (int i = 0; i < MZONE_COUNT; ++i) {
        field_6[i] = 0;
    }

    for (int i = 0; i < ARRAY_SIZE(Overlapper); ++i) {
        Overlapper[i] = nullptr;
    }
}

/**
 * @brief Gets the color of the cell based on what it contains and the type of terrain.
 *
 * 0x0049EEF8
 */
int CellClass::Cell_Color(BOOL none) const
{
    // TODO Needs BuildingClass
#ifndef RAPP_STANDALONE
    int (*func)(const CellClass *, BOOL) = reinterpret_cast<int (*)(const CellClass *, BOOL)>(0x0049EEF8);
    return func(this, none);
#elif 0
    DEBUG_ASSERT(CellNumber < MAP_MAX_AREA);

    static const int _ground_color[9] = { 141, 141, 172, 21, 21, 158, 141, 141, 174 };
    static const int _snow_color[9] = { 141, 141, 172, 21, 21, 158, 141, 141, 174 };

    int color;
    BuildingClass *bptr = Cell_Building();

    if (bptr == nullptr || bptr->Class_Of().IsInvisible) {
        if (none) {
            color = 0;
        } else {
            switch (g_lastTheater) {
                case THEATER_SNOW:
                    color = _snow_color[Land];
                    break;

                default: // All others use this one.
                    color = _ground_color[Land];
                    break;
            }
        }

        return color;
    }

    return ColorRemaps[HouseClass::As_Reference(bptr->Owner()).Color].WindowPalette[7];
#else
    return 0;
#endif
}

/**
 * @brief Returns a pointer to a TechnoClass object in this cell that can be found closest the given pixel coordinates.
 *
 * 0x0049EFBC
 */
TechnoClass *CellClass::Cell_Techno(int x, int y) const
{
    DEBUG_ASSERT(CellNumber < MAP_MAX_AREA);

    uint32_t coord = Coord_From_Pixel_XY(x, y);
    ObjectClass *object = nullptr;

    for (ObjectClass *obj = OccupierPtr; obj != nullptr; obj = obj->Get_Next()) {
        uint32_t sdistance = 0;

        if (obj->Is_Techno()) {
            uint32_t obj_coord = obj->Center_Coord();
            uint32_t distance = Distance(obj_coord &= 0x00FF00FF,
                coord); // TODO what is going on with obj_coord?!  is it fetching sub cell or cell xy?

            if (object == nullptr || (signed int)distance < (signed int)sdistance) {
                sdistance = distance;
                object = obj;
            }
        }
    }

    return reinterpret_cast<TechnoClass *>(object);
}

/**
 * @brief Checks occupiers to see if any are the requested type and returns a pointer to the first one found.
 *
 * 0x0049F084
 */
ObjectClass *CellClass::Cell_Find_Object(RTTIType type) const
{
    DEBUG_ASSERT(CellNumber < MAP_MAX_AREA);

    for (ObjectClass *object = OccupierPtr; object != nullptr; object = object->Get_Next()) {
        if (object->What_Am_I() == type) {
            return object;
        }
    }

    return nullptr;
}

/**
 * @brief Checks occupiers to see if any are a building and returns a pointer to the first one found.
 *
 * 0x0049F0B0
 */
BuildingClass *CellClass::Cell_Building() const
{
    return reinterpret_cast<BuildingClass *>(Cell_Find_Object(RTTI_BUILDING));
}

/**
 * @brief Checks occupiers to see if any are infantry and returns a pointer to the first one found.
 *
 * 0x0049F5E4
 */
InfantryClass *CellClass::Cell_Infantry() const
{
    return reinterpret_cast<InfantryClass *>(Cell_Find_Object(RTTI_INFANTRY));
}

/**
 * @brief Checks occupiers to see if any are units and returns a pointer to the first one found.
 *
 * 0x0049F5BC
 */
UnitClass *CellClass::Cell_Unit() const
{
    return reinterpret_cast<UnitClass *>(Cell_Find_Object(RTTI_UNIT));
}

/**
 * @brief Checks occupiers to see if any are vessels and returns a pointer to the first one found.
 *
 * 0x0049F5D0
 */
VesselClass *CellClass::Cell_Vessel() const
{
    return reinterpret_cast<VesselClass *>(Cell_Find_Object(RTTI_VESSEL));
}

/**
 * @brief Checks occupiers to see if any are aircraft and returns a pointer to the first one found.
 */
AircraftClass *CellClass::Cell_Aircraft() const
{
    return reinterpret_cast<AircraftClass *>(Cell_Find_Object(RTTI_AIRCRAFT));
}

/**
 * @brief Checks occupiers to see if any are terrain objects and returns a pointer to the first one found.
 *
 * 0x0049F0C4
 */
TerrainClass *CellClass::Cell_Terrain() const
{
    return reinterpret_cast<TerrainClass *>(Cell_Find_Object(RTTI_TERRAIN));
}

/**
 * @brief Checks occupiers to see if any are techno or terrain objects and returns a pointer to the first one found.
 *
 * 0x0049F0D8
 */
ObjectClass *CellClass::Cell_Object(int x, int y) const
{
    ObjectClass *object = (ObjectClass *)Cell_Aircraft();

    if (object == nullptr) {
        object = (ObjectClass *)Cell_Techno(x, y);

        if (object == nullptr) {
            object = (ObjectClass *)Cell_Terrain();
        }
    }

    return object;
}

/**
 * @brief Recalculates the LandType of this cell for movement and pathfind calculations based on current terrain and overlay.
 *
 * 0x0049F314
 */
void CellClass::Recalc_Attributes()
{
    if (g_lastTheater != THEATER_INTERIOR || (Template != TEMPLATE_NONE && Template >= TEMPLATE_FIRST)) {
        if (Overlay == OVERLAY_NONE || (Land = OverlayTypeClass::As_Reference(Overlay).Get_Land(), Land == LAND_CLEAR)) {
            // TODO Bug TEMPLATE_ARRO0003 can only be land type LAND_CLEAR because it is 0xFF, the value that was
            // TEMPLATE_NONE in the TD and SS map formats due to lower number of templates and using uint8_t to hold the
            // value.
            if (Template == TEMPLATE_NONE || Template == 0xFF) {
                Land = LAND_CLEAR;
            } else {
                Land = TemplateTypeClass::As_Reference(Template).Land_Type(Icon);
            }
        }

    } else {
        Land = LAND_ROCK;
    }
}

/**
 * @brief Checks if Ore can grow in this cell.
 *
 * 0x004A1C4C
 */
BOOL CellClass::Can_Ore_Grow() const
{
    DEBUG_ASSERT(CellNumber < MAP_MAX_AREA);

    if (Rule.Ore_Grows()) {
        if (Session.Game_To_Play() == GAME_CAMPAIGN || Session.MPlayer_Ore_Growth()) {
            if (Land == LAND_ORE && OverlayFrame < ORESTAGE_FULLGROWN) { // see OreStageEnum?
                return Contains_Ore();
            }
        }
    }

    return false;
}

/**
 * @brief Checks if Ore can spread from this cell to neighbours.
 *
 * 0x004A1CCC
 */
BOOL CellClass::Can_Ore_Spread() const
{
    DEBUG_ASSERT(CellNumber < MAP_MAX_AREA);

    if (Rule.Ore_Spreads()) {
        if (Session.Game_To_Play() == GAME_CAMPAIGN || Session.MPlayer_Ore_Growth()) {
            if (Land == LAND_ORE && OverlayFrame > ORESTAGE_SPREADING) { // see OreStageEnum?
                return Contains_Ore();
            }
        }
    }

    return false;
}

/**
 * @brief Checks if Ore can spawn in this cell.
 *
 * 0x004A1E40
 */
BOOL CellClass::Can_Ore_Germinate() const
{
    // TODO Needs BuildingClass.
#ifndef RAPP_STANDALONE
    BOOL (*func)(const CellClass *) = reinterpret_cast<BOOL (*)(const CellClass *)>(0x004A1E40);
    return func(this);
#elif 0
    DEBUG_ASSERT(CellNumber < MAP_MAX_AREA);

    if (Map.In_Radar(CellNumber)) {
        if (Is_Bridge_Here()) {
            return false;
        }

        BuildingClass *bptr = Cell_Building();

        // IsInvisible needs double checking
        // TS has this checking for "InvisibleInGame", that internally toggles;
        // IsInvisible to true;
        // IsRadarVisible to false;
        // in Read_INI() (TS of course).

        if (bptr == nullptr || bptr->Class_Of().IsInvisible) {
            if (GroundClass::Ground[Land].Is_Buildable()) {
                return Overlay == OVERLAY_NONE;
            }
        }
    }

    return false;
#else
    return false;
#endif
}

/**
 * @brief Performs Ore growth logic in this cell.
 *
 * 0x004A1D4C
 */
BOOL CellClass::Grow_Ore()
{
    DEBUG_ASSERT(CellNumber < MAP_MAX_AREA);

    if (Can_Ore_Grow()) {
        ++OverlayFrame;
        Redraw_Objects();

        return true;
    }

    return false;
}

/**
 * @brief Performs Ore spread logic to neighbouring cells.
 *
 * 0x004A1D7C
 */
BOOL CellClass::Spread_Ore(BOOL force)
{
    // TODO Requires OverlayClass.
#ifndef RAPP_STANDALONE
    BOOL (*func)(const CellClass *, BOOL) = reinterpret_cast<BOOL (*)(const CellClass *, BOOL)>(0x004A1D7C);
    return func(this, force);
#elif 0
    if (force || Can_Ore_Spread()) {
        // Pick a random direction to start in.
        FacingType start_dir = (FacingType)Scen.Get_Random_Value(FACING_FIRST, FACING_COUNT - 1);

        for (FacingType facing = FACING_FIRST; facing < FACING_COUNT; ++facing) {
            // Check each facing starting with our random one to see if Ore can be spawned there.
            const CellClass &cell = Adjacent_Cell((FacingType)(unsigned(start_dir + facing) % FACING_COUNT));

            if (cell.Can_Ore_Germinate()) {
                OverlayType overlay = (OverlayType)Scen.Get_Random_Value(OVERLAY_GOLD_01, OVERLAY_GOLD_04);
                OverlayClass *optr = new OverlayClass(overlay, cell.CellNumber, OwnerHouse);
                DEBUG_ASSERT(optr != nullptr);
                OverlayFrame = 0;

                return true;
            }
        }
    }

    return false;
#else
    return false;
#endif
}

/**
 * @brief Gets a reference to the cell that is adjacent on the given facing.
 *
 * 0x004A01EC
 */
CellClass &CellClass::Adjacent_Cell(FacingType facing)
{
    DEBUG_ASSERT(CellNumber < MAP_MAX_AREA);
    DEBUG_ASSERT(facing < FACING_COUNT);

    static const int16_t AdjacentCell[FACING_COUNT] = {
        (-MAP_MAX_WIDTH), // NORTH
        (-MAP_MAX_WIDTH + 1), // NORTH EAST
        (1), // EAST
        (MAP_MAX_WIDTH + 1), // SOUTH EAST
        (MAP_MAX_WIDTH), // SOUTH
        (MAP_MAX_WIDTH - 1), // SOUTH WEST
        (-1), // WEST
        (-MAP_MAX_WIDTH - 1) // NORTH WEST
    };

    if (facing != FACING_NONE && facing < FACING_COUNT) {
        DEBUG_ASSERT_PRINT(uint16_t(CellNumber + AdjacentCell[facing]) <= MAP_MAX_AREA,
            "Attempting to get adjacent cell outside valid map.\n");

        // This logic relies on map cells being held in a contiguous area of memory, which in MapClass they are.
        if (uint16_t(CellNumber + AdjacentCell[facing]) <= MAP_MAX_AREA) {
            return this[AdjacentCell[facing]];
        }
    }

    return *this;
}

/**
 * @brief Checks if this cell is part of a bridge.
 *
 * 0x004A1BE8
 */
BOOL CellClass::Is_Bridge_Here() const
{
    switch (Template) {
        case TEMPLATE_BRIDGE1: // Intentional fall through
        case TEMPLATE_BRIDGE1D:
        case TEMPLATE_BRIDGE2:
        case TEMPLATE_BRIDGE2D:
        case TEMPLATE_BRIDGE1A:
        case TEMPLATE_BRIDGE1B:
        case TEMPLATE_BRIDGE1C:
        case TEMPLATE_BRIDGE2A:
        case TEMPLATE_BRIDGE2B:
        case TEMPLATE_BRIDGE2C:
        case TEMPLATE_BRIDGE3A:
        case TEMPLATE_BRIDGE3B:
        case TEMPLATE_BRIDGE3C:
        case TEMPLATE_BRIDGE3D:
        case TEMPLATE_BRIDGE3E:
        case TEMPLATE_BRIDGE3F:
            return true;

        default:
            break;
    }

    return false;
}

/**
 * @brief Marks any objects in this cell to be redrawn at the next call to redraw the map.
 *
 * 0x0049F10C
 */
void CellClass::Redraw_Objects(BOOL force)
{
    // TODO Requires DisplayClass and RadarClass layers of IOMap hierachy.
#ifndef RAPP_STANDALONE
    void (*func)(const CellClass *, BOOL) = reinterpret_cast<void (*)(const CellClass *, BOOL)>(0x0049F10C);
    func(this, force);
#elif 0
    DEBUG_ASSERT(CellNumber < MAP_MAX_AREA);

    // Check if we are even visible first, then check if already flagged for a redraw.
    if (Map.In_View(CellNumber)) {
        if (force || !Map.Is_Cell_Flagged(CellNumber)) {
            Map.Flag_Cell(CellNumber);

            // Loop through occupiers and if active mark them for redraw.
            for (ObjectClass *optr = OccupierPtr; optr != nullptr; optr = optr->Get_Next()) {
                DEBUG_ASSERT(optr->Is_Active());
                optr->Mark(MARK_REDRAW);
            }

            // Loop through overlappers and if active mark them for redraw.
            for (int j = 0; j < ARRAY_SIZE(Overlapper); ++j) {
                if (Overlapper[j] != nullptr) {
                    DEBUG_ASSERT(Overlapper[j]->Is_Active());
                    Overlapper[j]->Mark(MARK_REDRAW);
                }
            }
        }
    }
#endif
}

/**
 * @brief Check if this cell can be built upon.
 *
 * 0x0049F1FC
 */
BOOL CellClass::Is_Clear_To_Build(SpeedType speed) const
{
    DEBUG_ASSERT(CellNumber < MAP_MAX_AREA);

    if (ScenarioInit) {
        return true;
    }

    if (Cell_Object(1, 0) != nullptr) {
        return false;
    }

    if (HasFlag) {
        return false;
    }

    if (!g_inMapEditor
        || Overlay != OVERLAY_NONE && OverlayFrame != -1
            && (Overlay == OVERLAY_FPLS || OverlayTypeClass::As_Reference(Overlay).Is_Wall())) {
        return false;
    }

    if (Smudge != SMUDGE_NONE && SmudgeFrame != -1 && SmudgeTypeClass::As_Reference(Smudge).Is_Bib()) {
        return false;
    }

    if (speed != SPEED_NONE) {
        if (Ground[Land].Get_Speed(speed) != fixed::_0_1) {
            return false;
        }

        return true;
    }

    if (Is_Bridge_Here()) {
        return false;
    }

    return Ground[Land].Is_Buildable();
}

/**
 * @brief Move an object down the occupy list for this cell.
 *
 * 0x0049F394
 */
void CellClass::Occupy_Down(ObjectClass *object)
{
    // TODO Requires RadarClass layer of IOMap hierachy.
#ifndef RAPP_STANDALONE
    void (*func)(const CellClass *, ObjectClass *) =
        reinterpret_cast<void (*)(const CellClass *, ObjectClass *)>(0x0049F394);
    func(this, object);
#elif 0
    DEBUG_ASSERT(CellNumber < MAP_MAX_AREA);
    DEBUG_ASSERT(object->Is_Active());

    if (object != nullptr) {
        // If object is a building, and already have a cell occupier, find end of occupier chanin and stick object on the
        // end. Else stick object on the front.
        if (object->What_Am_I() == RTTI_BUILDING && OccupierPtr != nullptr) {
            SmartPtr<ObjectClass> *next_ptr;

            for (ObjectClass *next = OccupierPtr; next != nullptr; next = next->Get_Next()) {
                next_ptr = &next->Get_Next();
            }

            *next_ptr = object;
            object->Get_Next() = nullptr;
        } else {
            object->Get_Next() = OccupierPtr;
            OccupierPtr = object;
        }

        // Dunno what this does yet.
        Map.Radar_Pixel(CellNumber);

        if (Bit4 && Session.Game_To_Play() != GAME_CAMPAIGN) {
            object->Revealed(PlayerPtr);
        }

        // Set OccupantBit based on type of object.
        switch (object->What_Am_I()) {
            case RTTI_BUILDING:
                OccupantBit |= OCCUPANT_BUILDING;
                break;

            case RTTI_AIRCRAFT: // Fall through
            case RTTI_UNIT: // Fall through
            case RTTI_VESSEL:
                OccupantBit |= OCCUPANT_UNIT;
                break;

            case RTTI_TERRAIN:
                OccupantBit |= OCCUPANT_TERRAIN;
                break;

            default:
                break;
        }
    }
#endif
}

/**
 * @brief Move an object up the occupy list for this cell.
 *
 * 0x0049F450
 */
void CellClass::Occupy_Up(ObjectClass *object)
{
    // TODO Requires RadarClass layer of IOMap hierachy.
#ifndef RAPP_STANDALONE
    void (*func)(const CellClass *, ObjectClass *) =
        reinterpret_cast<void (*)(const CellClass *, ObjectClass *)>(0x0049F450);
    func(this, object);
#elif 0
    DEBUG_ASSERT(CellNumber < MAP_MAX_AREA);
    DEBUG_ASSERT(object->Is_Active());

    if (object != nullptr) {
        if (OccupierPtr == object) {
            OccupierPtr = object->Get_Next();
            object->Next = nullptr;
        } else {
            bool found = false;

            for (ObjectClass *next = OccupierPtr; next != nullptr; next = next->Get_Next()) {
                SmartPtr<ObjectClass> &next_ptr = next->Get_Next();

                if (object == next) {
                    next_ptr = object->Get_Next();
                    object->Get_Next() = nullptr;
                    found = true;
                    break;
                }
            }
        }

        // Dunno what this does yet. Updates radar pixel color for this cell?
        Map.Radar_Pixel(CellNumber);

        // Clear OccupantBit based on type of object.
        switch (object->What_Am_I()) {
            case RTTI_BUILDING:
                OccupantBit &= ~OCCUPANT_BUILDING;
                break;

            case RTTI_AIRCRAFT: // Fall through
            case RTTI_UNIT: // Fall through
            case RTTI_VESSEL:
                OccupantBit &= ~OCCUPANT_UNIT;
                break;

            case RTTI_TERRAIN:
                OccupantBit &= ~OCCUPANT_TERRAIN;
                break;

            default:
                break;
        }
    }
#endif
}

/**
 * @brief Move an object down the overlap list for this cell.
 *
 * 0x0049F4E4
 */
void CellClass::Overlap_Down(ObjectClass *object)
{
    // TODO Requires HouseClass.
#ifndef RAPP_STANDALONE
    void (*func)(const CellClass *, ObjectClass *) =
        reinterpret_cast<void (*)(const CellClass *, ObjectClass *)>(0x0049F4E4);
    func(this, object);
#elif 0
    DEBUG_ASSERT(CellNumber < MAP_MAX_AREA);
    DEBUG_ASSERT(object->Is_Active());

    if (object == nullptr) {
        return;
    }

    ObjectClass **overlap_slot = nullptr;

    // Go through our list over overlappers, return if object is already in otherwise get pointer to last free index.
    for (int i = 0; i < ARRAY_SIZE(Overlapper); ++i) {
        if (Overlapper[i] == object) {
            return;
        }

        if (Overlapper[i] == nullptr) {
            overlap_slot = &Overlapper[i];
        }
    }

    // If object is a building and it didn't get a free slot, loop through and try and find a slot that isn't already a
    // building or terrain and use that one instead.
    if (overlap_slot == nullptr && object->What_Am_I() == RTTI_BUILDING) {
        for (int i = 0; i < ARRAY_SIZE(Overlapper); ++i) {
            if (Overlapper[i]->What_Am_I() != RTTI_BUILDING && Overlapper[i]->What_Am_I() != RTTI_TERRAIN) {
                Overlapper[i] = object;
                break;
            }
        }
    }

    // If we got a slot, just use it.
    if (overlap_slot != nullptr) {
        *overlap_slot = object;
    }

    // Reveal object to the player?
    if (Bit4) {
        object->Revealed(PlayerPtr);
    }
#endif
}

/**
 * @brief Move an object up the overlap list for this cell.
 *
 * 0x0049F590
 */
void CellClass::Overlap_Up(ObjectClass *object)
{
    // TODO Requires RadarClass layer of IOMap hierachy.
#ifndef RAPP_STANDALONE
    void (*func)(const CellClass *, ObjectClass *) =
        reinterpret_cast<void (*)(const CellClass *, ObjectClass *)>(0x0049F590);
    func(this, object);
#elif 0
    DEBUG_ASSERT(CellNumber < MAP_MAX_AREA);
    DEBUG_ASSERT(object->Is_Active());

    for (int i = 0; i < ARRAY_SIZE(Overlapper); ++i) {
        if (Overlapper[i] == object) {
            Overlapper[i] = nullptr;
            break;
        }
    }

    if (g_inMapEditor) {
        Map.Radar_Pixel(CellNumber);
    }
#endif
}

/**
 * @brief Returns the tile in a clear set to use for this cell number to give impression of randomness.
 *
 * 0x004A010C
 */
int CellClass::Clear_Icon() const
{
    DEBUG_ASSERT(CellNumber < MAP_MAX_AREA);
    return ((CellNumber >> 5) & 0xC) | (CellNumber & 0x3);
}

/**
 * @brief Draws the cell and some of its contents.
 *
 * 0x0049F5F8
 */
void CellClass::Draw_It(int x, int y, BOOL unk_bool) const
{
    // TODO Requires RadarClass layer of IOMap hierachy.
#ifndef RAPP_STANDALONE
    void (*func)(const CellClass *, int, int, BOOL) =
        reinterpret_cast<void (*)(const CellClass *, int, int, BOOL)>(0x0049F5F8);
    func(this, x, y, unk_bool);
#elif 0
    DEBUG_ASSERT(CellNumber < MAP_MAX_AREA);

    int icon_num;
    void *fading_table = nullptr;
    ObjectClass *obj = nullptr;
    TemplateTypeClass *tt;

    GraphicViewPortClass dbgcell(g_logicPage->Get_Graphic_Buffer(),
        g_logicPage->Get_XPos() + WindowList[WINDOW_TACTICAL].X,
        g_logicPage->Get_YPos() + WindowList[WINDOW_TACTICAL].Y,
        WindowList[WINDOW_TACTICAL].W,
        WindowList[WINDOW_TACTICAL].H);

    if (!unk_bool) {
        //++CellCount; // Original has a global variable here. Not sure what this is used for.

        // TODO
        // TEMPLATE_ARRO0003 is tile 255, but that value is treated as clear in many vanilla maps, even interior ones so it
        // cannot be used. Possible solution is to handle this at map load and if no 0xFFFF values found, convert any 0xFF to
        // 0xFFFF and only handle 0xFFFF here?
        if (Template != TEMPLATE_NONE && Template != 255 && Template >= TEMPLATE_FIRST && Template < TEMPLATE_COUNT) {
            tt = &TemplateTypeClass::As_Reference(Template);
            icon_num = Icon;
        } else {
            tt = &TemplateTypeClass::As_Reference(TEMPLATE_CLEAR);
            icon_num = Clear_Icon();
        }

#if 0 // TODO Editor related stuff.
        if ((g_inMapEditor && Map.field_8567) /*|| Debug_CellPassable*/) {
            if (!Ground[Land].Is_Buildable() || Ground[Land].Get_Speed(SPEED_FOOT) == fixed::_0_1 && obj != nullptr) {
                fading_table = DisplayClass::FadingRed;
            }
        }
    
        obj = Cell_Occupier();

        if ((g_inMapEditor
                && obj != nullptr /*&& obj->Get_Next() != nullptr*/) /*|| Debug_CellOccupied*/) {
            if (Occupied_By_Foot()) {
                fading_table = DisplayClass::FadingYellow;
            }
        }
#endif

        if (tt->Get_Image_Data() != nullptr) {
            GraphicViewPortClass remap(g_logicPage->Get_Graphic_Buffer(),
                g_logicPage->Get_XPos() + WindowList[WINDOW_TACTICAL].X,
                g_logicPage->Get_YPos() + WindowList[WINDOW_TACTICAL].Y,
                WindowList[WINDOW_TACTICAL].W,
                WindowList[WINDOW_TACTICAL].H);

            g_logicPage->Draw_Stamp(tt->Get_Image_Data(),
                icon_num,
                x,
                y,
                nullptr,
                WindowList[WINDOW_TACTICAL].X,
                WindowList[WINDOW_TACTICAL].Y,
                WindowList[WINDOW_TACTICAL].W,
                WindowList[WINDOW_TACTICAL].H);
        }

        if (Smudge != SMUDGE_NONE && SmudgeFrame != -1) {
            SmudgeTypeClass::As_Reference(Smudge).Draw_It(x, y, SmudgeFrame);
        }

        if (Overlay != OVERLAY_NONE && OverlayFrame != -1) {
            OverlayTypeClass::As_Reference(Overlay).Draw_It(x, y, OverlayFrame);
        }

        if (PlacementCheck) {
            SpeedType speed = SPEED_NONE;

            if (Map.PendingObjTypePtr != nullptr && Map.PendingObjTypePtr->What_Am_I() == RTTI_BUILDING) {
                speed = ((BuildingTypeClass *)Map.PendingObjTypePtr)->Speed;
            }

            int placement_icon = PLACEMENT_CLEAR;

            // TODO, handle object in cell, do yellow tile
            if (Map.PassedProximityCheck && Cell_Techno() != nullptr) {
                placement_icon = PLACEMENT_YELLOW;
            } else if (!Map.PassedProximityCheck && !Is_Clear_To_Build(speed)) {
                placement_icon = PLACEMENT_RED;
            }

            g_logicPage->Draw_Stamp(DisplayClass::TransIconset,
                placement_icon,
                x,
                y,
                nullptr,
                WindowList[WINDOW_TACTICAL].X,
                WindowList[WINDOW_TACTICAL].Y,
                WindowList[WINDOW_TACTICAL].W,
                WindowList[WINDOW_TACTICAL].H);
#if 0 // TODO Edwin map stuff.
            if (g_inMapEditor) {
                if (Map.PendingObjTypePtr != nullptr) {
                    switch (Map.PendingObjTypePtr->What_Am_I()) {
                        case RTTI_VESSEL:
                            // TODO

                        case RTTI_BUILDING:
                            // TODO

                        case RTTI_UNIT:
                            // TODO

                        case RTTI_INFANTRY:
                            // TODO

                        case RTTI_AIRCRAFT:
                            // TODO
                        case RTTI_OVERLAYTYPE:
                            break;
                        case RTTI_SMUDGETYPE:
                            break;
                        case RTTI_TEMPLATETYPE:
                            break;

                        default:
                            break;
                    }
                }
            }
        }
#endif

            if (HasFlag) {
                int flag_x;
                int flag_y;
                int flag_frame;
                void *flag_shape;

                flag_shape = MixFileClass<CCFileClass>::Retrieve("FLAGFLY.SHP");

                // 'flag_frame' will be the number of frames in the shape sequence, so it draws it based on what frame the
                // game is on, wrapped so it goes 0 to 13. So when game frame is 14 it will return 0, 15 will return 1 and so
                // on.
                flag_frame = g_frame % Get_Build_Frame_Count(flag_shape); // shape has 14 frames

                // 12 is what is set, but the shape is 23, so im gonna take it as / 2. or is this centered to a icon w/h? (24
                // sq)
                flag_x = x + (Get_Build_Frame_Width(flag_shape) / 2) + 1;
                flag_y = y + (Get_Build_Frame_Height(flag_shape) / 2) + 1;

                CC_Draw_Shape(
                    flag_shape, // MixFileClass<CCFileClass>::Retrieve("FLAGFLY.SHP"), //no point called retrieve again.
                    flag_frame,
                    flag_x,
                    flag_y,
                    WINDOW_TACTICAL,
                    SHAPE_SHADOW | SHAPE_FADING | SHAPE_CENTER,
                    HouseClass::As_Pointer(OwnerHouse)->Remap_Table(false, REMAP_1),
                    DisplayClass::UnitShadow);
            }
        }
#endif
}

void CellClass::Concrete_Calc()
{
    DEBUG_ASSERT(CellNumber < MAP_MAX_AREA);

    static FacingType _even[] = {
        FACING_NORTH, FACING_SOUTH, FACING_SOUTH_WEST, FACING_WEST, FACING_NORTH_WEST, FACING_NORTH
    };

    static FacingType _odd[] = {
        FACING_NORTH, FACING_NORTH_EAST, FACING_EAST, FACING_SOUTH_EAST, FACING_SOUTH, FACING_NORTH
    };

    // TODO Function exists in RA, but just returns, C&C has full code though. Is logic unique to C&C or just elsewhere in
    // RA? Could be left over from concrete placement in Dune2 which was also in early versions of C&C.
}

void CellClass::Wall_Update()
{
    DEBUG_ASSERT(CellNumber < MAP_MAX_AREA);

    static FacingType _offsets[] = { FACING_NORTH, FACING_EAST, FACING_SOUTH, FACING_WEST, FACING_NONE };

    for (int index = 0; index < ARRAY_SIZE(_offsets); ++index) {
        CellClass &adjcell = Adjacent_Cell(_offsets[index]);

        if (adjcell.Overlay != OVERLAY_NONE && OverlayTypeClass::As_Reference(adjcell.Overlay).Is_Wall()) {
            int neighbour_mask = 0;

            // Identify neighbour cells that are the same overlay.
            for (int j = 0; j < ARRAY_SIZE(_offsets) - 1; ++j) {
                CellClass &neighbour = adjcell.Adjacent_Cell(_offsets[j]);

                if (neighbour.Overlay == adjcell.Overlay) {
                    neighbour_mask |= (1 << j);
                }
            }

            // What is going on here? Okay, the & 0xF0 masks it to be either 0, 16 or 32 in practice which determines if its
            // a damaged state or not, then the | adds the frame offset based on the adjacent cells as determined by the loop
            // above.
            adjcell.OverlayFrame = (adjcell.OverlayFrame & 0xF0) | neighbour_mask;

            if (adjcell.Overlay == OVERLAY_BRICK_WALL && adjcell.OverlayFrame == 48) {
                adjcell.OverlayFrame = -1;
                adjcell.Overlay = OVERLAY_NONE;
                adjcell.OwnerHouse = HOUSES_NONE; // C&C DOS sets this, bug fix perhaps?
                Detach_This_From_All(As_Target(CellNumber), 1);
            }

            if (adjcell.Overlay == OVERLAY_SANDBAG && adjcell.OverlayFrame == 16) {
                adjcell.OverlayFrame = -1;
                adjcell.Overlay = OVERLAY_NONE;
                adjcell.OwnerHouse = HOUSES_NONE;
                Detach_This_From_All(As_Target(CellNumber), 1);
            }

            if (adjcell.Overlay == OVERLAY_CYCLONE_FENCE && OverlayFrame == 32) {
                adjcell.OverlayFrame = -1;
                adjcell.Overlay = OVERLAY_NONE;
                adjcell.OwnerHouse = HOUSES_NONE;
                Detach_This_From_All(As_Target(CellNumber), 1);
            }

            if (adjcell.Overlay == OVERLAY_FENC && (adjcell.OverlayFrame == 16 || adjcell.OverlayFrame == 32)) {
                adjcell.OverlayFrame = -1;
                adjcell.Overlay = OVERLAY_NONE;
                adjcell.OwnerHouse = HOUSES_NONE;
                Detach_This_From_All(As_Target(CellNumber), 1);
            }

            if (adjcell.Overlay == OVERLAY_BARB_WIRE && adjcell.OverlayFrame == 16) {
                adjcell.OverlayFrame = -1;
                adjcell.Overlay = OVERLAY_NONE;
                adjcell.OwnerHouse = HOUSES_NONE;
                Detach_This_From_All(As_Target(CellNumber), 1);
            }

            adjcell.Recalc_Attributes();
            adjcell.Redraw_Objects(false);
        }
    }
}
