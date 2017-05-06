#pragma once

#include "inc/enums.h"
#include "inc/types.h"
#include "inc/natives.h"
#include "inc/main.h"

#pragma region vehicle hash map
static std::map<DWORD, std::string> vehiclehash_map = {
	{ 0x3D8FA25C, "ninef" },
	{ 0xA8E38B01, "ninef2" },
	{ 0xEB70965F, "blista" },
	{ 0x94204D89, "asea" },
	{ 0x9441D8D5, "asea2" },
	{ 0x1F3D44B5, "boattrailer" },
	{ 0xD577C962, "bus" },
	{ 0xB8081009, "armytanker" },
	{ 0xA7FF33F5, "armytrailer" },
	{ 0x9E6B14D6, "armytrailer2" },
	{ 0xEF2295C9, "suntrap" },
	{ 0x84718D34, "coach" },
	{ 0x4C80EB0E, "airbus" },
	{ 0x8E9254FB, "asterope" },
	{ 0x5D0AAC8F, "airtug" },
	{ 0x45D56ADA, "ambulance" },
	{ 0xCEEA3F4B, "barracks" },
	{ 0x4008EABB, "barracks2" },
	{ 0xCFCA3668, "baller" },
	{ 0x8852855, "baller2" },
	{ 0x32B29A4B, "bjxl" },
	{ 0xC1E908D2, "banshee" },
	{ 0x7A61B330, "benson" },
	{ 0x432AA566, "bfinjection" },
	{ 0x32B91AE8, "biff" },
	{ 0x8125BCF9, "blazer" },
	{ 0xFD231729, "blazer2" },
	{ 0xB44F0582, "blazer3" },
	{ 0xFEFD644F, "bison" },
	{ 0x7B8297C5, "bison2" },
	{ 0x67B3F020, "bison3" },
	{ 0x898ECCEA, "boxville" },
	{ 0xF21B33BE, "boxville2" },
	{ 0x7405E08, "boxville3" },
	{ 0x3FC5D440, "bobcatxl" },
	{ 0xAA699BB6, "bodhi2" },
	{ 0xD756460C, "buccaneer" },
	{ 0xEDD516C6, "buffalo" },
	{ 0x2BEC3CBE, "buffalo2" },
	{ 0x7074F39D, "bulldozer" },
	{ 0x9AE6DDA1, "bullet" },
	{ 0xF7004C86, "blimp" },
	{ 0xAFBB2CA4, "burrito" },
	{ 0xC9E8FF76, "burrito2" },
	{ 0x98171BD3, "burrito3" },
	{ 0x353B561D, "burrito4" },
	{ 0x437CF2A0, "burrito5" },
	{ 0x779F23AA, "cavalcade" },
	{ 0xD0EB2BE5, "cavalcade2" },
	{ 0x1B38E955, "policet" },
	{ 0x97FA4F36, "gburrito" },
	{ 0xC6C3242D, "cablecar" },
	{ 0x44623884, "caddy" },
	{ 0xDFF0594C, "caddy2" },
	{ 0x6FD95F68, "camper" },
	{ 0x7B8AB45F, "carbonizzare" },
	{ 0xB1D95DA0, "cheetah" },
	{ 0xC1AE4D16, "comet2" },
	{ 0x13B57D8A, "cogcabrio" },
	{ 0x67BC037, "coquette" },
	{ 0xC3FBA120, "cutter" },
	{ 0xA3FC0F4D, "gresley" },
	{ 0xBC993509, "dilettante" },
	{ 0x64430650, "dilettante2" },
	{ 0x9CF21E0F, "dune" },
	{ 0x1FD824AF, "dune2" },
	{ 0x239E390, "hotknife" },
	{ 0x698521E3, "dloader" },
	{ 0x462FE277, "dubsta" },
	{ 0xE882E5F6, "dubsta2" },
	{ 0x810369E2, "dump" },
	{ 0x9A5B1DCC, "rubble" },
	{ 0xCB44B1CA, "docktug" },
	{ 0x4CE68AC, "dominator" },
	{ 0xD7278283, "emperor" },
	{ 0x8FC3AADC, "emperor2" },
	{ 0xB5FCF74E, "emperor3" },
	{ 0xB2FE5CF9, "entityxf" },
	{ 0xFFB15B5E, "exemplar" },
	{ 0xDE3D9D22, "elegy2" },
	{ 0xDCBCBE48, "f620" },
	{ 0x432EA949, "fbi" },
	{ 0x9DC66994, "fbi2" },
	{ 0xE8A8BDA8, "felon" },
	{ 0xFAAD85EE, "felon2" },
	{ 0x8911B9F5, "feltzer2" },
	{ 0x73920F8E, "firetruk" },
	{ 0x50B0215A, "flatbed" },
	{ 0x58E49664, "forklift" },
	{ 0xBC32A33B, "fq2" },
	{ 0x1DC0BA53, "fusilade" },
	{ 0x71CB2FFB, "fugitive" },
	{ 0x7836CE2F, "futo" },
	{ 0x9628879C, "granger" },
	{ 0x94B395C5, "gauntlet" },
	{ 0x34B7390F, "habanero" },
	{ 0x5A82F9AE, "hauler" },
	{ 0x1A7FCEFA, "handler" },
	{ 0x18F25AC7, "infernus" },
	{ 0xB3206692, "ingot" },
	{ 0x34DD8AA1, "intruder" },
	{ 0xB9CB3B69, "issi2" },
	{ 0xDAC67112, "jackal" },
	{ 0xF8D48E7A, "journey" },
	{ 0x3EAB5555, "jb700" },
	{ 0x206D1B68, "khamelion" },
	{ 0x4BA4E8DC, "landstalker" },
	{ 0x1BF8D381, "lguard" },
	{ 0x81634188, "manana" },
	{ 0x36848602, "mesa" },
	{ 0xD36A4B44, "mesa2" },
	{ 0x84F42E51, "mesa3" },
	{ 0x132D5A1A, "crusader" },
	{ 0xED7EADA4, "minivan" },
	{ 0xD138A6BB, "mixer" },
	{ 0x1C534995, "mixer2" },
	{ 0xE62B361B, "monroe" },
	{ 0x6A4BD8F6, "mower" },
	{ 0x35ED670B, "mule" },
	{ 0xC1632BEB, "mule2" },
	{ 0x506434F6, "oracle" },
	{ 0xE18195B2, "oracle2" },
	{ 0x21EEE87D, "packer" },
	{ 0xCFCFEB3B, "patriot" },
	{ 0x885F3671, "pbus" },
	{ 0xE9805550, "penumbra" },
	{ 0x6D19CCBC, "peyote" },
	{ 0x809AA4CB, "phantom" },
	{ 0x831A21D5, "phoenix" },
	{ 0x59E0FBF3, "picador" },
	{ 0x7DE35E7D, "pounder" },
	{ 0x79FBB0C5, "police" },
	{ 0x8A63C7B9, "police4" },
	{ 0x9F05F101, "police2" },
	{ 0x71FA16EA, "police3" },
	{ 0xA46462F7, "policeold1" },
	{ 0x95F4C618, "policeold2" },
	{ 0xF8DE29A8, "pony" },
	{ 0x38408341, "pony2" },
	{ 0xA988D3A2, "prairie" },
	{ 0x2C33B46E, "pranger" },
	{ 0x8FB66F9B, "premier" },
	{ 0xBB6B404F, "primo" },
	{ 0x153E1B0A, "proptrailer" },
	{ 0x6210CBB0, "rancherxl" },
	{ 0x7341576B, "rancherxl2" },
	{ 0x8CB29A14, "rapidgt" },
	{ 0x679450AF, "rapidgt2" },
	{ 0x9D96B45B, "radi" },
	{ 0xD83C13CE, "ratloader" },
	{ 0xB802DD46, "rebel" },
	{ 0xFF22D208, "regina" },
	{ 0x8612B64B, "rebel2" },
	{ 0xBE819C63, "rentalbus" },
	{ 0xF26CEFF9, "ruiner" },
	{ 0x4543B74D, "rumpo" },
	{ 0x961AFEF7, "rumpo2" },
	{ 0x2EA68690, "rhino" },
	{ 0xB822A1AA, "riot" },
	{ 0xCD935EF9, "ripley" },
	{ 0x7F5C91F1, "rocoto" },
	{ 0x2560B2FC, "romero" },
	{ 0x9B909C94, "sabregt" },
	{ 0xDC434E51, "sadler" },
	{ 0x2BC345D1, "sadler2" },
	{ 0xB9210FD0, "sandking" },
	{ 0x3AF8C345, "sandking2" },
	{ 0xB52B5113, "schafter2" },
	{ 0xD37B7976, "schwarzer" },
	{ 0x9A9FD3DF, "scrap" },
	{ 0x48CECED3, "seminole" },
	{ 0x50732C82, "sentinel" },
	{ 0x3412AE2D, "sentinel2" },
	{ 0xBD1B39C3, "zion" },
	{ 0xB8E2AE18, "zion2" },
	{ 0x4FB1A214, "serrano" },
	{ 0x9BAA707C, "sheriff" },
	{ 0x72935408, "sheriff2" },
	{ 0xCFB3870C, "speedo" },
	{ 0x2B6DC64A, "speedo2" },
	{ 0xA7EDE74D, "stanier" },
	{ 0x5C23AF9B, "stinger" },
	{ 0x82E499FA, "stingergt" },
	{ 0x6827CF72, "stockade" },
	{ 0xF337AB36, "stockade3" },
	{ 0x66B4FC45, "stratum" },
	{ 0x39DA2754, "sultan" },
	{ 0x42F2ED16, "superd" },
	{ 0x16E478C1, "surano" },
	{ 0x29B0DA97, "surfer" },
	{ 0xB1D80E06, "surfer2" },
	{ 0x8F0E3594, "surge" },
	{ 0x744CA80D, "taco" },
	{ 0xC3DDFDCE, "tailgater" },
	{ 0xC703DB5F, "taxi" },
	{ 0x72435A19, "trash" },
	{ 0x61D6BA8C, "tractor" },
	{ 0x843B73DE, "tractor2" },
	{ 0x562A97BD, "tractor3" },
	{ 0x3CC7F596, "graintrailer" },
	{ 0xE82AE656, "baletrailer" },
	{ 0x2E19879, "tiptruck" },
	{ 0xC7824E5E, "tiptruck2" },
	{ 0x1BB290BC, "tornado" },
	{ 0x5B42A5C4, "tornado2" },
	{ 0x690A4153, "tornado3" },
	{ 0x86CF7CDD, "tornado4" },
	{ 0x73B1C3CB, "tourbus" },
	{ 0xB12314E0, "towtruck" },
	{ 0xE5A2D6C6, "towtruck2" },
	{ 0x1ED0A534, "utillitruck" },
	{ 0x34E6BF6B, "utillitruck2" },
	{ 0x7F2153DF, "utillitruck3" },
	{ 0x1F3766E3, "voodoo2" },
	{ 0x69F06B57, "washington" },
	{ 0x8B13F083, "stretch" },
	{ 0x3E5F6B8, "youga" },
	{ 0x2D3BD401, "ztype" },
	{ 0x2EF89E46, "sanchez" },
	{ 0xA960B13E, "sanchez2" },
	{ 0xF4E1AA15, "scorcher" },
	{ 0x4339CD69, "tribike" },
	{ 0xB67597EC, "tribike2" },
	{ 0xE823FB48, "tribike3" },
	{ 0xCE23D3BF, "fixter" },
	{ 0x1ABA13B5, "cruiser" },
	{ 0x43779C54, "bmx" },
	{ 0xFDEFAEC3, "policeb" },
	{ 0x63ABADE7, "akuma" },
	{ 0xABB0C0, "carbonrs" },
	{ 0x806B9CC3, "bagger" },
	{ 0xF9300CC5, "bati" },
	{ 0xCADD5D2D, "bati2" },
	{ 0xCABD11E8, "ruffian" },
	{ 0x77934CEE, "daemon" },
	{ 0x9C669788, "double" },
	{ 0xC9CEAF06, "pcj" },
	{ 0xF79A00F7, "vader" },
	{ 0xCEC6B9B7, "vigero" },
	{ 0x350D1AB, "faggio2" },
	{ 0x11F76C14, "hexer" },
	{ 0x31F0B376, "annihilator" },
	{ 0x2F03547B, "buzzard" },
	{ 0x2C75F0DD, "buzzard2" },
	{ 0xFCFCB68B, "cargobob" },
	{ 0x60A7EA10, "cargobob2" },
	{ 0x53174EEF, "cargobob3" },
	{ 0x3E48BF23, "skylift" },
	{ 0x1517D4D9, "polmav" },
	{ 0x9D0450CA, "maverick" },
	{ 0xDA288376, "nemesis" },
	{ 0x2C634FBD, "frogger" },
	{ 0x742E9AC0, "frogger2" },
	{ 0xD9927FE3, "cuban800" },
	{ 0x39D6779E, "duster" },
	{ 0x81794C70, "stunt" },
	{ 0x97E55D11, "mammatus" },
	{ 0x3F119114, "jet" },
	{ 0xB79C1BF5, "shamal" },
	{ 0x250B0C5E, "luxor" },
	{ 0x761E2AD3, "titan" },
	{ 0xB39B0AE6, "lazer" },
	{ 0x15F27762, "cargoplane" },
	{ 0x17DF5EC2, "squalo" },
	{ 0xC1CE1183, "marquis" },
	{ 0x3D961290, "dinghy" },
	{ 0x107F392C, "dinghy2" },
	{ 0x33581161, "jetmax" },
	{ 0xE2E7D4AB, "predator" },
	{ 0x1149422F, "tropic" },
	{ 0xC2974024, "seashark" },
	{ 0xDB4388E4, "seashark2" },
	{ 0x2DFF622F, "submersible" },
	{ 0xCBB2BE0E, "trailers" },
	{ 0xA1DA3C91, "trailers2" },
	{ 0x8548036D, "trailers3" },
	{ 0x967620BE, "tvtrailer" },
	{ 0x174CB172, "raketrailer" },
	{ 0xD46F4737, "tanker" },
	{ 0x782A236D, "trailerlogs" },
	{ 0x7BE032C6, "tr2" },
	{ 0x6A59902D, "tr3" },
	{ 0x7CAB34D0, "tr4" },
	{ 0xAF62F6B2, "trflat" },
	{ 0x2A72BEAB, "trailersmall" },
	{ 0x9C429B6A, "velum" },
	{ 0xB779A091, "adder" },
	{ 0x9F4B77BE, "voltic" },
	{ 0x142E0DC3, "vacca" },
	{ 0xEB298297, "bifta" },
	{ 0xDC60D2B, "speeder" },
	{ 0x58B3979C, "paradise" },
	{ 0x5852838, "kalahari" },
	{ 0xB2A716A3, "jester" },
	{ 0x185484E1, "turismor" },
	{ 0x4FF77E37, "vestra" },
	{ 0x2DB8D1AA, "alpha" },
	{ 0x1D06D681, "huntley" },
	{ 0x6D6F8F43, "thrust" },
	{ 0xF77ADE32, "massacro" },
	{ 0xDA5819A3, "massacro2" },
	{ 0xAC5DF515, "zentorno" },
	{ 0xB820ED5E, "blade" },
	{ 0x47A6BC1, "glendale" },
	{ 0xE644E480, "panto" },
	{ 0x404B6381, "pigalle" },
	{ 0x51D83328, "warrener" },
	{ 0x322CF98F, "rhapsody" },
	{ 0xB6410173, "dubsta3" },
	{ 0xCD93A7DB, "monster" },
	{ 0x2C509634, "sovereign" },
	{ 0xF683EACA, "innovation" },
	{ 0x4B6C568A, "hakuchou" },
	{ 0xBF1691E0, "furoregt" },
	{ 0x9D80F93, "miljet" },
	{ 0x3C4E2113, "coquette2" },
	{ 0x6FF6914, "btype" },
	{ 0xE2C013E, "buffalo3" },
	{ 0xC96B73D9, "dominator2" },
	{ 0x14D22159, "gauntlet2" },
	{ 0x49863E9C, "marshall" },
	{ 0x2B26F456, "dukes" },
	{ 0xEC8F7094, "dukes2" },
	{ 0x72A4C31E, "stalion" },
	{ 0xE80F67EE, "stalion2" },
	{ 0x3DEE5EDA, "blista2" },
	{ 0xDCBC1C3B, "blista3" },
	{ 0xCA495705, "dodo" },
	{ 0xC07107EE, "submersible2" },
	{ 0x39D6E83F, "hydra" },
	{ 0x9114EADA, "insurgent" },
	{ 0x7B7E56F0, "insurgent2" },
	{ 0x83051506, "technical" },
	{ 0xFB133A17, "savage" },
	{ 0xA09E15FD, "valkyrie" },
	{ 0xAE2BFE94, "kuruma" },
	{ 0x187D938D, "kuruma2" },
	{ 0xBE0E6126, "jester2" },
	{ 0x3822BDFE, "casco" },
	{ 0x403820E8, "velum2" },
	{ 0x825A9F4C, "guardian" },
	{ 0x6882FA73, "enduro" },
	{ 0x26321E67, "lectro" },
	{ 0x2B7F9DE3, "slamvan" },
	{ 0x31ADBBFC, "slamvan2" },
	{ 0xDCE1D9F7, "ratloader2" },
	{ 0x4019CB4C, "swift2" },
	{ 0xB79F589E, "luxor2" },
	{ 0xA29D6D10, "feltzer3" },
	{ 0x767164D6, "osiris" },
	{ 0xE2504942, "virgo" },
	{ 0x5E4327C8, "windsor" },
	{ 0x6CBD1D6D, "besra" },
	{ 0xEBC24DF2, "swift" },
	{ 0xDB6B4924, "blimp2" },
	{ 0xAF599F01, "vindicator" },
	{ 0x3FD5AA2F, "toro" },
	{ 0x6322B39A, "t20" },
	{ 0x2EC385FE, "coquette3" },
	{ 0x14D69010, "chino" },
	{ 0xA7CE1BC5, "brawler" },
	{ 0xC397F748, "buccaneer2" },
	{ 0xAED64A63, "chino2" },
	{ 0x81A9CDDF, "faction" },
	{ 0x95466BDB, "faction2" },
	{ 0x1F52A43F, "moonbeam" },
	{ 0x710A2B9B, "moonbeam2" },
	{ 0x86618EDA, "primo2" },
	{ 0x779B4F2D, "voodoo" },
	{ 0x7B47A6A7, "lurcher" },
	{ 0xCE6B35A4, "btype2" },
	{ 0x6FF0F727, "baller3" },
	{ 0x25CBE2E2, "baller4" },
	{ 0x1C09CF5E, "baller5" },
	{ 0x27B4E6B0, "baller6" },
	{ 0x78BC1A3C, "cargobob4" },
	{ 0x360A438E, "cog55" },
	{ 0x29FCD3E4, "cog552" },
	{ 0x86FE0B60, "cognoscenti" },
	{ 0xDBF2D57A, "cognoscenti2" },
	{ 0x33B47F96, "dinghy4" },
	{ 0xF92AEC4D, "limo2" },
	{ 0x9CFFFC56, "mamba" },
	{ 0x8C2BD0DC, "nightshade" },
	{ 0xA774B5A6, "schafter3" },
	{ 0x58CF185C, "schafter4" },
	{ 0xCB0E7CD9, "schafter5" },
	{ 0x72934BE4, "schafter6" },
	{ 0xED762D49, "seashark3" },
	{ 0x1A144F2A, "speeder2" },
	{ 0x2A54C47D, "supervolito" },
	{ 0x9C5E5644, "supervolito2" },
	{ 0x362CAC6D, "toro2" },
	{ 0x56590FE9, "tropic2" },
	{ 0x5BFA5C4B, "valkyrie2" },
	{ 0x41B77FA4, "verlierer2" },
	{ 0x39F9C898, "tampa" },
	{ 0x25C5AF13, "banshee2" },
	{ 0xEE6024BC, "sultanrs" }
};
#pragma endregion

