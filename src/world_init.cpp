#include "world_init.hpp"
#include "tiny_ecs_registry.hpp"


Entity createSalmon(RenderSystem* renderer, vec2 pos)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SALMON);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = M_PI;
	motion.input_velocity = { 0.f, 0.f };
	motion.scale = mesh.original_size * 300.f;
	motion.acceleration = { 0.f, 0.f };
	// motion.scale.x *= -1; // point front to the right
	motion.scale.y *= -1;
	// TODO A1: why was it motion.scale.y before?

	// create an empty Salmon component for our character
	registry.players.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no texture is needed
			EFFECT_ASSET_ID::SALMON,
			GEOMETRY_BUFFER_ID::SALMON });

	return entity;
}

Entity createFish(RenderSystem* renderer, vec2 position)
{
	// Reserve en entity
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.input_velocity = { -50.f, 0.f };
	motion.position = position;
	motion.acceleration = { 0.f, 0.f };

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ -FISH_BB_WIDTH, FISH_BB_HEIGHT });

	// Create an (empty) Bug component to be able to refer to all bug
	registry.eatables.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::FISH,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		});

	return entity;
}

Entity createPuffer(RenderSystem*renderer, vec2 position, float rand, float rand2) {
	printf("spawning puffer");
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.input_velocity = { 0.f, -300.f };
	motion.position = position;
	motion.acceleration = { copysignf(300.f*(rand2/2 +0.5), rand), 0.f};
	motion.initial_sign = copysignf(1.0, rand);

	motion.scale = vec2({ -FISH_BB_WIDTH, FISH_BB_HEIGHT});

	registry.eatables.emplace(entity).points = 5;

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::PUFFER,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		});
	return entity;

}

Entity createEel(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.input_velocity = { -100.f, 0.f };
	motion.position = position;
	motion.acceleration = { 0.f, 0.f };

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ -EEL_BB_WIDTH, EEL_BB_HEIGHT });

	// create an empty Eel component to be able to refer to all eels
	registry.deadlys.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::EEL,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		});

	return entity;
}

Entity createWhirlpool(RenderSystem* renderer, vec2 position, float rand) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f; // i want to slowly rotate the whirlpool
	motion.input_velocity = { 0.f, 0.f };
	motion.position = position;
	motion.acceleration = { 0.f, 0.f };

	motion.scale = vec2({ -WHIRL_BB_WIDTH*rand, WHIRL_BB_HEIGHT*rand});

	registry.deadlys.emplace(entity);
	Attractor& attractor = registry.attractors.emplace(entity);
	attractor.force *= rand;
	attractor.radius *= rand;
	registry.deathTimers.emplace(entity).counter_ms = WHIRLPOOL_DEATH_TIMER;
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::WHIRLPOOL,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		});

	return entity;

}

Entity createLine(vec2 position, vec2 scale)
{
	Entity entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	registry.renderRequests.insert(
		entity, {
			TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::EGG,
			GEOMETRY_BUFFER_ID::DEBUG_LINE
		});

	// Create motion
	Motion& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.input_velocity = { 0, 0 };
	motion.position = position;
	motion.scale = scale;
	motion.acceleration = { 0.f, 0.f };

	registry.debugComponents.emplace(entity);
	return entity;
}

Entity createEgg(vec2 pos, vec2 size)
{
	auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.input_velocity = { 0.f, 0.f };
	motion.scale = size;
	motion.acceleration = { 0.f, 0.f };

	// create an empty component for our eggs
	registry.deadlys.emplace(entity);
	registry.renderRequests.insert(
		entity, {
			TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::EGG,
			GEOMETRY_BUFFER_ID::EGG
		});

	return entity;
}