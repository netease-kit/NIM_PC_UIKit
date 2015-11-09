#ifndef _NIM_SDK_CPP_COMMON_HELPER_H_
#define _NIM_SDK_CPP_COMMON_HELPER_H_

#include <string>
#include <list>
#include "assert.h"
#include "json.h"

namespace nim
{
/** @brief SDK¸¨Öú·½·¨
  * @copyright (c) 2015, NetEase Inc. All rights reserved
  * @author Oleg
  * @date 2015/09/08
  */

bool StrListToJsonString(const std::list<std::string>& list, std::string& out);
bool JsonStrArrayToList(const Json::Value& array_str, std::list<std::string>& out);
std::string PCharToString(const char* str);

}

#endif //_NIM_SDK_CPP_COMMON_HELPER_H_