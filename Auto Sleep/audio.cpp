#include "audio.h"

static BOOL coInitialized = FALSE;
static IMMDeviceEnumerator* pEnumerator = NULL;
static IMMDevice* pDevice = NULL;
static IAudioMeterInformation* pAudioMeterInfo = NULL;
static std::atomic_bool isRunning = false;
static std::thread thread;
extern std::atomic<time_t> lastEventTime;

BOOL InitializeAudioMeter() {
	if (!isRunning) {
		isRunning = true;
		thread = std::thread(AudioMeterThread);
	}

	if (!coInitialized) {
		if SUCCEEDED(CoInitialize(NULL)) {
			coInitialized = TRUE;

			if SUCCEEDED(CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (LPVOID*)&pEnumerator))
				if SUCCEEDED(pEnumerator->GetDefaultAudioEndpoint(EDataFlow::eRender, ERole::eMultimedia, &pDevice))
					if SUCCEEDED(pDevice->Activate(__uuidof(IAudioMeterInformation), CLSCTX_ALL, NULL, (void**)&pAudioMeterInfo))
						return TRUE;
		}
	}
	return FALSE;
}

void UninitializeAudioMeter() {
	if (isRunning) {
		isRunning = false;
		thread.join();
	}

	if (pAudioMeterInfo) {
		pAudioMeterInfo->Release();
		pAudioMeterInfo = NULL;
	}
	if (pDevice) {
		pDevice->Release();
		pDevice = NULL;
	}
	if (pEnumerator) {
		pEnumerator->Release();
		pEnumerator = NULL;
	}

	if (coInitialized) {
		CoUninitialize();
		coInitialized = FALSE;
	}
}

void AudioMeterThread() {
	FLOAT peak;
	time_t currentTime;

	while (isRunning) {
		if (pAudioMeterInfo) {
			pAudioMeterInfo->GetPeakValue(&peak);

			if (peak > 10e-5f) {
				time(&currentTime);
				lastEventTime = currentTime;
			}
		}

		Sleep(100);
	}
}