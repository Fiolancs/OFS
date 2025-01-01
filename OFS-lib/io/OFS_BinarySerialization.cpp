#include "OFS_BinarySerialization.h"

#include <nlohmann/json.hpp>

#include <span>
#include <string>
#include <vector>
#include <string_view>


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
