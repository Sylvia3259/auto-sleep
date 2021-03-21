#pragma once

#include <atomic>
#include <thread>
#include <time.h>
#include <Windows.h>

BOOL InitializeMouseHook();
BOOL UninitializeMouseHook();
void MouseHookThread();
LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);