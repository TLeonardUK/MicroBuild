/*
MicroBuild
Copyright (C) 2016 TwinDrills

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SCHEMA_FILE
#error No schema file defined
#endif
#ifndef SCHEMA_CLASS
#error No schema class defined
#endif

// ----------------------------------------------------------------------------

#define START_OPTION(ValueType, Group, Key, Description) \
	ValueType SCHEMA_CLASS::Get_##Group##_##Key() \
	{ \
		return m_##Group##_##Key##_value; \
	} \
	void SCHEMA_CLASS::Set_##Group##_##Key(const ValueType& value) \
	{ \
		m_##Group##_##Key##_value = value; \
		std::string converted; \
		StringCast(value, converted); \
		SetOrAddValue(#Group, #Key, converted, true); \
	} \
	bool SCHEMA_CLASS::Validate_##Group##_##Key() \
	{ \
		std::string groupName = #Group;\
		std::string keyName = #Key;\
		std::vector<std::string> options;\
		std::vector<std::string> values = GetValues(groupName, keyName); \
		ValueType& output = m_##Group##_##Key##_value; \
		bool bResult = true;

#define START_ARRAY_OPTION(ValueType, Group, Key, Description) \
	std::vector<ValueType> SCHEMA_CLASS::Get_##Group##_##Key() \
	{ \
		return m_##Group##_##Key##_value; \
	} \
	void SCHEMA_CLASS::Set_##Group##_##Key(const std::vector<ValueType>& value) \
	{ \
		m_##Group##_##Key##_value = value; \
		std::vector<std::string> result; \
		for (const ValueType& type : value) \
		{ \
			std::string convertedValue; \
			StringCast<ValueType, std::string>(type, convertedValue); \
			result.push_back(convertedValue); \
		} \
		SetOrAddValue(#Group, #Key, result, true); \
	} \
	bool SCHEMA_CLASS::Validate_##Group##_##Key() \
	{ \
		std::string groupName = #Group;\
		std::string keyName = #Key;\
		std::vector<std::string> options;\
		std::vector<std::string> values = GetValues(groupName, keyName); \
		std::vector<ValueType>& output = m_##Group##_##Key##_value; \
		output.clear(); \
		ValueType converterTemp; \
		bool bResult = true;

#define START_KEY_VALUE_ARRAY_OPTION(Group, Description) \
	std::vector<ConfigFile::KeyValuePair> SCHEMA_CLASS::Get_##Group() \
	{ \
		return m_##Group##_value; \
	} \
	void SCHEMA_CLASS::Set_##Group(const std::vector<ConfigFile::KeyValuePair>& value) \
	{ \
		UNUSED_PARAMETER(value); \
		assert(false); \
	} \
	bool SCHEMA_CLASS::Validate_##Group() \
	{ \
		m_##Group##_value = GetPairs(#Group); \

#define OPTION_RULE_REQUIRED() \
		if (values.size() == 0) \
		{ \
			ValidateError("Expected value '%s.%s'.", groupName.c_str(), keyName.c_str());\
			bResult = false; \
		} 

#define OPTION_RULE_ORDER_IMPORTANT() // Already ordered by default, this is pointless now.

#define OPTION_RULE_DEFAULT(Value) \
		if (values.size() == 0) \
		{ \
			std::string outVal; \
			StringCast<decltype(Value), std::string>(Value, outVal);\
			values.push_back(outVal); \
		} 

#define OPTION_RULE_VALIDATOR(ValidatorFunction) \
		if (ValidatorFunction(values) == 0) \
		{ \
			bResult = false; \
		} 

#define OPTION_RULE_EXPAND_PATH_WILDCARDS(bCanCache) \
		bResult = bResult && ExpandPaths(values, bCanCache);

#define OPTION_RULE_ABSOLUTE_PATH() \
		if (values.size() > 0) \
		{ \
			Platform::Path subvalue(values[values.size() - 1]); \
			if (subvalue.IsRelative()) \
			{ \
				ValidateError("Path '%s.%s' does not resolve to an absolute path. All paths must be absolute. Use tokens to expand relative paths to absolute.", groupName.c_str(), keyName.c_str()); \
				bResult = false; \
			} \
			values[values.size() - 1] = subvalue.ToString(); \
		}

#define OPTION_RULE_OPTION(Option) \
		options.push_back(#Option);

#define OPTION_RULE_NO_INHERIT() \
		SetGroupUnmergable(groupName, true);

#define END_OPTION() \
		if (options.size() > 0) \
		{ \
			bResult = bResult && ValidateOptions( \
									values, options, groupName, keyName); \
		} \
		if (values.size() >= 1) \
		{ \
			if (!StringCast(values[values.size() - 1], output)) \
			{ \
				ValidateError("Value '%s' is invalid for '%s.%s'.", values[values.size() - 1].c_str(), groupName.c_str(), keyName.c_str()); \
				bResult = false; \
			} \
		} \
		return bResult; \
	}

#define END_ARRAY_OPTION() \
		if (options.size() > 0) \
		{ \
			bResult = bResult && ValidateOptions( \
									values, options, groupName, keyName); \
		} \
		for (std::string& result : values) \
		{ \
			if (!StringCast(result, converterTemp)) \
			{ \
				ValidateError("Value '%s' is invalid for '%s.%s'.", result.c_str(), groupName.c_str(), keyName.c_str()); \
				bResult = false; \
			} \
			output.push_back(converterTemp); \
		} \
		return bResult; \
	}

#define END_KEY_VALUE_ARRAY_OPTION() \
		return true; \
	}


#define START_ENUM(Name) \
	}; /*namespace MicroBuild*/ \
	extern const char* Name##_Strings[(int)MicroBuild::Name::COUNT]; \
	bool StringConvert(const MicroBuild::Name& value, std::string& type) \
	{ \
		type = Name##_Strings[(int)value]; \
		return true; \
	} \
	bool StringConvert(const std::string& value, MicroBuild::Name& type) \
	{ \
		for (int i = 0; i < (int)MicroBuild::Name::COUNT; i++) \
		{ \
			if (MicroBuild::Strings::CaseInsensitiveEquals(Name##_Strings[i], value)) \
			{ \
				type = (MicroBuild::Name)i; \
				return true; \
			} \
		} \
		return false; \
	} \
	const char* Name##_Strings[(int)MicroBuild::Name::COUNT] = { 

#define ENUM_KEY(Name) \
		#Name,

#define END_ENUM() \
	}; \
	namespace MicroBuild {

#include SCHEMA_FILE

#undef START_OPTION
#undef START_ARRAY_OPTION
#undef START_KEY_VALUE_ARRAY_OPTION
#undef OPTION_RULE_REQUIRED
#undef OPTION_RULE_ORDER_IMPORTANT
#undef OPTION_RULE_DEFAULT
#undef OPTION_RULE_VALIDATOR
#undef OPTION_RULE_EXPAND_PATH_WILDCARDS
#undef OPTION_RULE_ABSOLUTE_PATH
#undef OPTION_RULE_OPTION
#undef OPTION_RULE_NO_INHERIT
#undef END_OPTION
#undef END_ARRAY_OPTION
#undef END_KEY_VALUE_ARRAY_OPTION
#undef START_ENUM
#undef ENUM_KEY
#undef END_ENUM

// ----------------------------------------------------------------------------

bool SCHEMA_CLASS::Validate()
{
#ifndef SCHEMA_IS_BASE
	if (!BaseConfigFile::Validate())
	{
		return false;
	}
#endif

	bool bResult = true;
#define START_OPTION(ValueType, Group, Key, Description) \
	bResult = bResult && Validate_##Group##_##Key();
#define START_ARRAY_OPTION(ValueType, Group, Key, Description) \
	bResult = bResult && Validate_##Group##_##Key();
#define START_KEY_VALUE_ARRAY_OPTION(Group, Description) \
	bResult = bResult && Validate_##Group();
#define OPTION_RULE_REQUIRED()
#define OPTION_RULE_ORDER_IMPORTANT()
#define OPTION_RULE_DEFAULT(Value)
#define OPTION_RULE_VALIDATOR(ValidatorFunction)
#define OPTION_RULE_EXPAND_PATH_WILDCARDS(bCanCache)
#define OPTION_RULE_OPTION(Option) 
#define OPTION_RULE_NO_INHERIT()
#define OPTION_RULE_ABSOLUTE_PATH()
#define END_ARRAY_OPTION()
#define END_OPTION()
#define END_KEY_VALUE_ARRAY_OPTION()
#define START_ENUM(Name) 
#define ENUM_KEY(Name) 
#define END_ENUM() 

#include SCHEMA_FILE

#undef START_OPTION
#undef START_ARRAY_OPTION
#undef START_KEY_VALUE_ARRAY_OPTION
#undef OPTION_RULE_REQUIRED
#undef OPTION_RULE_ORDER_IMPORTANT
#undef OPTION_RULE_DEFAULT
#undef OPTION_RULE_VALIDATOR
#undef OPTION_RULE_EXPAND_PATH_WILDCARDS
#undef OPTION_RULE_ABSOLUTE_PATH
#undef OPTION_RULE_OPTION
#undef OPTION_RULE_NO_INHERIT
#undef END_OPTION
#undef END_ARRAY_OPTION
#undef END_KEY_VALUE_ARRAY_OPTION
#undef START_ENUM
#undef ENUM_KEY
#undef END_ENUM

	return bResult;
}

// ----------------------------------------------------------------------------
