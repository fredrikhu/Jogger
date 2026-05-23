#pragma once
#include <Windows.h>

struct Rect {
	LONG left;
	LONG top;
	LONG width;
	LONG height;

	RECT ToRect(Rect rect) {
		return {
			.left = rect.left,
			.top = rect.top,
			.right = rect.left + rect.width,
			.bottom = rect.top + rect.height
		};
	}
};

inline Rect ToRect(RECT rect) {
	return {
		.left = rect.left,
		.top = rect.top,
		.width = rect.right - rect.left,
		.height = rect.bottom - rect.top
	};
}
