#pragma once

#include "stdint.h"
#include "stddef.h"
#include "core/float3.h"
#include "dice_variants.h"

#define MAX_LED_COUNT 21

namespace Config
{
	// Flags for various debugging options
	enum class DebugFlags : uint32_t
	{
		None = 0,
		LEDsStayOff = 1,			 // Prevent LEDs for lighting up
		OnBootToggleLEDsStayOff = 2, // Toggle above flag on firmware boot
		LoopCycleAnimation = 4,		 // Light up LEDs one by one, forever
	};

	struct Settings
	{
		// Indicates whether there is valid data
		uint32_t headMarker;
		int version;

		// Physical Appearance
		DiceVariants::DesignAndColor designAndColor;

		char name[10];

		// Face detector
		float jerkClamp;
		float sigmaDecay;
		float startMovingThreshold;
		float stopMovingThreshold;
		float faceThreshold;
		float fallingThreshold;
		float shockThreshold;
		float accDecay;
		float heatUpRate;
		float coolDownRate;

		// Battery
		float batteryLow;
		float batteryHigh;

		// Calibration data
		Core::float3 faceNormals[MAX_LED_COUNT];

		// Indicates whether there is valid data
		uint32_t tailMarker;
	};

	namespace SettingsManager
	{
		typedef void (*SettingsWrittenCallback)(bool success);

		void init(SettingsWrittenCallback callback);
		bool checkValid();
		Config::Settings const * const getSettings();

		void setDefaults(Settings& outSettings);
		void programDefaults(SettingsWrittenCallback callback);
		void programDefaultParameters(SettingsWrittenCallback callback);
		void programCalibrationData(const Core::float3* newNormals, int count, SettingsWrittenCallback callback);
		void programDesignAndColor(DiceVariants::DesignAndColor design, SettingsWrittenCallback callback);
		void programName(const char* newName, SettingsWrittenCallback callback);
	}
}
