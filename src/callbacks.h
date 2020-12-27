#pragma once

struct CamMetadataHookDef_t
{
	int globalTextId;
	int actionType;
	union ValueBounds {
		struct {
			int min;
			int max;
		} integer;
		struct {
			float min;
			float max;
		} decimal;
	};
};