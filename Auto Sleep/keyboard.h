#pragma once

#include <atomic>
#include <thread>
#include <time.h>
#include <Windows.h>

BOOL InitializeKeyboardHook();
BOOL UninitializeKeyboardHook();
void KeyboardHookThread();
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);