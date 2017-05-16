#pragma once

typedef void(*CMenuItemInvokedCallback)(int settingIndex, int value);

typedef int(*CMenuItemUpdatedCallback)();

struct CustomMenuPref
{
	CMenuItemInvokedCallback m_callback;
	CMenuItemUpdatedCallback m_updated;

	int m_resetvalue;

	CustomMenuPref(CMenuItemInvokedCallback callback, CMenuItemUpdatedCallback updated, int resetValue) :
		m_callback(callback),
		m_updated(updated),
		m_resetvalue(resetValue) {
	}
};

union CamPresetValue
{
	bool enabled;
	int integer;
	unsigned int unsignedInt;
	float fvalue;
	double dvalue;
	const char * string;
	float vector[3];
};

struct CamMetadataPreset
{
	CamMetadataPreset() {
		memset(this, 0x0, sizeof(CamMetadataPreset));
	}
	CamMetadataPreset(eMetadataHash metadataHash, std::string name, CamPresetType type, CamPresetValue value) :
		metadataHash(metadataHash), name(name), type(type), value(value) {}
	eMetadataHash metadataHash;
	std::string name;
	CamPresetType type;
	CamPresetValue value;

	std::string toString() const
	{
		std::stringstream sstream;

		switch (type)
		{
		case CPT_BOOLEAN:
			sstream << std::boolalpha << value.enabled;
			break;
		case CPT_INTEGER:
			sstream << value.integer;
			break;
		case CPT_UINTEGER:
			sstream << value.unsignedInt;
			break;
		case CPT_FLOAT:
			sstream << value.fvalue;
			break;
		case CPT_DOUBLE:
			sstream << value.dvalue;
			break;
		default:
			break;
		}

		return sstream.str();
	}
};

struct GxtEntry
{
	const char * m_text;
	const char * m_alias;
};