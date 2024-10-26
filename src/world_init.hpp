#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"

// These are hardcoded to the dimensions of the entity texture
// BB = bounding box
const float FISH_BB_WIDTH  = 0.6f * 165.f;
const float FISH_BB_HEIGHT = 0.6f * 165.f;
const float EEL_BB_WIDTH   = 0.6f * 300.f;	// 1001
const float EEL_BB_HEIGHT  = 0.6f * 202.f;	// 870
const float WHIRL_BB_WIDTH = 0.6f * 200.f;	// 1001
const float WHIRL_BB_HEIGHT = 0.6f * 200.f;	// 1001
const size_t WHIRLPOOL_DEATH_TIMER = 18000.f;

const float FISH_SPEED = 50.f;

// the player
Entity createSalmon(RenderSystem* renderer, vec2 pos);

// the prey
Entity createFish(RenderSystem* renderer, vec2 position);

// the enemy
Entity createEel(RenderSystem* renderer, vec2 position);

// another prey
Entity createPuffer(RenderSystem* renderer, vec2 position, float rand, float rand2);

// the other enemy weird enemy thing
Entity createWhirlpool(RenderSystem* renderer, vec2 position, float rand);

// a red line for debugging purposes
Entity createLine(vec2 position, vec2 size);

// create a bounding box for an object
void createTrackerLines(Entity entity);

// calculate the AABB of an object
vec4 calculate_AABB(Object& obj);

