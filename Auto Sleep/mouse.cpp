#include "mouse.h"

static HHOOK hMouseHook = NULL;
static std::atomic_bool isInitialized = false;
static std::atomic_bool isRunning = false;
static std::thread thread;
extern std::atomic<time_t> lastEventTime;

BOOL InitializeMouseHook() {
	if (!isRunning) {
		isRunning = true;
		thread = std::thread(MouseHookThread);

		while (!isInitialized);
		return hMouseHook != NULL;
	}
	return FALSE;
}

BOOL UninitializeMouseHook() {
	if (isRunning) {
		isRunning = false;
		thread.join();

		return hMouseHook == NULL;
	}
	return FALSE;
}

void MouseHookThread() {
	hMouseHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, GetModuleHandle(NULL), 0);
	isInitialized = true;

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) && isRunning) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (hMouseHook)
		if (UnhookWindowsHookEx(hMouseHook))
			hMouseHook = NULL;
	isInitialized = false;
}

LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
	time_t currentTime;
	time(&currentTime);

	if (lastEventTime != currentTime)
		lastEventTime = currentTime;

	return CallNextHookEx(hMouseHook, nCode, wParam, lParam);
}