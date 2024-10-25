#include "world_init.hpp"
#include "tiny_ecs_registry.hpp"


Entity createSalmon(RenderSystem* renderer, vec2 pos)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SALMON);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	
	Object& object = registry.objects.emplace(entity);
	object.position = pos;
	object.angle = M_PI;
	object.scale = mesh.original_size * 300.f;
	object.scale.y *= -1;

	Motion& motion = registry.motions.emplace(entity);
	motion.input_velocity = { 0.f, 0.f };
	motion.acceleration = { 0.f, 0.f };

	BoundingBox& bb = registry.boundingBoxes.emplace(entity);
	vec4 bb_info = calculate_AABB(object);
	bb.bounding_box = vec2(bb_info.x, bb_info.y);
	bb.pos = vec2(bb_info.z, bb_info.w);
	createTrackerLines(entity);

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
	motion.input_velocity = { -50.f, 0.f };
	motion.acceleration = { 0.f, 0.f };

	// Setting initial values, scale is negative to make it face the opposite way
	auto& object = registry.objects.emplace(entity);
	object.position = position;
	object.angle = 0.f;
	object.scale = vec2({ -FISH_BB_WIDTH, FISH_BB_HEIGHT });

	BoundingBox& bb = registry.boundingBoxes.emplace(entity);
	vec4 bb_info = calculate_AABB(object);
	bb.bounding_box = vec2(bb_info.x, bb_info.y);
	bb.pos = vec2(bb_info.z, bb_info.w);
	createTrackerLines(entity);

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
	motion.input_velocity = { 0.f, -300.f };
	motion.acceleration = { copysignf(300.f*(rand2/2 +0.5), rand), 0.f};
	motion.initial_sign = copysignf(1.0, rand);

	auto& object = registry.objects.emplace(entity);
	object.angle = 0.f;
	object.position = position;
	object.scale = vec2({ -FISH_BB_WIDTH, FISH_BB_HEIGHT});

	BoundingBox& bb = registry.boundingBoxes.emplace(entity);
	vec4 bb_info = calculate_AABB(object);
	bb.bounding_box = vec2(bb_info.x, bb_info.y);
	bb.pos = vec2(bb_info.z, bb_info.w);
	createTrackerLines(entity);

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
	motion.acceleration = { 0.f, 0.f };
	motion.input_velocity = { -100.f, 0.f };

	// Setting initial values, scale is negative to make it face the opposite way
	auto& object = registry.objects.emplace(entity);
	object.position = position;
	object.angle = 0.f;
	object.scale = vec2({ -EEL_BB_WIDTH, EEL_BB_HEIGHT });

	BoundingBox& bb = registry.boundingBoxes.emplace(entity);
	vec4 bb_info = calculate_AABB(object);
	bb.bounding_box = vec2(bb_info.x, bb_info.y);
	bb.pos = vec2(bb_info.z, bb_info.w);
	printf("created bounding box with position %f %f and size %f %f\n", bb.pos.x, bb.pos.y, bb.bounding_box.x, bb.bounding_box.y);
	printf("for reference, the object position is %f %f and scale is %f %f\n", object.position.x, object.position.y, object.scale.x, object.scale.y);
	createTrackerLines(entity);
	
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
	motion.input_velocity = { 0.f, 0.f };
	motion.acceleration = { 0.f, 0.f };

	auto& object = registry.objects.emplace(entity);
	object.angle = 0.f; // i want to slowly rotate the whirlpool
	object.position = position;
	object.scale = vec2({ -WHIRL_BB_WIDTH * rand, WHIRL_BB_HEIGHT * rand });

	BoundingBox& bb = registry.boundingBoxes.emplace(entity);
	vec4 bb_info = calculate_AABB(object);
	bb.bounding_box = vec2(bb_info.x, bb_info.y);
	bb.pos = vec2(bb_info.z, bb_info.w);
	createTrackerLines(entity);

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
	Object& object = registry.objects.emplace(entity);
	object.angle = 0.f;
	object.position = position;
	object.scale = scale;

	// registry.debugComponents.emplace(entity);
	return entity;
}

void createTrackerLines(Entity entity) {
	printf("creating tracker lines ");
	BoundingBox bb = registry.boundingBoxes.get(entity);
	printf("for bounding box with position %f %f and size %f %f\n", bb.pos.x, bb.pos.y, bb.bounding_box.x, bb.bounding_box.y);
	// top line
	Entity line = createLine(vec2(bb.pos.x, bb.pos.y-(bb.bounding_box.y/2)), { bb.bounding_box.x, 10 });
	registry.boundingLines.emplace(line, BOUNDING_LINE_POS::TOP, entity);
	// bottom line
	line = createLine(vec2(bb.pos.x, bb.pos.y + (bb.bounding_box.y / 2)), { bb.bounding_box.x, 10 });
	registry.boundingLines.emplace(line, BOUNDING_LINE_POS::BOTTOM, entity);
	// left line
	line = createLine(vec2(bb.pos.x - (bb.bounding_box.x / 2), bb.pos.y), { 10, bb.bounding_box.y });
	registry.boundingLines.emplace(line, BOUNDING_LINE_POS::LEFT, entity);
	// right line
	line = createLine(vec2(bb.pos.x + (bb.bounding_box.x / 2), bb.pos.y), { 10, bb.bounding_box.y });
	registry.boundingLines.emplace(line, BOUNDING_LINE_POS::RIGHT, entity);

	return;
}


// Function to compute AABB with position as center
vec4 calculate_AABB(Object& obj) {
	// Define the four corners of the original AABB relative to the center
	vec2 half_scale = { fabs(obj.scale.x) / 2.0f, fabs(obj.scale.y) / 2.0f };
	vec2 corners[4] = {
		{ -half_scale.x, -half_scale.y }, // Top-Left
		{  half_scale.x, -half_scale.y }, // Top-Right
		{  half_scale.x,  half_scale.y }, // Bottom-Right
		{ -half_scale.x,  half_scale.y }  // Bottom-Left
	};

	// Compute rotation components
	float c = cosf(obj.angle);
	float s = sinf(obj.angle);

	// Initialize min and max with extreme values
	float min_x = FLT_MAX, max_x = -FLT_MAX;
	float min_y = FLT_MAX, max_y = -FLT_MAX;

	// Rotate each corner and find the min/max
	for (int i = 0; i < 4; ++i) {
		// Apply rotation
		// In OpenGL, +y is down, so rotation direction is adjusted
		float x_rot = corners[i].x * c + corners[i].y * s;
		float y_rot = -corners[i].x * s + corners[i].y * c; // Adjusted for +y down

		// Update min and max
		min_x = std::min(min_x, x_rot);
		max_x = std::max(max_x, x_rot);
		min_y = std::min(min_y, y_rot);
		max_y = std::max(max_y, y_rot);
	}

	// Calculate the dimensions of the bounding box
	vec2 new_size = { max_x - min_x, max_y - min_y };

	// Calculate the center of the bounding box relative to the object's center
	vec2 aabb_center = { (min_x + max_x) / 2.0f, (min_y + max_y) / 2.0f };

	// The absolute position of the bbox is the object's position plus the relative center	
	vec2 pos = obj.position + aabb_center;

	// Populate the BoundingBox struct
	vec4 bb_info = { new_size.x, new_size.y, pos.x, pos.y };

	return bb_info;
}