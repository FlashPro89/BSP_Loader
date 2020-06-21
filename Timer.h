#pragma once

#ifndef _TIMER_H_
#define _TIMER_H_

#include <windows.h>

class gTimer
{
public:
	gTimer();
	~gTimer();

	float getDelta(); // !!! set last perion to current, delta = 0 !!!
protected:
	LARGE_INTEGER m_last;
	LARGE_INTEGER m_freq;
};

#endif