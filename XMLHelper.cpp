#include "stdafx.h"

XMLElement* XMLHelper::FirstElement(XMLElement* element, std::string elementName)
{
	return element->FirstChildElement(elementName.c_str());
}

XMLElement* XMLHelper::FindByAttribute(XMLElement* element, std::string attribute, std::string value)
{
	return FindByAttribute(element, "", attribute, value);
}

XMLElement* XMLHelper::FindByAttribute(XMLElement* element, std::string name, std::string attribute, std::string value)
{
	return FindIf(element, name, [&](XMLElement* e) -> bool {
		return !strcmp(e->Attribute(attribute.c_str()), value.c_str()); });
}
