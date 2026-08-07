#include "sol/sol.hpp"
#include <glm/glm.hpp>
namespace lte {
	class LuaHandler {
		static inline sol::state lua;
		static void Init()
		{
			//can open moar later!
			lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::table);
		}

		static void exampleMove(int actorID, float steps) {
			std::cout << "Actor " << actorID << " moved " << steps << " steps!\n";
		}

		void exampleSetupBindings(sol::state& lua) {
			// Bind a simple global function
			lua["move_actor"] = &exampleMove;

			//bind an entire C++ struct/class if your blocks use objects/
			//should generate on the fly
			lua.new_usertype<glm::vec3>("Vec3",
				"x", &glm::vec3::x,
				"y", &glm::vec3::y,
				"z", &glm::vec3::z
			);
		}
		std::string luaCodeSnippet = "function on_update(actor_id)\nif touching_wall(actor_id) then\n\tmove_actor(actor_id, -10.0)\n\tend\nend";
		void exampleCompile(sol::state& lua, const std::string& generatedLuaCode) {
			try {
				//compiles and runs the script
				lua.script(generatedLuaCode);

				// If the script defined an 'on_update' function, you can call it every frame:
				sol::protected_function updateFunc = lua["on_update"];
				if (updateFunc.valid()) {
					updateFunc(42); // Passes '42' as the actor_id argument
				}
			}
			catch (const sol::error& e) {
				std::cerr << "Lua Script Error: " << e.what() << std::endl;
			}
		}
	};
}