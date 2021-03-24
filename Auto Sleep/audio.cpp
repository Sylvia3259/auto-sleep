#include "audio.h"

static BOOL coInitialized = FALSE;
static IMMDeviceEnumerator* pEnumerator = NULL;
static IMMDevice* pDevice = NULL;
static IAudioMeterInformation* pAudioMeterInfo = NULL;
static std::atomic_bool isRunning = false;
static std::thread thread;
extern std::atomic<time_t> lastEventTime;

BOOL InitializeAudioMeter() {
	if (!coInitialized) {
		if SUCCEEDED(CoInitialize(NULL)) {
			coInitialized = TRUE;

			if SUCCEEDED(CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (LPVOID*)&pEnumerator)) {
				if SUCCEEDED(pEnumerator->GetDefaultAudioEndpoint(EDataFlow::eRender, ERole::eMultimedia, &pDevice)) {
					if SUCCEEDED(pDevice->Activate(__uuidof(IAudioMeterInformation), CLSCTX_ALL, NULL, (void**)&pAudioMeterInfo)) {

						if (!isRunning) {
							isRunning = true;
							thread = std::thread(AudioMeterThread);
						}

						return TRUE;
					}
				}
			}
		}
	}
	return FALSE;
}

void UninitializeAudioMeter() {
	if (isRunning) {
		isRunning = false;
		thread.join();
	}

	if (coInitialized) {
		CoUninitialize();
		coInitialized = FALSE;

		if (pEnumerator) {
			pEnumerator->Release();
			pEnumerator = NULL;
			if (pDevice) {
				pDevice->Release();
				pDevice = NULL;
				if (pAudioMeterInfo) {
					pAudioMeterInfo->Release();
					pAudioMeterInfo = NULL;
				}
			}
		}
	}
}

void AudioMeterThread() {
	float peak;
	while (isRunning) {
		pAudioMeterInfo->GetPeakValue(&peak);
		if (peak > 10e-5f) {
			time_t currentTime;
			time(&currentTime);

			if (lastEventTime != currentTime)
				lastEventTime = currentTime;
		}
		Sleep(100);
	}
}