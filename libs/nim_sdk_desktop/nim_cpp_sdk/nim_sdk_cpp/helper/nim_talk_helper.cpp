#include "nim_talk_helper.h"

namespace nim
{

bool ParseMessage(const std::string& msg_json, IMMessage& message)
{
	Json::Value values;
	Json::Reader reader;
	if (reader.parse(msg_json, values) && values.isObject())
	{
		ParseMessage(values, message);
		return true;
	}
	return false;
}

bool ParseReceiveMessage(const std::string& msg_json, IMMessage& message)
{
	Json::Value values;
	Json::Reader reader;
	if (reader.parse(msg_json, values) && values.isObject())
	{
		message.rescode_ = (NIMResCode)values[kNIMMsgKeyLocalRescode].asUInt();
		message.feature_ = (NIMMessageFeature)values[kNIMMsgKeyLocalMsgFeature].asUInt();

		ParseMessage(values[kNIMMsgKeyLocalReceiveMsgContent], message);
		return true;
	}
	
	return false;
}

void ParseMessage(const Json::Value& msg_json, IMMessage& message)
{
	message.session_type_ = (NIMSessionType)msg_json[kNIMMsgKeyToType].asUInt();
	message.receiver_accid_ = msg_json[kNIMMsgKeyToAccount].asString();
	message.sender_accid_ = msg_json[kNIMMsgKeyFromAccount].asString();
	message.readonly_sender_client_type_ = msg_json[kNIMMsgKeyFromClientType].asUInt();
	message.readonly_sender_device_id_ = msg_json[kNIMMsgKeyFromDeviceId].asString();
	message.readonly_sender_nickname_ = msg_json[kNIMMsgKeyFromNick].asString();
	message.timetag_ = msg_json[kNIMMsgKeyTime].asUInt64();
	message.type_ = (NIMMessageType)msg_json[kNIMMsgKeyType].asUInt();
	message.content_ = msg_json[kNIMMsgKeyBody].asString();
	message.attach_ = msg_json[kNIMMsgKeyAttach].asString();
	message.client_msg_id_ = msg_json[kNIMMsgKeyClientMsgid].asString();

	message.status_ = (NIMMsgLogStatus)msg_json[kNIMMsgKeyLocalLogStatus].asUInt();
	message.sub_status_ = (NIMMsgLogSubStatus)msg_json[kNIMMsgKeyLocalLogSubStatus].asUInt();
}

}