inline bool getVehicleModelName(DWORD vehicleHash, std::string& str)
{
	auto it = vehiclehash_map.find(vehicleHash);
	if (it != vehiclehash_map.end())
	{
		str = it->second;
		return true;
	}
	str = std::to_string(vehicleHash);
	return false;
}

inline std::string getEntityName(Entity entity)
{
	auto address = getScriptHandleBaseAddress(entity);

	// probably a better way to get this...
	if (*(BYTE*)(address + 0xC0) & 0x40)
	{
		auto fragInst = *reinterpret_cast<uintptr_t*>(address + 0x30);

		if (fragInst)
		{
			auto fragType = *reinterpret_cast<uintptr_t*>(fragInst + 0x78);

			if (fragType)
			{
				auto text = *(char**)(fragType + 0x58);

				if (text)
				{
					auto cppstr = std::string(text);

					auto off = cppstr.find_first_of('/');

					return cppstr.substr(off + 1, cppstr.size() - off);
				}
			}
		}
	}

	return "";
}
	

inline unsigned int getHashKey(const char * str)
{
	unsigned int hash = 0;
	for (int i = 0; i < strlen(str); ++i)
	{
		hash += str[i];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}
	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);
	return hash;
}

inline unsigned int parseHashString(const char * str)
{
	char * p;
	unsigned long numericHash = strtoul(str, &p, 10);
	return *p != '\0' ? getHashKey(str) : static_cast<unsigned int>(numericHash);
}

inline void notifyAboveMap(char* message)
{
	UI::_SET_NOTIFICATION_TEXT_ENTRY("STRING");
	UI::_ADD_TEXT_COMPONENT_STRING(message);
	UI::_DRAW_NOTIFICATION(0, 1);
}

/*inline void showSubtitle(const char * msg, int duration = 5000)
{
	UI::_SET_TEXT_ENTRY_2("CELL_EMAIL_BCON");

	const unsigned int maxStringLength = 99;

	char subStr[maxStringLength];

	for (unsigned int i = 0; i < strlen(msg); i += maxStringLength)
	{
		memcpy_s(subStr, sizeof(subStr), &msg[i], min(maxStringLength - 1, strlen(msg) - i));

		subStr[maxStringLength - 1] = '\0';

		UI::_ADD_TEXT_COMPONENT_STRING(subStr);
	}

	UI::_DRAW_SUBTITLE_TIMED(duration, 1);
}*/
