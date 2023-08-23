#include <RE/Skyrim.h>
#include <REL/Relocation.h>
#include <SKSE/SKSE.h>
#include <RE/P/PlayerCharacter.h>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>

SKSEPluginInfo(.Version   = REL::Version{ 1, 0, 0, 1 },
    .Name                 = "Horseback",
    .Author               = "usernameak",
    .SupportEmail         = "usernameak@protonmail.com",
    .StructCompatibility  = SKSE::StructCompatibility::Independent,
    .RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary);

void InitLogger() {
    auto path = SKSE::log::log_directory();
    if (!path)
        return;

    auto plugin = SKSE::PluginDeclaration::GetSingleton();
    *path /= fmt::format(FMT_STRING("{}.log"), plugin->GetName());

    std::shared_ptr<spdlog::sinks::sink> sink;
    if (SKSE::WinAPI::IsDebuggerPresent()) {
        sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
    } else {
        sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
    }

    auto log = std::make_shared<spdlog::logger>("global log", sink);
    log->set_level(spdlog::level::info);
    log->flush_on(spdlog::level::info);

    spdlog::set_default_logger(std::move(log));
    spdlog::set_pattern("%s(%#): [%^%l%$] %v");
}

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

static void DamageActorValue(RE::Actor *actor, RE::ACTOR_VALUE_MODIFIER avModifier, RE::ActorValue actorValue, float value);
static REL::Relocation<decltype(DamageActorValue)> DamageActorValue_Trampoline;

static void DamageActorValue(RE::Actor *actor, RE::ACTOR_VALUE_MODIFIER avModifier, RE::ActorValue actorValue, float value) {
    RE::PlayerCharacter *player = RE::PlayerCharacter::GetSingleton();
    if (player &&
        value < 0.0f &&
        actorValue == RE::ActorValue::kStamina &&
        avModifier == RE::ACTOR_VALUE_MODIFIER::kDamage) {
        RE::ActorPtr horseActor = nullptr;
        if (player->GetMount(horseActor)) {
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
                        value = -value * 0.4f;
                    }
                }
            }
        }
    }
    DamageActorValue_Trampoline(actor, avModifier, actorValue, value);
}

static void SetupHooks() {
    auto hook_addr = REL::RelocationID{ 37522, 38467 }.address() + 0x14;
    SKSE::AllocTrampoline(14);
    DamageActorValue_Trampoline = SKSE::GetTrampoline().write_call<5>(hook_addr, (uintptr_t)&DamageActorValue);
}

static void SKSEMessageHandler(SKSE::MessagingInterface::Message *message) {
    if (message->type == SKSE::MessagingInterface::kDataLoaded) {
        SetupHooks();
    }
}

extern "C" __declspec(dllexport) bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface *skse) {
    InitLogger();

    SKSE::Init(skse);

    auto *msgIfc = SKSE::GetMessagingInterface();
    msgIfc->RegisterListener(&SKSEMessageHandler);

    SKSE::log::info("Horseback loaded");

    return true;
}
