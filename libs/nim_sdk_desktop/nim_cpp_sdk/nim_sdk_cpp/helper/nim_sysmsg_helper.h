#ifndef _NIM_SDK_CPP_SYSMSG_HELPER_H_
#define _NIM_SDK_CPP_SYSMSG_HELPER_H_

#include <string>
#include <list>
#include <functional>
#include "json.h"

namespace nim
{
/** @brief sysmsg 辅助方法和数据结构定义
  * @copyright (c) 2015, NetEase Inc. All rights reserved
  * @author Oleg
  * @date 2015/10/20
  */

#include "nim_sysmsg_def.h"
#include "nim_msglog_def.h"
#include "nim_res_code_def.h"

struct SysMessage
{
	__int64		timetag_;
	NIMSysMsgType	type_;
	std::string	receiver_accid_;
	std::string sender_accid_;
	std::string content_;
	std::string	attach_;
	__int64		id_;
	bool		support_offline_;
	std::string	apns_text_;
	NIMSysMsgStatus	status_;

	NIMResCode	rescode_;
	NIMMessageFeature	feature_;
	int			total_unread_count_;
	std::string client_msg_id_;

	SysMessage() : timetag_(0)
		, id_(0)
		, support_offline_(true)
		, total_unread_count_(0)
		, type_(kNIMSysMsgTypeUnknown)
		, status_(kNIMSysMsgStatusNone)
		, feature_(kNIMMessageFeatureDefault) {}

	std::string	ToJsonString() const
	{
		Json::Value values;
		values[kNIMSysMsgKeyToAccount] = receiver_accid_;
		values[kNIMSysMsgKeyFromAccount] = sender_accid_;
		values[kNIMSysMsgKeyType] = type_;
		values[kNIMSysMsgKeyAttach] = attach_;
		values[kNIMSysMsgKeyMsg] = content_;
		values[kNIMSysMsgKeyLocalClientMsgId] = client_msg_id_;
		values[kNIMSysMsgKeyCustomSaveFlag] = support_offline_ ? 1 : 0;
		values[kNIMSysMsgKeyCustomApnsText] = apns_text_;
		values[kNIMSysMsgKeyTime] = timetag_;
		values[kNIMSysMsgKeyMsgId] = id_;
		values[kNIMSysMsgKeyLocalStatus] = status_;
		return values.toStyledString();
	}
};

bool ParseSysMessage(const std::string& sysmsg_json, SysMessage& msg);
bool ParseSysMessages(const std::string& sysmsgs_json, std::list<SysMessage>& msgs, int* unread);
void ParseSysMessageContent(const Json::Value& content_json, SysMessage& msg);
} //namespace nim

#endif //_NIM_SDK_CPP_SYSMSG_HELPER_H_