// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>

#include "physics_system.hpp"

// Game configuration
const size_t MAX_NUM_EELS = 15;
const size_t MAX_NUM_FISH = 5;
const size_t MAX_NUM_WHIRL = 1;
const size_t EEL_SPAWN_DELAY_MS = 2000 * 3;
const size_t FISH_SPAWN_DELAY_MS = 5000 * 3;
const size_t WHIRLPOOL_SPAWN_DELAY_MS = 8000;
const size_t PUFFER_SPAWN_DELAY_MS = 2000 * 5;
const size_t PUFFER_TRAJ_SWAP = 1000;

// create the underwater world
WorldSystem::WorldSystem()
	: points(0)
	, next_eel_spawn(0.f)
	, next_fish_spawn(0.f)
	, keyUp(false)
	, keyDown(false)
	, keyLeft(false)
	, keyRight(false) 
	, state(0) 
	, next_puffer_spawn(0.f) {
	// Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
	printf("Color shift and distortion are active\n");
}

WorldSystem::~WorldSystem() {
	
	// destroy music components
	if (background_music != nullptr)
		Mix_FreeMusic(background_music);
	if (salmon_dead_sound != nullptr)
		Mix_FreeChunk(salmon_dead_sound);
	if (salmon_eat_sound != nullptr)
		Mix_FreeChunk(salmon_eat_sound);

	Mix_CloseAudio();

	// Destroy all created components
	registry.clear_all_components();

	// Close the window
	glfwDestroyWindow(window);
}

// Debugging
namespace {
	void glfw_err_cb(int error, const char *desc) {
		fprintf(stderr, "%d: %s", error, desc);
	}
}

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the renderer
GLFWwindow* WorldSystem::create_window() {
	///////////////////////////////////////
	// Initialize GLFW
	glfwSetErrorCallback(glfw_err_cb);
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW");
		return nullptr;
	}

	//-------------------------------------------------------------------------
	// If you are on Linux or Windows, you can change these 2 numbers to 4 and 3 and
	// enable the glDebugMessageCallback to have OpenGL catch your mistakes for you.
	// GLFW / OGL Initialization
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_RESIZABLE, 0);

	// Create the main window (for rendering, keyboard, and mouse input)
	window = glfwCreateWindow(window_width_px, window_height_px, "Salmon Game Assignment", nullptr, nullptr);
	if (window == nullptr) {
		fprintf(stderr, "Failed to glfwCreateWindow");
		return nullptr;
	}

	// Setting callbacks to member functions upon keypresses and cursor movement (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this); // store a pointer to the current WorldSystem instance, accessed by glfwGetWindowUserPointer (see directly below)
	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); }; // parameters are the window that this occurred in and the key information, function triggers on_key
	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 }); }; // parameters are the window that this occurred in and the cursor position, function triggers on_mouse_move
	glfwSetKeyCallback(window, key_redirect); // set the key callback function to key_redirect
	glfwSetCursorPosCallback(window, cursor_pos_redirect); // set the cursor position callback function to cursor_pos_redirect

	//////////////////////////////////////
	// Loading music and sounds with SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "Failed to initialize SDL Audio");
		return nullptr;
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
		fprintf(stderr, "Failed to open audio device");
		return nullptr;
	}

	background_music = Mix_LoadMUS(audio_path("music.wav").c_str());
	salmon_dead_sound = Mix_LoadWAV(audio_path("death_sound.wav").c_str());
	salmon_eat_sound = Mix_LoadWAV(audio_path("eat_sound.wav").c_str());

	if (background_music == nullptr || salmon_dead_sound == nullptr || salmon_eat_sound == nullptr) {
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
			audio_path("music.wav").c_str(),
			audio_path("death_sound.wav").c_str(),
			audio_path("eat_sound.wav").c_str());
		return nullptr;
	}

	return window;
}

