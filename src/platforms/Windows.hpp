#include "Platform.hpp"
#include <GLFW/glfw3.h>

#include <BaseTsd.h>

namespace vi {
constexpr UINT glfwKeyToVK(int key) noexcept {
	// Printable keys
	if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z) {
		return key;
	}
	if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9) {
		return key;
	}

	// Function row.
	if (key >= GLFW_KEY_F1 && key <= GLFW_KEY_F25) {
		return VK_F1 + (key - GLFW_KEY_F1);
	}

	// The annoying ones.
	switch (key) {
	case GLFW_KEY_ESCAPE:		 return VK_ESCAPE;
	case GLFW_KEY_ENTER:		 return VK_RETURN;
	case GLFW_KEY_TAB:			 return VK_TAB;
	case GLFW_KEY_BACKSPACE:	 return VK_BACK;
	case GLFW_KEY_INSERT:		 return VK_INSERT;
	case GLFW_KEY_DELETE:		 return VK_DELETE;
	case GLFW_KEY_RIGHT:		 return VK_RIGHT;
	case GLFW_KEY_LEFT:			 return VK_LEFT;
	case GLFW_KEY_DOWN:			 return VK_DOWN;
	case GLFW_KEY_UP:			 return VK_UP;
	case GLFW_KEY_PAGE_UP:		 return VK_PRIOR;
	case GLFW_KEY_PAGE_DOWN:	 return VK_NEXT;
	case GLFW_KEY_HOME:			 return VK_HOME;
	case GLFW_KEY_END:			 return VK_END;
	case GLFW_KEY_CAPS_LOCK:	 return VK_CAPITAL;
	case GLFW_KEY_SCROLL_LOCK:	 return VK_SCROLL;
	case GLFW_KEY_NUM_LOCK:		 return VK_NUMLOCK;
	case GLFW_KEY_PRINT_SCREEN:	 return VK_SNAPSHOT;
	case GLFW_KEY_PAUSE:		 return VK_PAUSE;
	case GLFW_KEY_KP_DECIMAL:	 return VK_DECIMAL;
	case GLFW_KEY_KP_DIVIDE:	 return VK_DIVIDE;
	case GLFW_KEY_KP_MULTIPLY:	 return VK_MULTIPLY;
	case GLFW_KEY_KP_SUBTRACT:	 return VK_SUBTRACT;
	case GLFW_KEY_KP_ADD:		 return VK_ADD;
	case GLFW_KEY_KP_ENTER:		 return VK_RETURN;
	case GLFW_KEY_KP_EQUAL:		 return VK_OEM_PLUS;
	case GLFW_KEY_LEFT_SHIFT:	 return VK_LSHIFT;
	case GLFW_KEY_LEFT_CONTROL:	 return VK_LCONTROL;
	case GLFW_KEY_LEFT_ALT:		 return VK_LMENU;
	case GLFW_KEY_LEFT_SUPER:	 return VK_LWIN;
	case GLFW_KEY_RIGHT_SHIFT:	 return VK_RSHIFT;
	case GLFW_KEY_RIGHT_CONTROL: return VK_RCONTROL;
	case GLFW_KEY_RIGHT_ALT:	 return VK_RMENU;
	case GLFW_KEY_RIGHT_SUPER:	 return VK_RWIN;
	case GLFW_KEY_SPACE:		 return VK_SPACE;
	case GLFW_KEY_APOSTROPHE:	 return VK_OEM_7;
	case GLFW_KEY_COMMA:		 return VK_OEM_COMMA;
	case GLFW_KEY_MINUS:		 return VK_OEM_MINUS;
	case GLFW_KEY_PERIOD:		 return VK_OEM_PERIOD;
	case GLFW_KEY_SLASH:		 return VK_OEM_2;
	case GLFW_KEY_SEMICOLON:	 return VK_OEM_1;
	case GLFW_KEY_EQUAL:		 return VK_OEM_PLUS;
	case GLFW_KEY_MENU:			 return VK_APPS;
	case GLFW_KEY_LEFT_BRACKET:	 return VK_OEM_4;
	case GLFW_KEY_BACKSLASH:	 return VK_OEM_5;
	case GLFW_KEY_RIGHT_BRACKET: return VK_OEM_6;
	case GLFW_KEY_GRAVE_ACCENT:	 return VK_OEM_3;
	case GLFW_KEY_WORLD_1:
	case GLFW_KEY_WORLD_2:
	default:					 return 0;
	}
}
} // namespace vi