#include "keyboard.h"
#include "mouse.h"
#include "audio.h"
#include <tchar.h>
#include <Windows.h>
#include <powrprof.h>
#pragma comment (lib, "powrprof.lib")

BOOL IsAlreadyRunning();
BOOL SetPrivilege(LPCTSTR lpszPrivilege, BOOL bEnablePrivilege);
BOOL InitializeAll();
BOOL UninitializeAll();

std::atomic<time_t> lastEventTime = 0;

int main() {
	time_t currentTime;

	ShowWindow(GetConsoleWindow(), SW_HIDE);

	if (IsAlreadyRunning())
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

BOOL IsAlreadyRunning() {
	HANDLE hMutex = CreateMutex(NULL, FALSE, L"AUTO_SLEEP_MUTEX");

	if (!hMutex) {
		MessageBox(NULL, _T("CreateMutex() failed"), _T("Error"), MB_ICONERROR);
		return TRUE;
	}
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		MessageBox(NULL, _T("The program is already running"), _T("Error"), MB_ICONERROR);
		return TRUE;
	}

	return FALSE;
}

BOOL SetPrivilege(LPCTSTR lpszPrivilege, BOOL bEnablePrivilege) {
	HANDLE hToken;
	LUID luid;
	TOKEN_PRIVILEGES tokenPrivileges = {};

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
		MessageBox(NULL, _T("OpenProcessToken() failed"), _T("Error"), MB_ICONERROR);
		return FALSE;
	}

	if (!LookupPrivilegeValue(NULL, lpszPrivilege, &luid)) {
		MessageBox(NULL, _T("LookupPrivilegeValue() failed"), _T("Error"), MB_ICONERROR);
		return FALSE;
	}

	tokenPrivileges.PrivilegeCount = 1;
	tokenPrivileges.Privileges[0].Luid = luid;
	tokenPrivileges.Privileges[0].Attributes = bEnablePrivilege ? SE_PRIVILEGE_ENABLED : 0;

	if (!AdjustTokenPrivileges(hToken, FALSE, &tokenPrivileges, sizeof tokenPrivileges, NULL, NULL)) {
		MessageBox(NULL, _T("AdjustTokenPrivileges() failed"), _T("Error"), MB_ICONERROR);
		return FALSE;
	}
	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
		MessageBox(NULL, _T("The token does not have the specified privilege"), _T("Error"), MB_ICONERROR);
		return FALSE;
	}

	if (!CloseHandle(hToken)) {
		MessageBox(NULL, _T("CloseHandle() failed"), _T("Error"), MB_ICONERROR);
		return FALSE;
	}

	return TRUE;
}

BOOL InitializeAll() {
	if (!InitializeKeyboardHook()) {
		MessageBox(NULL, _T("InitializeKeyboardHook() failed"), _T("Error"), MB_ICONERROR);
		return FALSE;
	}
	if (!InitializeMouseHook()) {
		MessageBox(NULL, _T("InitializeMouseHook() failed"), _T("Error"), MB_ICONERROR);
		return FALSE;
	}
	if (!InitializeAudioMeter()) {
		MessageBox(NULL, _T("InitializeAudioMeter() failed"), _T("Error"), MB_ICONERROR);
		return FALSE;
	}

	return TRUE;
}

BOOL UninitializeAll() {
	BOOL bSuccess = TRUE;

	if (!UninitializeKeyboardHook()) {
		MessageBox(NULL, _T("UninitializeKeyboardHook() failed."), _T("Error"), MB_ICONERROR);
		bSuccess = FALSE;
	}
	if (UninitializeMouseHook()) {
		MessageBox(NULL, _T("UninitializeMouseHook() failed."), _T("Error"), MB_ICONERROR);
		bSuccess = FALSE;
	}
	UninitializeAudioMeter();

	return bSuccess;
}