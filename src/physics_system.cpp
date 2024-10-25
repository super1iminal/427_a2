// internal
#include "physics_system.hpp"
#include "world_init.hpp"

// Returns the local bounding coordinates scaled by the current size of the entity
vec2 get_bounding_box(const Object& object)
{
	// abs is to avoid negative scale due to the facing direction.
	return { abs(object.scale.x), abs(object.scale.y) };
}

// This is a SUPER APPROXIMATE check that puts a circle around the bounding boxes and sees
// if the center point of either object is inside the other's bounding-box-circle. You can
// surely implement a more accurate detection
bool collides(const Object& object1, const Object& object2)
{
	vec2 dp = object1.position - object2.position;
	float dist_squared = dot(dp,dp);
	const vec2 other_bonding_box = get_bounding_box(object1) / 2.f;
	const float other_r_squared = dot(other_bonding_box, other_bonding_box);
	const vec2 my_bonding_box = get_bounding_box(object2) / 2.f;
	const float my_r_squared = dot(my_bonding_box, my_bonding_box);
	const float r_squared = max(other_r_squared, my_r_squared);
	if (dist_squared < r_squared)
		return true;
	return false;
}

void PhysicsSystem::step(float elapsed_ms)
{
	// Move fish based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.
	auto& motion_registry = registry.motions;
	auto& object_registry = registry.objects;
	for(uint i = 0; i< motion_registry.size(); i++)
	{
		// !!! TODO A1: update motion.position based on step_seconds and motion.velocity
		//Motion& motion = motion_registry.components[i];
		// need to update position according to angle. 
		Entity entity = motion_registry.entities[i];
		Motion& motion = motion_registry.get(entity);
		Object& object = object_registry.get(entity);
		float step_seconds = elapsed_ms / 1000.f;

		// calculate input velocity (input from controls or set input for entities)
		Transform transform;
		transform.rotate(object.angle);

		// calculate external velocity (from attractors)
		vec2 external_velocity = { 0.f, 0.f };
		if (registry.attractors.size() > 0) {
			if (!registry.attractors.has(entity)) {
				for (Entity attractor : registry.attractors.entities) {
					Attractor& attractor_attract = registry.attractors.get(attractor);
					Motion& attractor_motion = motion_registry.get(attractor);
					Object& attractor_object = object_registry.get(attractor);
					attractor_object.angle += 0.0001f*elapsed_ms;
					if (attractor_object.angle >= 360.f) {
						attractor_object.angle -= 360.f;
					}
					float dist = distance(object.position, attractor_object.position);
					vec2 diff = object.position - attractor_object.position;
					if (dist < attractor_attract.radius)
					{
						// normalize the vector
						diff = normalize(diff);
						// scale the vector by the force
						diff *= attractor_attract.force;
						external_velocity -= diff;
					}
				}
			}
		} 
		motion.external_velocity = external_velocity;



		if (!registry.players.has(entity) && !registry.attractors.has(entity)) {
			float acceleration_magnitude = length(motion.acceleration);

			// Determine the sign of the starting acceleration

			// Adjust the perpendicular acceleration vector based on the sign
			vec2 perpendicular_acceleration = motion.initial_sign * vec2(-motion.input_velocity.y, motion.input_velocity.x);

			perpendicular_acceleration = normalize(perpendicular_acceleration);
			perpendicular_acceleration *= acceleration_magnitude;
			motion.acceleration = perpendicular_acceleration;
			motion.input_velocity += motion.acceleration * step_seconds;
		}

		////	
		////	
		////	


		// update position based on velocity
		vec3 trans_input = transform.mat * vec3(motion.input_velocity, 1.0f); // updating according to rotation
		vec2 result = vec2(trans_input.x, trans_input.y) + motion.external_velocity;
		object.position += result * step_seconds;
		
	}

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// CHECK FOR COLLISIONS
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	// Check for collisions between all moving entities
    ComponentContainer<Object> & object_container = registry.objects;
	for(uint i = 0; i< object_container.components.size(); i++)
	{
		Object& object_i = object_container.components[i];
		
		// note starting j at i+1 to compare all (i,j) pairs only once (and to not compare with itself)
		for(uint j = i+1; j< object_container.components.size(); j++)
		{
			Object& object_j = object_container.components[j];
			if (collides(object_i, object_j))
			{
				Entity entity_i = object_container.entities[i];
				Entity entity_j = object_container.entities[j];
				// Create a collisions event
				// We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
				registry.collisions.emplace_with_duplicates(entity_i, entity_j);
				registry.collisions.emplace_with_duplicates(entity_j, entity_i);
			}
		}
	}

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A2: HANDLE EGG collisions HERE
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 2
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
}