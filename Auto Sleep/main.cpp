#include <iostream>
#include "common.h"

std::atomic<time_t> lastEventTime = 0;

BOOL InitializeAll() {
	if (!InitializeKeyboardHook()) {
		MessageBox(NULL, L"InitializeKeyboardHook() failed.", L"Error", MB_ICONERROR);
		return FALSE;
	}
	if (!InitializeMouseHook()) {
		MessageBox(NULL, L"InitializeMouseHook() failed.", L"Error", MB_ICONERROR);
		return FALSE;
	}
	if (!InitializeAudioMeter()) {
		MessageBox(NULL, L"InitializeAudioMeter() failed.", L"Error", MB_ICONERROR);
		return FALSE;
	}
	return TRUE;
}

BOOL UninitializeAll() {
	BOOL success = TRUE;
	if (!UninitializeKeyboardHook()) {
		MessageBox(NULL, L"UninitializeKeyboardHook() failed.", L"Error", MB_ICONERROR);
		success = FALSE;
	}
	if (UninitializeMouseHook()) {
		MessageBox(NULL, L"UninitializeMouseHook() failed.", L"Error", MB_ICONERROR);
		success = FALSE;
	}
	UninitializeAudioMeter();
	return success;
}

int main() {
	HANDLE hMutex;
	time_t currentTime;

	ShowWindow(GetConsoleWindow(), SW_HIDE);

	hMutex = CreateMutex(NULL, FALSE, L"AUTO_SLEEP_MUTEX");
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		MessageBox(NULL, L"The program is already running.", L"Error", MB_ICONERROR);
		return 1;
	}
	else if (!hMutex) {
		MessageBox(NULL, L"CreateMutex() failed.", L"Error", MB_ICONERROR);
		return 1;
	}

	if (!InitializeAll()) {
		UninitializeAll();
		return 1;
	}

	time(&currentTime);
	lastEventTime = currentTime;

	for (;;) {
		time(&currentTime);

		if (currentTime - lastEventTime >= 15 * 60) {
			SetSuspendState(FALSE, TRUE, FALSE);

			time(&currentTime);
			lastEventTime = currentTime;
		}

		Sleep(100);
	}

	if (!UninitializeAll())
		return 1;

	if (!ReleaseMutex(hMutex)) {
		MessageBox(NULL, L"ReleaseMutex() failed.", L"Error", MB_ICONERROR);
		return 1;
	}

	return 0;
}