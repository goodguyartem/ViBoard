#pragma once

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <X11/Xlib.h>

namespace vi {
Display* getX11Display() noexcept;

constexpr KeySym glfwKeyToKeySym(int key) noexcept {
	// Printable keys can map directly.
	if (key >= GLFW_KEY_SPACE && key <= GLFW_KEY_GRAVE_ACCENT) {
		return static_cast<KeySym>(key);
	}

	// A-Z
	if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z) {
		return XK_a + (key - GLFW_KEY_A);
	}

	// F1-F25
	if (key >= GLFW_KEY_F1 && key <= GLFW_KEY_F25) {
		return XK_F1 + (key - GLFW_KEY_F1);
	}

	// Other
	switch (key) {
	case GLFW_KEY_ESCAPE:		 return XK_Escape;
	case GLFW_KEY_ENTER:		 return XK_Return;
	case GLFW_KEY_TAB:			 return XK_Tab;
	case GLFW_KEY_BACKSPACE:	 return XK_BackSpace;
	case GLFW_KEY_INSERT:		 return XK_Insert;
	case GLFW_KEY_DELETE:		 return XK_Delete;
	case GLFW_KEY_RIGHT:		 return XK_Right;
	case GLFW_KEY_LEFT:			 return XK_Left;
	case GLFW_KEY_DOWN:			 return XK_Down;
	case GLFW_KEY_UP:			 return XK_Up;
	case GLFW_KEY_PAGE_UP:		 return XK_Page_Up;
	case GLFW_KEY_PAGE_DOWN:	 return XK_Page_Down;
	case GLFW_KEY_HOME:			 return XK_Home;
	case GLFW_KEY_END:			 return XK_End;
	case GLFW_KEY_CAPS_LOCK:	 return XK_Caps_Lock;
	case GLFW_KEY_SCROLL_LOCK:	 return XK_Scroll_Lock;
	case GLFW_KEY_NUM_LOCK:		 return XK_Num_Lock;
	case GLFW_KEY_PRINT_SCREEN:	 return XK_Print;
	case GLFW_KEY_PAUSE:		 return XK_Pause;
	case GLFW_KEY_KP_0:			 return XK_KP_0;
	case GLFW_KEY_KP_1:			 return XK_KP_1;
	case GLFW_KEY_KP_2:			 return XK_KP_2;
	case GLFW_KEY_KP_3:			 return XK_KP_3;
	case GLFW_KEY_KP_4:			 return XK_KP_4;
	case GLFW_KEY_KP_5:			 return XK_KP_5;
	case GLFW_KEY_KP_6:			 return XK_KP_6;
	case GLFW_KEY_KP_7:			 return XK_KP_7;
	case GLFW_KEY_KP_8:			 return XK_KP_8;
	case GLFW_KEY_KP_9:			 return XK_KP_9;
	case GLFW_KEY_KP_DECIMAL:	 return XK_KP_Decimal;
	case GLFW_KEY_KP_DIVIDE:	 return XK_KP_Divide;
	case GLFW_KEY_KP_MULTIPLY:	 return XK_KP_Multiply;
	case GLFW_KEY_KP_SUBTRACT:	 return XK_KP_Subtract;
	case GLFW_KEY_KP_ADD:		 return XK_KP_Add;
	case GLFW_KEY_KP_ENTER:		 return XK_KP_Enter;
	case GLFW_KEY_KP_EQUAL:		 return XK_KP_Equal;
	case GLFW_KEY_LEFT_SHIFT:	 return XK_Shift_L;
	case GLFW_KEY_LEFT_CONTROL:	 return XK_Control_L;
	case GLFW_KEY_LEFT_ALT:		 return XK_Alt_L;
	case GLFW_KEY_LEFT_SUPER:	 return XK_Super_L;
	case GLFW_KEY_RIGHT_SHIFT:	 return XK_Shift_R;
	case GLFW_KEY_RIGHT_CONTROL: return XK_Control_R;
	case GLFW_KEY_RIGHT_ALT:	 return XK_Alt_R;
	case GLFW_KEY_RIGHT_SUPER:	 return XK_Super_R;
	case GLFW_KEY_MENU:			 return XK_Menu;
	default:					 return NoSymbol;
	}
}
} // namespace vi