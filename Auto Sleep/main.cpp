#include "keyboard.h"
#include "mouse.h"
#include "audio.h"
#include <powrprof.h>
#pragma comment (lib, "powrprof.lib")

BOOL CheckAlreadyRunning();
BOOL SetPrivilege(LPCTSTR lpszPrivilege, BOOL bEnablePrivilege);
BOOL InitializeAll();
BOOL UninitializeAll();

std::atomic<time_t> lastEventTime = 0;

int main() {
	time_t currentTime;

	ShowWindow(GetConsoleWindow(), SW_HIDE);

	if (CheckAlreadyRunning())
		return 1;

	if (!SetPrivilege(SE_SHUTDOWN_NAME, TRUE))
		return 1;

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

	return 0;
}

BOOL CheckAlreadyRunning() {
	HANDLE hMutex = CreateMutex(NULL, FALSE, L"AUTO_SLEEP_MUTEX");

	if (!hMutex) {
		MessageBox(NULL, L"CreateMutex() failed.", L"Error", MB_ICONERROR);
		return TRUE;
	}
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		MessageBox(NULL, L"The program is already running.", L"Error", MB_ICONERROR);
		return TRUE;
	}

	return FALSE;
}

BOOL SetPrivilege(LPCTSTR lpszPrivilege, BOOL bEnablePrivilege) {
	HANDLE hToken;
	LUID luid;
	TOKEN_PRIVILEGES tokenPrivileges{};

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
		MessageBox(NULL, L"OpenProcessToken() failed.", L"Error", MB_ICONERROR);
		return FALSE;
	}

	if (!LookupPrivilegeValue(NULL, lpszPrivilege, &luid)) {
		MessageBox(NULL, L"LookupPrivilegeValue() failed.", L"Error", MB_ICONERROR);
		return FALSE;
	}

	tokenPrivileges.PrivilegeCount = 1;
	tokenPrivileges.Privileges[0].Luid = luid;
	tokenPrivileges.Privileges[0].Attributes = bEnablePrivilege ? SE_PRIVILEGE_ENABLED : 0;

	if (!AdjustTokenPrivileges(hToken, FALSE, &tokenPrivileges, sizeof tokenPrivileges, NULL, NULL)) {
		MessageBox(NULL, L"AdjustTokenPrivileges() failed.", L"Error", MB_ICONERROR);
		return FALSE;
	}
	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
		MessageBox(NULL, L"The token does not have the specified privilege.", L"Error", MB_ICONERROR);
		return FALSE;
	}

	if (!CloseHandle(hToken)) {
		MessageBox(NULL, L"CloseHandle() failed.", L"Error", MB_ICONERROR);
		return FALSE;
	}

	return TRUE;
}

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
	BOOL bSuccess = TRUE;

	if (!UninitializeKeyboardHook()) {
		MessageBox(NULL, L"UninitializeKeyboardHook() failed.", L"Error", MB_ICONERROR);
		bSuccess = FALSE;
	}
	if (UninitializeMouseHook()) {
		MessageBox(NULL, L"UninitializeMouseHook() failed.", L"Error", MB_ICONERROR);
		bSuccess = FALSE;
	}
	UninitializeAudioMeter();

	return bSuccess;
}