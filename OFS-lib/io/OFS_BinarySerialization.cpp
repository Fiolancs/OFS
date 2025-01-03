#include "OFS_BinarySerialization.h"

#include <nlohmann/json.hpp>

#include <span>
#include <string>
#include <vector>
#include <string_view>

/*
* TODO: It will have to be this way until one of the following 
*  - Glaze officially supports CBOR and we skip the JSON <> CBOR conversion 
*  - We find a portable library that directly converts a JSON string to CBOR 
*  - We change libraries from Glaze to something like reflect-cpp but CBOR support is 
*/

std::vector<char> OFS::util::convertJSONtoCBOR(std::string_view jsonString)
{
	std::vector<char> ret{};
	if (auto json = nlohmann::json::parse(jsonString, nullptr, false, true); !json.is_discarded())
	{
		nlohmann::json::to_cbor(json, ret);
	}
	return ret;
}

std::string OFS::util::convertCBORtoJSON(std::span<char> jsonString)
{
	if (auto const json = nlohmann::json::from_cbor(jsonString); !json.is_discarded())
		return json.dump();
	return {};
}
