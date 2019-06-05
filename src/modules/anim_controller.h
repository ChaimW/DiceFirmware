#pragma once

#include "stdint.h"

namespace Modules
{
	/// <summary>
	/// Manages a set of running animations, talking to the LED controller
	/// to tell it what LEDs must have what intensity at what time.
	/// </summary>
	namespace AnimController
	{
		void stopAtIndex(int animIndex);
		void removeAtIndex(int animIndex);
		void update();
		void update(int ms);

		void init();
		void stop();
		void play(const Animation* anim);
		void stop(const Animation* anim);
		void stopAll();
	}
}

