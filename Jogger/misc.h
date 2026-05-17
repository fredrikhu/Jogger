#pragma once
#include <Windows.h>

namespace {
	HMENU ControlId(int id) {
		return reinterpret_cast<HMENU>(static_cast<INT_PTR>(id));
	}
}