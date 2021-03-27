#include <iostream>
#include "common.h"

std::atomic<time_t> lastEventTime = 0;

int main() {
	ShowWindow(GetConsoleWindow(), SW_HIDE);

	HANDLE hMutex = CreateMutex(NULL, FALSE, L"AUTO_SLEEP_MUTEX");
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		MessageBox(NULL, L"The program is already running.", L"Error", MB_ICONERROR);
		return 1;
	}
	else if (!hMutex) {
		MessageBox(NULL, L"CreateMutex() failed.", L"Error", MB_ICONERROR);
		return 1;
	}

	if (!InitializeKeyboardHook()) {
		MessageBox(NULL, L"InitializeKeyboardHook() failed.", L"Error", MB_ICONERROR);
		return 1;
	}
	if (!InitializeMouseHook()) {
		MessageBox(NULL, L"InitializeMouseHook() failed.", L"Error", MB_ICONERROR);
		return 1;
	}
	if (!InitializeAudioMeter()) {
		MessageBox(NULL, L"InitializeAudioMeter() failed.", L"Error", MB_ICONERROR);
		return 1;
	}

	time_t currentTime;
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

	if (!UninitializeKeyboardHook()) {
		MessageBox(NULL, L"UninitializeKeyboardHook() failed.", L"Error", MB_ICONERROR);
		return 1;
	}
	if (UninitializeMouseHook()) {
		MessageBox(NULL, L"UninitializeMouseHook() failed.", L"Error", MB_ICONERROR);
		return 1;
	}
	UninitializeAudioMeter();

	if (!ReleaseMutex(hMutex)) {
		MessageBox(NULL, L"ReleaseMutex() failed.", L"Error", MB_ICONERROR);
		return 1;
	}

	return 0;
}