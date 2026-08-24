// Horseback -- mod for The Elder Scrolls V: Skyrim that extends
// horseback riding features.
// Copyright (C) 2023-2026  usernameak
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 3 of the License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "HorseInCellsModule.hpp"

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

#include "PO3Helpers.hpp"

// 39116 = AIProcess::ComputeLastTimeProcessed
// 37818 = Actor::StopMovement
// 19799 = TESObjectREFR::MoveRefToNewSpace

// 38705 = Actor::Func38705
// 37170 = Actor::Func37170
// 37905 = Actor::Mount
// 38706 = Character::ValidateInteraction
// 40438 = PlayerCharacter::UpdateCellTransitions
// 40621 = PlayerCharacter::UpdateCrosshairPickText
//          + 0xA3 = IsOnMount call
// 17922 = TESObjectDOOR::Activate
//          + 0x125 = IsOnMount call

static bool Fake_IsOnMount(RE::Actor *actor);
static REL::Relocation<decltype(Fake_IsOnMount)> UpdateCrosshairPickText_IsOnMount_Trampoline;
static REL::Relocation<decltype(Fake_IsOnMount)> TESObjectDOOR_Activate_IsOnMount_Trampoline;

static bool Fake_IsOnMount(RE::Actor *) {
    return false;
}

static void AIProcess_ComputeLastTimeProcessed(RE::AIProcess *self) {
    using func_t = decltype(&AIProcess_ComputeLastTimeProcessed);
    REL::Relocation<func_t> func{ RELOCATION_ID(38158, 39116) };
    return func(self);
}

static void Actor_StopMovement(RE::Actor *actor) {
    using func_t = decltype(Actor_StopMovement);
    REL::Relocation<func_t> func{ RELOCATION_ID(36802, 37818) };
    func(actor);
}

static void TESObjectREFR_MoveRefToNewSpace(RE::TESObjectREFR *self, RE::TESObjectCELL *a_interior, RE::TESWorldSpace *a_world) {
    using func_t = decltype(&TESObjectREFR_MoveRefToNewSpace);
    static REL::Relocation<func_t> func{ RELOCATION_ID(19372, 19799) };
    return func(self, a_interior, a_world);
}

static void Actor_Func38705(RE::Actor *self, uint32_t unknown) {
    using func_t = decltype(&Actor_Func38705);
    static REL::Relocation<func_t> func{ RELOCATION_ID(37760, 38705) };
    return func(self, unknown);
}

static void Actor_Func37170(RE::Actor *self) {
    using func_t = decltype(&Actor_Func37170);
    static REL::Relocation<func_t> func{ RELOCATION_ID(36191, 37170) };
    return func(self);
}

static bool Actor_Mount(RE::Actor *self, RE::Actor *other) {
    using func_t = decltype(&Actor_Mount);
    static REL::Relocation<func_t> func{ RELOCATION_ID(36881, 37905) };
    return func(self, other);
}

thread_local bool validateInteractionHack = false;

static void ReattachMount(RE::PlayerCharacter *player, RE::NiPointer<RE::Actor> &mount, const RE::PLAYER_TARGET_LOC *targetLoc, bool &oldRespawnFlag) {
    if (!player->GetMount(mount)) {
        return;
    }

    // seems to reset the mounting state?
    Actor_Func38705(player, 2);
    Actor_Func37170(player);

    // set new mount position
    mount->SetPosition(targetLoc->location, true);

    // move the mount to the new worldspace
    TESObjectREFR_MoveRefToNewSpace(mount.get(), targetLoc->interior, targetLoc->world);

    // this should only be done in exterior cells, if at all
    if (!targetLoc->interior) {
        mount->SetParentCell(nullptr);
    }

    // reset AI
    mount->EvaluatePackage(false, true);
    RE::AIProcess *aiProc = mount->GetActorRuntimeData().currentProcess;
    if (aiProc) {
        Actor_StopMovement(mount.get());
        mount->StopCombat();
        mount->EndInterruptPackage(false);
        AIProcess_ComputeLastTimeProcessed(aiProc);
    }

    // the hell is this?
    static_cast<RE::MovementControllerAI *>(mount->GetActorRuntimeData().movementController.get())->Unk_06();

    // otherwise the horse teleports back to its original spawn point when switching world spaces!
    oldRespawnFlag = static_cast<bool>(mount->GetActorBase()->actorData.actorBaseFlags & RE::ACTOR_BASE_DATA::Flag::kRespawn);
    mount->GetActorBase()->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kRespawn);

    // remount the player!
    validateInteractionHack = true;
    Actor_Mount(player, mount.get());
    validateInteractionHack = false;
}

struct Character_ValidateInteraction_Hook {
    static bool thunk(RE::Character *source, RE::Character *target) {
        if (validateInteractionHack) {
            return true;
        }
        return func(source, target);
    }
    static inline REL::Relocation<decltype(thunk)> func;

    static void Install() {
        REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(37761, 38706) };
        po3helpers::HookFunctionPrologue<Character_ValidateInteraction_Hook, 5>(target.address());
    }
};

struct PlayerCharacter_UpdateCellTransitions_Hook {
    static void thunk(RE::PlayerCharacter *player) {
        RE::NiPointer<RE::Actor> mount;
        bool oldRespawnFlag = true;

        const RE::PLAYER_TARGET_LOC &loc = player->GetPlayerRuntimeData().queuedTargetLoc;
        if (loc.isValid && !loc.fastTravelMarker) {
            ReattachMount(player, mount, &loc, oldRespawnFlag);
        }

        func(player);

        // restore the kRespawn flag. this sucks!
        if (mount) {
            auto &baseFlags = mount->GetActorBase()->actorData.actorBaseFlags;
            if (oldRespawnFlag) {
                baseFlags.set(RE::ACTOR_BASE_DATA::Flag::kRespawn);
            }
        }
    }
    static inline REL::Relocation<decltype(thunk)> func;

    static void Install() {
        REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(39366, 40438) };
        po3helpers::HookFunctionPrologue<PlayerCharacter_UpdateCellTransitions_Hook, 5>(target.address());
    }
};

hback::HorseInCellsModule::HorseInCellsModule() = default;

void hback::HorseInCellsModule::loadConfig(const cpptoml::table &table) {
    Module::loadConfig(table);
}

size_t hback::HorseInCellsModule::getTrampolineSize() const {
    return 14 * 2 + 64;
}

void hback::HorseInCellsModule::dataLoaded() {
    // 1.6.1179 and 1.6.640 seem to match
    auto updateCrosshairPickTextHookAddr         = RELOCATION_ID(39535, 40621).address() + 0xA3;
    UpdateCrosshairPickText_IsOnMount_Trampoline = SKSE::GetTrampoline().write_call<5>(
        updateCrosshairPickTextHookAddr,
        reinterpret_cast<uintptr_t>(&Fake_IsOnMount));

    // 1.6.1179 and 1.6.640 seem to match
    auto doorActHookAddr                        = RELOCATION_ID(17521, 17922).address() + 0x125;
    TESObjectDOOR_Activate_IsOnMount_Trampoline = SKSE::GetTrampoline().write_call<5>(
        doorActHookAddr,
        reinterpret_cast<uintptr_t>(&Fake_IsOnMount));

    PlayerCharacter_UpdateCellTransitions_Hook::Install();
    Character_ValidateInteraction_Hook::Install();
}