void WorldSystem::init(RenderSystem* renderer_arg) {
	this->renderer = renderer_arg;
	// Playing background music indefinitely
	Mix_PlayMusic(background_music, -1);
	fprintf(stderr, "Loaded music\n");

	// Set all states to default
    restart_game();
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {
	// Updating window title with points
	std::stringstream title_ss;
	title_ss << "Points: " << points;
	glfwSetWindowTitle(window, title_ss.str().c_str());

	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
	    registry.remove_all_components_of(registry.debugComponents.entities.back());

	// Removing out of screen entities
	auto& motions_registry = registry.motions;

	// Remove entities that leave the screen on the left side
	// Iterate backwards to be able to remove without interfering with the next object to visit
	// (the containers exchange the last element with the current)
	for (int i = (int)motions_registry.components.size()-1; i>=0; --i) {
	    Motion& motion = motions_registry.components[i];
		if ((motion.position.x + abs(motion.scale.x) < 0.f) || (motion.position.y + abs(motion.scale.y) > window_height_px + 100)) {
			if(!registry.players.has(motions_registry.entities[i])) // don't remove the player
				registry.remove_all_components_of(motions_registry.entities[i]);
		}
	}

	// spawn new eels
	next_eel_spawn -= elapsed_ms_since_last_update * current_speed;
	if (registry.deadlys.components.size() <= MAX_NUM_EELS && next_eel_spawn < 0.f) {
		// reset timer
		next_eel_spawn = (EEL_SPAWN_DELAY_MS / 2) + uniform_dist(rng) * (EEL_SPAWN_DELAY_MS / 2);

		// create Eel with random initial position
        // createEel(renderer, vec2(50.f + uniform_dist(rng) * (window_width_px - 100.f), 100.f));
		createEel(renderer, vec2(window_width_px + 50, window_height_px*uniform_dist(rng)));
	}

	// spawn fish
	next_fish_spawn -= elapsed_ms_since_last_update * current_speed;
	if (registry.eatables.components.size() <= MAX_NUM_FISH && next_fish_spawn < 0.f) {
		// !!!  TODO A1: create new fish with createFish({0,0}), see eels above (done)
		next_fish_spawn = (FISH_SPAWN_DELAY_MS / 2) + uniform_dist(rng) * (FISH_SPAWN_DELAY_MS / 2);
		createFish(renderer, vec2(window_width_px + 50, window_height_px * uniform_dist(rng)));
	}

	// advanced mechanics
	if (state == 1) {
		// spawn whirlpools
		next_whirl_spawn -= elapsed_ms_since_last_update * current_speed;
		if (registry.attractors.components.size() <= MAX_NUM_WHIRL && next_whirl_spawn < 0.f) {
			next_whirl_spawn = (WHIRLPOOL_SPAWN_DELAY_MS / 2) + uniform_dist(rng) * (WHIRLPOOL_SPAWN_DELAY_MS / 2);
			createWhirlpool(renderer, vec2((window_width_px-100) * uniform_dist(rng) + 50, (window_height_px-70) * uniform_dist(rng) + 35), uniform_dist(rng) + 0.25);
		}

		next_puffer_spawn -= elapsed_ms_since_last_update * current_speed;
		if (next_puffer_spawn < 0.f) {
			next_puffer_spawn = (PUFFER_SPAWN_DELAY_MS / 2) + uniform_dist(rng) * (PUFFER_SPAWN_DELAY_MS / 2);
			createPuffer(renderer, vec2((window_width_px)*uniform_dist(rng), window_height_px), uniform_dist(rng) - 0.5f, uniform_dist(rng));
		}


	}

	// spawn pufferfish and change their trajectories


	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A2: HANDLE EGG SPAWN HERE
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 2
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	// Processing the salmon state
	// Added functionality to remove dead entities
	assert(registry.screenStates.components.size() <= 1);
    ScreenState &screen = registry.screenStates.components[0];

    float min_counter_ms = 3000.f;
	for (Entity entity : registry.deathTimers.entities) {
		// progress timer
		DeathTimer& counter = registry.deathTimers.get(entity);
		counter.counter_ms -= elapsed_ms_since_last_update;
		if (registry.players.has(entity)) {
			if (counter.counter_ms < min_counter_ms) {
				min_counter_ms = counter.counter_ms;
			}
			// restart the game once the death timer expired
			if (counter.counter_ms < 0) {
				registry.deathTimers.remove(entity);
				screen.darken_screen_factor = 0;
				restart_game();
				if (state == 1) {
					for (Entity player : registry.players.entities) {
						registry.motions.get(player).scale /= 1.5f;
					}
				}
				return true;
			}
		}
		else {
			// remove the entity once it dies
			if (counter.counter_ms < 0) {
				registry.deathTimers.remove(entity);
				registry.remove_all_components_of(entity);
			}
		}

	}
	// reduce window brightness if the salmon is dying
	screen.darken_screen_factor = 1 - min_counter_ms / 3000;

	// !!! TODO A1: update LightUp timers and remove if time drops below zero, similar to the death counter
	for (Entity entity : registry.lightUps.entities) {
		LightUp& counter = registry.lightUps.get(entity);
		counter.counter_ms -= elapsed_ms_since_last_update;
		if (counter.counter_ms < 0) {
			registry.lightUps.remove(entity);
		}
	}

	return true;
}

// Reset the world state to its initial state
void WorldSystem::restart_game() {
	// Debugging for memory/component leaks
	registry.list_all_components();
	printf("Restarting\n");

	// Reset the game speed
	current_speed = 1.f;

	// Remove all entities that we created
	// All that have a motion, we could also iterate over all fish, eels, ... but that would be more cumbersome
	while (registry.motions.entities.size() > 0)
	    registry.remove_all_components_of(registry.motions.entities.back());

	// Debugging for memory/component leaks
	registry.list_all_components();

	// create a new Salmon
	// player_salmon = createSalmon(renderer, { window_width_px/2, window_height_px - 200 });
	player_salmon = createSalmon(renderer, {0,0});
	registry.colors.emplace(player_salmon, vec3(1, 0.8f, 0.8f));

	points = 0;
	next_fish_spawn = 0.f;
	next_eel_spawn = 0.f;
	next_whirl_spawn = 0.f;
	next_puffer_spawn = 0.f;

	// !! TODO A2: Enable static eggs on the ground, for reference
	// Create eggs on the floor, use this for reference
	/*
	for (uint i = 0; i < 20; i++) {
		int w, h;
		glfwGetWindowSize(window, &w, &h);
		float radius = 30 * (uniform_dist(rng) + 0.3f); // range 0.3 .. 1.3
		Entity egg = createEgg({ uniform_dist(rng) * w, h - uniform_dist(rng) * 20 },
			         { radius, radius });
		float brightness = uniform_dist(rng) * 0.5 + 0.5;
		registry.colors.insert(egg, { brightness, brightness, brightness});
	}
	*/
}

// Compute collisions between entities
void WorldSystem::handle_collisions() {
	// Loop over all collisions detected by the physics system
	auto& collisionsRegistry = registry.collisions;
	for (uint i = 0; i < collisionsRegistry.components.size(); i++) {
		// The entity and its collider
		Entity entity = collisionsRegistry.entities[i];
		Entity entity_other = collisionsRegistry.components[i].other;

		// for now, we are only interested in collisions that involve the salmon
		if (registry.players.has(entity)) {
			//Player& player = registry.players.get(entity);

			// Checking Player - Deadly collisions
			if (registry.deadlys.has(entity_other)) {
				// initiate death unless already dying
				if (!registry.deathTimers.has(entity) && (!registry.deathTimers.has(entity_other) || registry.attractors.has(entity_other))) {
					// Scream, reset timer, and make the salmon sink
					registry.deathTimers.emplace(entity);
					Mix_PlayChannel(-1, salmon_dead_sound, 0);
					assert(registry.motions.has(entity) && "Player does not have motion!");
					Motion& motion = registry.motions.get(entity);

					// !!! TODO A1: change the salmon's orientation and color on death

					// change orientation
					motion.angle = 0.f;
					motion.input_velocity.y = -100.f;
					motion.input_velocity.x = 0.f;

					// make red
					registry.colors.get(entity) = vec3(1.f, 0.f, 0.f);

				}
			}
			// Checking Player - Eatable collisions
			else if (registry.eatables.has(entity_other)) {
				if (!registry.deathTimers.has(entity) && !registry.deathTimers.has(entity_other)) {
					// chew, count points, and set the LightUp timer
					points += registry.eatables.get(entity_other).points;
					registry.remove_all_components_of(entity_other);
					Mix_PlayChannel(-1, salmon_eat_sound, 0);
					

					// !!! TODO A1: create a new struct called LightUp in components.hpp and add an instance to the salmon entity by modifying the ECS registry
					if (!registry.lightUps.has(entity)) {
						registry.lightUps.emplace(entity);
					}
					else {
						registry.lightUps.get(entity).counter_ms = 500.f;
					}
				}
			}
		} 
		// handling whirlpool kills
		else if (registry.attractors.has(entity)) {
			if (!registry.players.has(entity_other)) {
				if (!registry.deathTimers.has(entity_other)) {
					registry.deathTimers.emplace(entity_other);
					Mix_PlayChannel(-1, salmon_dead_sound, 0);
					Motion& motion = registry.motions.get(entity_other);
					motion.angle = M_PI;
					motion.input_velocity.y = 100.f;
					motion.input_velocity.x = 0.f;
					motion.acceleration = { 0,0 };

					// death
					if (registry.colors.has(entity_other)) {
						registry.colors.get(entity_other) = vec3(1.f, 0.f, 0.f);
					}
				}
			}
		}
	}

	// Remove all collisions from this simulation step
	registry.collisions.clear();
}

// Should the game be over ?
bool WorldSystem::is_over() const {
	return bool(glfwWindowShouldClose(window));
}

// On key callback
void WorldSystem::on_key(int key, int, int action, int mod) {
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A1: HANDLE SALMON MOVEMENT HERE
	// key is of 'type' GLFW_KEY_
	// action can be GLFW_PRESS GLFW_RELEASE GLFW_REPEAT
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


	// handling keypresses:
	if (registry.players.entities.size() > 0 && !registry.deathTimers.has(registry.players.entities[0])) {
		// Member variables to keep track of key states

		// Key handler
		if (key == GLFW_KEY_LEFT || key == GLFW_KEY_DOWN || key == GLFW_KEY_RIGHT || key == GLFW_KEY_UP) {
			assert(registry.players.entities.size() > 0 && "No player found for moving.");
			Entity& player_salmon = registry.players.entities[0];
			Motion& motion = registry.motions.get(player_salmon);
			float speed = 100.f;

			// Update key states
			if (action == GLFW_PRESS) {
				if (key == GLFW_KEY_UP) keyUp = true;
				if (key == GLFW_KEY_DOWN) keyDown = true;
				if (key == GLFW_KEY_LEFT) keyLeft = true;
				if (key == GLFW_KEY_RIGHT) keyRight = true;
			}
			else if (action == GLFW_RELEASE) {
				if (key == GLFW_KEY_UP) keyUp = false;
				if (key == GLFW_KEY_DOWN) keyDown = false;
				if (key == GLFW_KEY_LEFT) keyLeft = false;
				if (key == GLFW_KEY_RIGHT) keyRight = false;
			}

			// Compute input direction
			glm::vec2 input_direction(0.f, 0.f);
			if (keyUp) input_direction.x -= 1.f;
			if (keyDown) input_direction.x += 1.f;
			if (keyLeft) input_direction.y += 1.f;
			if (keyRight) input_direction.y -= 1.f;

			// Normalize the input direction
			if (glm::length(input_direction) > 0.f) {
				input_direction = glm::normalize(input_direction);
			}

			// Set input velocity
			glm::vec2 input_velocity = speed * input_direction;

			// Assign input velocity to motion component
			motion.input_velocity = input_velocity;
		}

	}

	if (action == GLFW_RELEASE && key == GLFW_KEY_A && state == 0) {
		printf("Switching to Advanced Mode\n"); // TODO: RESET ANY ADVANCED MODE SPECIFIC STATES
		state = 1;
		restart_game();
		for (Entity player : registry.players.entities) {
			registry.motions.get(player).scale /= 1.5f;
		}
	}

	if (action == GLFW_RELEASE && key == GLFW_KEY_B && state == 1) {
		printf("Switching to Basic Mode\n"); // TODO: CLEANUP WHEN SWITCHED BACK
		state = 0;
		restart_game();
	}

	if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE) {
		glfwSetWindowShouldClose(window, true);
	}

	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
		int w, h;
		glfwGetWindowSize(window, &w, &h);
        restart_game();
		if (state == 1) {
			for (Entity player : registry.players.entities) {
				registry.motions.get(player).scale /= 1.5f;
			}
		}
	}

	// Debugging
	if (key == GLFW_KEY_D) {
		if (action == GLFW_RELEASE)
			debugging.in_debug_mode = false;
		else
			debugging.in_debug_mode = true;
	}

	// Control the current speed with `<` `>`
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_COMMA) {
		current_speed -= 0.1f;
		printf("Current speed = %f\n", current_speed);
	}
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_PERIOD) {
		current_speed += 0.1f;
		printf("Current speed = %f\n", current_speed);
	}
	current_speed = fmax(0.f, current_speed);
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A1: HANDLE SALMON ROTATION HERE
	// xpos and ypos are relative to the top-left of the window, the salmon's
	// default facing direction is (1, 0)
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// we translate first, so rotation doesn't affect translation. we could rotate first, though, which would be GREAT. see render_system.cpp

	/*
	basic math:
	tan theta = opposite / adjacent
	opposite here is the difference in y, adjacent is the difference in x

	so, theta = arctan((y2 - y1) / (x2 - x1))
	we get an answer in radians, which is what we want.

	subtract the salmon's position from the mouse's position to get the difference in x and y	
	*/

	// again, we're getting the salmon entity via players[0] because we're only dealing with one player
	if (registry.players.entities.size() > 0 && !registry.deathTimers.has(registry.players.entities[0])) {
		assert(registry.players.entities.size() > 0 && "No player found for rotating.");
		Entity& player_salmon = registry.players.entities[0]; // just gonna use the first player in the players ComponentContainer
		Motion& motion = registry.motions.get(player_salmon);

		vec2 diff = motion.position - mouse_position;
		float angle = atan2(diff.y, diff.x); // opposite, adjacent
		motion.angle = angle;
	}
}
