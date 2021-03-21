#pragma once

#include <atomic>
#include <thread>
#include <time.h>
#include <Windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>

BOOL InitializeAudioMeter();
void UninitializeAudioMeter();
void AudioMeterThread();