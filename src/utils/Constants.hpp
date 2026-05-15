#pragma once

namespace Constants
{
    constexpr const char* StartString = "Start";
    constexpr const char* IdleDispatcherString = "IdleDispatcher";
    constexpr const char* CheatString = "Cheat";
    constexpr const char* MatchString = "Match";
    constexpr const char* GameObjectClassString = "GameObjectClass";

    constexpr const char* CLASS_RECYCLERVEHICLE = "CLASS_RECYCLERVEHICLE";
    constexpr const char* CLASS_RECYCLERVEHICLEH = "CLASS_RECYCLERVEHICLEH";
    constexpr const char* CLASS_RECYCLER = "CLASS_RECYCLER";
    constexpr const char* CLASS_FACTORY = "CLASS_FACTORY";
    constexpr const char* CLASS_ARMORY = "CLASS_ARMORY";
    constexpr const char* CLASS_CONSTRUCTIONRIG = "CLASS_CONSTRUCTIONRIG";
    constexpr const char* CLASS_CONSTRUCTIONRIGT = "CLASS_CONSTRUCTIONRIGT";

    constexpr int MAX_BUILD_SLOTS = 9;
    constexpr int MAX_BUILD_GROUPS = 10; // Usually 9 + 1 so the loops can grab the right headers before going through each build group.
    constexpr int MAX_PLAN_CONDITIONS = 16;
    constexpr int MAX_UNITS_PER_TEAM = 128;
    constexpr int MAX_PLAN_NAME_LENGTH = 128;
    constexpr int MAX_PLANS = 1024;
    constexpr int MAX_POSSIBLE = 1024;
    constexpr int MAX_RESPONSES = 8;
    constexpr int MAX_STAGE_POINTS = 32;
    constexpr int MAX_MATCHES = 99;
    constexpr int IGNORE_POWER = -9999;

    // Unsure what the MAX_WORLDS const is from context. Perhaps 3 given what Ken explained with lockstep.
    // For now, MAX_WORLDS has been set to 3. We'll adjust as needed if something breaks.
    constexpr int MAX_WORLDS = 3;

    constexpr int WORLD_LOCKSTEP = 0;

    constexpr float BIRTH_TIME_IDLE = 10.0f;
    constexpr float SAFE_SCRAP_DISTANCE = 150.0f * 150.0f;
    constexpr float IdleDistSq = 8.0f * 8.0f;
    constexpr float BaseDistSq = 75.0f * 75.0f;

    constexpr float MIN_KEEPATTACK_AMMO = 0.02f;
    constexpr float MIN_SEND_AMMO = 0.10f;
    constexpr float MIN_BUILD_AMMO = 0.02f;
    constexpr float MIN_ATTACK_AMMO = 0.10f;

    constexpr float MIN_WRECK_AMMO = 0.10f;
    constexpr float MIN_WRECK_HEALTH = 0.25f;
    constexpr float MAX_WRECK_AMMO = 0.10f;
    constexpr float MAX_WRECK_HEALTH = 0.25f;
    constexpr float MAX_REPAIR_TIME = 20.0f;
    constexpr float MAX_WRECK_DIST = 30.0f;

    constexpr float OLD_ENEMY_SEARCH_RADIUS = 400.0f;

    constexpr float MAX_FLOAT = 3.4028e+38f;

    constexpr float MAX_HOLD_TIME = 20.0f;
    constexpr float MIN_HOLD_DIST = 10.0f;
    constexpr float MAX_HOLD_DIST = 100.0f;
}