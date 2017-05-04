#pragma once

typedef void(*CMenuItemInvokedCallback)(int settingIndex, int value);

struct CustomMenuPref
{
	CMenuItemInvokedCallback m_callback;

	int m_value, m_resetvalue;

	CustomMenuPref(CMenuItemInvokedCallback callback, int value, int resetValue) :
		m_callback(callback),
		m_value(value),
		m_resetvalue(resetValue) {
	}
};

enum CamPresetType
{
	CPT_BOOLEAN,
	CPT_INTEGER,
	CPT_UINTEGER,
	CPT_FLOAT,
	CPT_DOUBLE
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
public:
	CamMetadataPreset() {
		memset(this, 0x0, sizeof(CamMetadataPreset));
	}
	CamMetadataPreset(eMetadataHash metadataHash, std::string name, CamPresetType type, CamPresetValue value) :
		metadataHash(metadataHash), name(name), type(type), value(value) {}
	eMetadataHash metadataHash;
	std::string name;
	CamPresetType type;
	CamPresetValue value;

	std::string toString()
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
