#ifndef _NIM_SDK_CPP_TALK_HELPER_H_
#define _NIM_SDK_CPP_TALK_HELPER_H_

#include <string>
#include <list>
#include <functional>
#include "json.h"

namespace nim
{
/** @brief Talk 辅助方法和数据结构定义
  * @copyright (c) 2015, NetEase Inc. All rights reserved
  * @author Oleg
  * @date 2015/10/16
  */

#include "nim_talk_def.h"
#include "nim_session_def.h"
#include "nim_msglog_def.h"
#include "nim_res_code_def.h"

struct IMMessage
{
public:
	NIMResCode	rescode_;
	NIMMessageFeature	feature_;

public:
	NIMSessionType session_type_;
	std::string	   receiver_accid_;
	std::string    sender_accid_;
	__int64		   timetag_;
	std::string	   content_;
	NIMMessageType type_;
	std::string	   attach_;
	std::string	   client_msg_id_;
	bool		   resend_flag_;
	bool		   support_cloud_history_;
	bool		   support_roam_msg_;
	bool		   support_sync_msg_;

public:
	std::string	   local_res_path_;
	std::string	   local_talk_id_;
	std::string	   local_res_id_;
	NIMMsgLogStatus	status_;
	NIMMsgLogSubStatus	sub_status_;

public:
	int			   readonly_sender_client_type_;
	std::string	   readonly_sender_device_id_;
	std::string	   readonly_sender_nickname_;
	__int64		   readonly_server_id_;

	IMMessage() : resend_flag_(false)
				, support_cloud_history_(true)
				, support_roam_msg_(true)
				, support_sync_msg_(true)
				, readonly_sender_client_type_(0) 
				, readonly_server_id_(0)
				, feature_(kNIMMessageFeatureDefault)
				, session_type_(kNIMSessionTypeP2P)
				, timetag_(0)
				, status_(nim::kNIMMsgLogStatusNone)
				, sub_status_(nim::kNIMMsgLogSubStatusNone) {}

	std::string		ToJsonString(bool use_to_send) const
	{
		Json::Value values;
		values[kNIMMsgKeyToType] = session_type_;
		values[kNIMMsgKeyToAccount] = receiver_accid_;
		values[kNIMMsgKeyFromAccount] = sender_accid_;
		values[kNIMMsgKeyTime] = timetag_;
		values[kNIMMsgKeyType] = type_;
		values[kNIMMsgKeyBody] = content_;
		values[kNIMMsgKeyAttach] = attach_;
		values[kNIMMsgKeyClientMsgid] = client_msg_id_;
		values[kNIMMsgKeyResendFlag] = resend_flag_ ? 1 : 0;
		values[kNIMMsgKeyHistorySave] = support_cloud_history_ ? 1 : 0;
		values[kNIMMsgKeyMsgRoaming] = support_roam_msg_ ? 1 : 0;
		values[kNIMMsgKeyMsgSync] = support_sync_msg_ ? 1 : 0;
		values[kNIMMsgKeyLocalFilePath] = local_res_path_;
		values[kNIMMsgKeyLocalTalkId] = receiver_accid_;
		values[kNIMMsgKeyLocalResId] = local_res_id_;
		values[kNIMMsgKeyLocalLogStatus] = status_;
		values[kNIMMsgKeyLocalLogSubStatus] = sub_status_;

		if (!use_to_send)
		{
			values[kNIMMsgKeyFromClientType] = readonly_sender_client_type_;
			values[kNIMMsgKeyFromDeviceId] = readonly_sender_device_id_;
			values[kNIMMsgKeyFromNick] = readonly_sender_nickname_;
			values[kNIMMsgKeyServerMsgid] = readonly_server_id_;
		}
		return values.toStyledString();
	}
};

struct IMFile
{
	std::string	md5_;
	__int64		size_;
	std::string url_;
	std::string display_name_;
	std::string file_extension_;

	IMFile() : size_(0) {}

	std::string ToJsonString(Json::Value &attach) const
	{
		//以下客户端可以选填
		if (!display_name_.empty())
			attach[kNIMFileMsgKeyDisplayName] = display_name_;
		if (!file_extension_.empty())
			attach[kNIMFileMsgKeyExt] = file_extension_;
		if (!md5_.empty())
			attach[kNIMFileMsgKeyMd5] = md5_;
		if (size_ > 0)
			attach[kNIMFileMsgKeySize] = size_;

		return attach.toStyledString();
	}

	std::string ToJsonString() const
	{
		Json::Value attach;
		attach[kNIMFileMsgKeyDisplayName] = display_name_;

		return ToJsonString(attach);
	}
};

struct IMImage : IMFile
{
	int			width_;
	int			height_;

	IMImage() : width_(0), height_(0) {}

	std::string ToJsonString() const
	{
		Json::Value attach;
		attach[kNIMImgMsgKeyWidth] = width_;
		attach[kNIMImgMsgKeyHeight] = height_;
		attach[kNIMImgMsgKeyDisplayName] = display_name_;

		return __super::ToJsonString(attach);
	}
};

struct IMLocation
{
	std::string	description_;
	double		latitude_;
	double		longitude_;

	IMLocation() : latitude_(0), longitude_(0) {}

	std::string ToJsonString() const
	{
		Json::Value attach;
		attach[kNIMLocationMsgKeyTitle] = description_;
		attach[kNIMLocationMsgKeyLatitude] = latitude_;
		attach[kNIMLocationMsgKeyLongitude] = longitude_;

		return attach.toStyledString();
	}
};

struct IMAudio : IMFile
{
	int			duration_;

	IMAudio() : duration_(0) {}

	std::string ToJsonString() const
	{
		Json::Value attach;
		attach[kNIMAudioMsgKeyDisplayName] = display_name_;

		return __super::ToJsonString(attach);
	}
};

struct IMVideo : IMFile
{
	int			duration_;
	int			width_;
	int			height_;

	IMVideo() : duration_(0), width_(0), height_(0) {}

	std::string ToJsonString() const
	{
		Json::Value attach;
		attach[kNIMVideoMsgKeyWidth] = width_;
		attach[kNIMVideoMsgKeyHeight] = height_;
		attach[kNIMVideoMsgKeyDuration] = duration_;

		return __super::ToJsonString(attach);
	}
};

bool ParseMessage(const std::string& msg_json, IMMessage& message);
bool ParseReceiveMessage(const std::string& msg_json, IMMessage& message);
void ParseMessage(const Json::Value& msg_json, IMMessage& message);

} //namespace nim

#endif //_NIM_SDK_CPP_TALK_HELPER_H_