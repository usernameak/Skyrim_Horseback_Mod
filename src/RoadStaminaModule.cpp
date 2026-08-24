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

#include "RoadStaminaModule.hpp"

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

// 19839 = TESObjectREFR::GetRelevantWaterHeight
// 30635 = Pathing::GetCharacterLocation
// 401037 = Pathing::st_instance
// 90392 = FindTriangleForLocationWaterFilter::FindTriangleForLocationWaterFilter
// 90367 = BSNavmeshLocationInfo::FindTriangleForLocation

static float TESObjectREFR_GetRelevantWaterHeight(RE::TESObjectREFR *self) {
    using func_t = decltype(&TESObjectREFR_GetRelevantWaterHeight);
    REL::Relocation<func_t> func{ RELOCATION_ID(19411, 19839) };
    return func(self);
}

struct BSNavmeshInfo;

struct BSNavmeshLocationInfo {
    uint32_t field_0;
    uint32_t field_4;
    uint32_t field_8;
    uint32_t field_C;
    BSNavmeshInfo *m_navmeshInfo;
    uint64_t field_18;
    uint64_t field_20;
    uint32_t m_triangleIndex;
    uint32_t field_2C;
};

static_assert(sizeof(BSNavmeshLocationInfo) == 0x30);

struct BSNavmeshInfo {
    int field_0;
    int field_4;
    int field_8;
    int field_C;
    int field_10;
    int field_14;
    int field_18;
    int field_1C;
    int field_20;
    int field_24;
    RE::BSNavmesh *m_navmesh;
};

struct Pathing;

static void Pathing_GetCharacterLocation(Pathing *self, BSNavmeshLocationInfo *info, RE::TESObjectREFR *refr) {
    using func_t = decltype(&Pathing_GetCharacterLocation);
    REL::Relocation<func_t> func{ RELOCATION_ID(29819, 30635) };
    func(self, info, refr);
}

struct FindTriangleForLocationWaterFilter {
    void *vftable;
    int field_8;
    int field_C;
    float m_waterHeight;
};

static FindTriangleForLocationWaterFilter *
FindTriangleForLocationWaterFilter_ctor(FindTriangleForLocationWaterFilter *self, float waterHeight) {
    using func_t = decltype(&FindTriangleForLocationWaterFilter_ctor);
    REL::Relocation<func_t> func{ RELOCATION_ID(88005, 90392) };
    return func(self, waterHeight);
}

static bool
BSNavmeshLocationInfo_FindTriangleForLocation(BSNavmeshLocationInfo *self, FindTriangleForLocationWaterFilter *filter) {
    using func_t = decltype(&BSNavmeshLocationInfo_FindTriangleForLocation);
    REL::Relocation<func_t> func{ RELOCATION_ID(87983, 90367) };
    return func(self, filter);
}


// ---- //

static hback::RoadStaminaModule *st_roadStaminaModule;

hback::RoadStaminaModule::RoadStaminaModule()
    : m_staminaConsumptionRateOnRoad(-0.4f) {
    setEnabled(true);

    st_roadStaminaModule = this;
}

void hback::RoadStaminaModule::loadConfig(const cpptoml::table &table) {
    Module::loadConfig(table);

    m_staminaConsumptionRateOnRoad = static_cast<float>(table.get_as<double>("staminaConsumptionRateOnRoad").value_or(m_staminaConsumptionRateOnRoad));
}

size_t hback::RoadStaminaModule::getTrampolineSize() const {
    return 14;
}

static void DamageActorValue(RE::Actor *actor, RE::ACTOR_VALUE_MODIFIER avModifier, RE::ActorValue actorValue, float value);
static REL::Relocation<decltype(DamageActorValue)> DamageActorValue_Trampoline;

static void DamageActorValue(RE::Actor *actor, RE::ACTOR_VALUE_MODIFIER avModifier, RE::ActorValue actorValue, float value) {
    RE::PlayerCharacter *player = RE::PlayerCharacter::GetSingleton();
    if (player &&
        value < 0.0f &&
        actorValue == RE::ActorValue::kStamina &&
        avModifier == RE::ACTOR_VALUE_MODIFIER::kDamage) {
        RE::ActorPtr horseActor = nullptr;
        if (player->GetMount(horseActor) && actor == horseActor.get()) {
            // found a horse (or not a horse? todo: needs check)

            RE::TESObjectREFR *horseReference = horseActor->AsReference();
            float relevantWaterHeight         = TESObjectREFR_GetRelevantWaterHeight(horseReference);

            REL::Relocation<Pathing *> pathing{ RELOCATION_ID(514893, 401037) };

            BSNavmeshLocationInfo navmeshInfo{};
            Pathing_GetCharacterLocation(pathing.get(), &navmeshInfo, horseReference);

            FindTriangleForLocationWaterFilter filter{};
            FindTriangleForLocationWaterFilter_ctor(&filter, relevantWaterHeight);

            if (BSNavmeshLocationInfo_FindTriangleForLocation(&navmeshInfo, &filter) &&
                navmeshInfo.m_navmeshInfo &&
                navmeshInfo.m_triangleIndex < 0xFFFF) {
                if (auto *navmesh = navmeshInfo.m_navmeshInfo->m_navmesh) {
                    bool isPreferred = navmesh->triangles[navmeshInfo.m_triangleIndex]
                                           .triangleFlags.all(RE::BSNavmeshTriangle::TriangleFlag::kPreferred);
                    if (isPreferred) {
                        value = value * st_roadStaminaModule->getStaminaConsumptionRateOnRoad();
                    }
                }
            }
        }
    }
    DamageActorValue_Trampoline(actor, avModifier, actorValue, value);
}

void hback::RoadStaminaModule::dataLoaded() {
    auto damageAvHookAddr       = RELOCATION_ID(37522, 38467).address() + 0x14;
    DamageActorValue_Trampoline = SKSE::GetTrampoline().write_call<5>(damageAvHookAddr, reinterpret_cast<uintptr_t>(&DamageActorValue));
}
