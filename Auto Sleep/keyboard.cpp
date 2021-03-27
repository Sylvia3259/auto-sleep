#include "keyboard.h"

static HHOOK hKeyboardHook = NULL;
static std::atomic_bool isInitialized = false;
static std::atomic_bool isRunning = false;
static std::thread thread;
extern std::atomic<time_t> lastEventTime;

BOOL InitializeKeyboardHook() {
	if (!isRunning) {
		isRunning = true;
		thread = std::thread(KeyboardHookThread);

		while (!isInitialized);
		return hKeyboardHook != NULL;
	}
	return FALSE;
}

BOOL UninitializeKeyboardHook() {
	if (isRunning) {
		isRunning = false;
		thread.join();

		return hKeyboardHook == NULL;
	}
	return TRUE;
}

void KeyboardHookThread() {
	hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), 0);
	isInitialized = true;

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) && isRunning) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (hKeyboardHook)
		if (UnhookWindowsHookEx(hKeyboardHook))
			hKeyboardHook = NULL;
	isInitialized = false;
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
	time_t currentTime;

	time(&currentTime);
	lastEventTime = currentTime;

	return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
}