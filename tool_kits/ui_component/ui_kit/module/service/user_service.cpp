#include "user_service.h"
#include "module/local/local_helper.h"
#include "shared/xml_util.h"
#include "module/login/login_manager.h"


std::string GetConfigValue(const std::string& key)
{
	std::string value;
	std::wstring server_conf_path = QPath::GetAppPath();
	server_conf_path.append(L"server_conf.txt");
	TiXmlDocument document;
	if (shared::LoadXmlFromFile(document, server_conf_path))
	{
		TiXmlElement* root = document.RootElement();
		if (root)
		{
			Json::Value srv_config;
			if (auto pchar = root->Attribute(key.c_str()))
			{
				value = pchar;
			}
		}
	}

	return value;
}

namespace nim_comp
{

UserService::UserService()
{
	//向SDK注册监听好友列表变化
	nim::Friend::RegChangeCb(nbase::Bind(&UserService::OnFriendListChange, this, std::placeholders::_1));

	//向SDK注册监听用户名片变化
	nim::User::RegUserNameCardChangedCb(nbase::Bind(&UserService::OnUserInfoChange, this, std::placeholders::_1));
}

void UserService::OnFriendListChange(const nim::FriendChangeEvent& change_event)
{
	std::list<std::string> add_list;
	std::list<std::string> delete_list;

	switch (change_event.type_)
	{
	case nim::kNIMFriendChangeTypeDel:
	{
		nim::FriendDelEvent del_event;
		nim::Friend::ParseFriendDelEvent(change_event, del_event);
		delete_list.push_back(del_event.accid_);
		friend_list_.erase(del_event.accid_); // 从friend_list_删除
		break;
	}
	case nim::kNIMFriendChangeTypeRequest:
	{
		nim::FriendAddEvent add_event;
		nim::Friend::ParseFriendAddEvent(change_event, add_event);
		if (add_event.add_type_ == nim::kNIMVerifyTypeAdd || add_event.add_type_ == nim::kNIMVerifyTypeAgree)
		{
			add_list.push_back(add_event.accid_);
			friend_list_.insert(add_event.accid_);
		}
		break;
	}
	case nim::kNIMFriendChangeTypeSyncList:
	{
		nim::FriendProfileSyncEvent sync_event;
		nim::Friend::ParseFriendProfileSyncEvent(change_event, sync_event);
		for (auto& info : sync_event.profiles_)
		{
			if (info.GetRelationship() == nim::kNIMFriendFlagNormal)
			{
				add_list.push_back(info.GetAccId());
				friend_list_.insert(info.GetAccId());
			}
			else
			{
				delete_list.push_back(info.GetAccId());
				friend_list_.erase(info.GetAccId()); // 从friend_list_删除
			}
		}
		break;
	}
	default:
		break;
	}

	if (!add_list.empty())
	{
		OnGetUserInfoCallback cb = ToWeakCallback([this](std::list<nim::UserNameCard> uinfos) {
			for (auto iter = uinfos.cbegin(); iter != uinfos.cend(); iter++)
				InvokeFriendListChangeCallback(kChangeTypeAdd, *iter);
		});
		GetUserInfoWithEffort(add_list, cb);
	}
	if (!delete_list.empty())
	{
		OnGetUserInfoCallback cb = ToWeakCallback([this](std::list<nim::UserNameCard> uinfos) {
			for (auto iter = uinfos.cbegin(); iter != uinfos.cend(); iter++)
				InvokeFriendListChangeCallback(kChangeTypeDelete, *iter);
		});
		GetUserInfoWithEffort(delete_list, cb);
	}
}

void UserService::OnUserInfoChange(const std::list<nim::UserNameCard> &json_result)
{
	assert(nbase::MessageLoop::current()->ToUIMessageLoop());

	for (auto& info : json_result)
	{
		auto iter = all_user_.find(info.GetAccId());
		if (iter != all_user_.end())
			iter->second.Update(info);
		else
			InvokeGetUserInfo(std::list<std::string>(1, info.GetAccId()), nullptr);

		if (!info.GetIconUrl().empty())
			DownloadUserPhoto(info);
	}

	for (auto& it : uinfo_change_cb_list_) // 执行回调列表中所有回调
		(*(it.second))(json_result);
}

UnregisterCallback UserService::RegFriendListChange(const OnFriendListChangeCallback& callback)
{
	OnFriendListChangeCallback* new_callback = new OnFriendListChangeCallback(callback);
	int cb_id = (int)new_callback;
	assert(nbase::MessageLoop::current()->ToUIMessageLoop());
	friend_list_change_cb_list_[cb_id].reset(new_callback);
	auto cb = ToWeakCallback([this, cb_id]() {
		friend_list_change_cb_list_.erase(cb_id);
	});
	return cb;
}

UnregisterCallback UserService::RegUserInfoChange(const OnUserInfoChangeCallback& callback)
{
	OnUserInfoChangeCallback* new_callback = new OnUserInfoChangeCallback(callback);
	int cb_id = (int)new_callback;
	assert(nbase::MessageLoop::current()->ToUIMessageLoop());
	uinfo_change_cb_list_[cb_id].reset(new_callback);
	auto cb = ToWeakCallback([this, cb_id]() {
		uinfo_change_cb_list_.erase(cb_id);
	});
	return cb;
}

UnregisterCallback UserService::RegUserPhotoReady(const OnUserPhotoReadyCallback & callback)
{
	OnUserPhotoReadyCallback* new_callback = new OnUserPhotoReadyCallback(callback);
	int cb_id = (int)new_callback;
	assert(nbase::MessageLoop::current()->ToUIMessageLoop());
	photo_ready_cb_list_[cb_id].reset(new_callback);
	auto cb = ToWeakCallback([this, cb_id]() {
		photo_ready_cb_list_.erase(cb_id);
	});
	return cb;
}

void UserService::UIFriendListChangeCallback(UserChangeType change_type, const nim::UserNameCard& uinfo)
{
	assert(nbase::MessageLoop::current()->ToUIMessageLoop());
	for (auto& it : friend_list_change_cb_list_)
	{
		(*(it.second))(change_type, uinfo);
	}
}

void UserService::InvokeFriendListChangeCallback(UserChangeType change_type, const nim::UserNameCard& user_infos)
{
	auto task = nbase::Bind(&UserService::UIFriendListChangeCallback, this, change_type, user_infos);
	nbase::ThreadManager::PostTask(kThreadUI, task);
}

void UserService::DownloadUserPhoto(const nim::UserNameCard &info)
{
	if (info.GetIconUrl().find_first_of("http") != 0) //info.head_image不是正确的url
		return;

	std::wstring photo_path = GetUserPhotoDir() + nbase::UTF8ToUTF16(QString::GetMd5(info.GetIconUrl()));
	if (info.GetIconUrl().empty() || CheckPhotoOK(photo_path)) // 如果头像已经存在且完好，就不下载
		return;

	nim::NOS::DownloadMediaCallback cb = ToWeakCallback([this, info, photo_path](nim::NIMResCode res_code, const std::string& file_path, const std::string& call_id, const std::string& res_id) {
		if (res_code == nim::kNIMResSuccess)
		{
			std::wstring ws_file_path = nbase::UTF8ToUTF16(file_path);
			if (nbase::FilePathIsExist(ws_file_path, false))
			{
				nbase::CopyFileW(ws_file_path, photo_path);
				nbase::DeleteFile(ws_file_path);

				for (auto &it : photo_ready_cb_list_) // 执行监听头像下载的回调
					(*it.second)(info.GetAccId(), photo_path);
			}
		}
	});
	nim::NOS::DownloadResource(info.GetIconUrl(), cb);
}

void UserService::InvokeRegisterAccount(const std::string &username, const std::string &password, const std::string &nickname, const OnRegisterAccountCallback& cb)
{
	std::string addressbook_address = kAppServerAddress;
	std::string new_addressbook_address = GetConfigValue(g_AppServerAddress);
	if (!new_addressbook_address.empty())
	{
		addressbook_address = new_addressbook_address;
	}
	addressbook_address += "/api/createDemoUser";
	auto reg_cb = [cb](bool ret, int response_code, const std::string& reply)
	{
		if (ret && response_code == 200) {
			Json::Value json;
			Json::Reader reader;
			bool res = reader.parse(reply, json);
			if (!res)
			{
				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, 0, "未知错误"));
				return;
			}

			int json_res = json["res"].asInt();
			std::string err_msg = json["errmsg"].asString();
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, json_res, err_msg));
		}
		else {
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, 0, nbase::UTF16ToUTF8(L"网络出现问题，请确认网络连接")));
		}
	};

	std::string body;
	body += "username=" + username;
	body += "&password=" + password;
	body += "&nickname=" + nickname;

	std::string app_key = "45c6af3c98409b18a84451215d0bdd6e";
	std::string new_app_key = GetConfigValue(g_AppKey);
	if (!new_app_key.empty())
	{
		app_key = new_app_key;
	}
	nim_http::HttpRequest request(addressbook_address, body.c_str(), body.size(), reg_cb);
	request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
	request.AddHeader("charset", "utf-8");
	request.AddHeader("appkey", app_key);
	request.AddHeader("User-Agent", "nim_demo_pc");
	request.SetMethodAsPost();
	nim_http::PostRequest(request);
}

void UserService::InvokeGetAllUserInfo(const OnGetUserInfoCallback& cb)
{
	nim::Friend::GetList(ToWeakCallback([this, cb](nim::NIMResCode res_code, const std::list<nim::FriendProfile>& user_profile_list)
	{
		std::list<std::string> account_list;
		for (auto& it : user_profile_list)
		{
			if (it.GetRelationship() == nim::kNIMFriendFlagNormal)
				friend_list_.insert(it.GetAccId()); //插入friend_list_（类的成员变量）好友列表
			account_list.push_back(it.GetAccId());
		}
		if(!account_list.empty())
			InvokeGetUserInfo(account_list, cb); // 从db和服务器查询用户信息
	}));
}

void UserService::InvokeGetUserInfo(const std::list<std::string>& account_list, const OnGetUserInfoCallback & cb)
{
	// 先在本地db中找
	nim::User::GetUserNameCardCallback cb1 = ToWeakCallback([this, account_list, cb](const std::list<nim::UserNameCard> &json_result)
	{
		std::list<nim::UserNameCard> already_get;
		std::set<std::string> not_get_set(account_list.cbegin(), account_list.cend());
		for (auto& card : json_result)
		{
			already_get.push_back(card);
			not_get_set.erase(card.GetAccId());
			all_user_[card.GetAccId()] = card; // 插入all_user
			//能在数据库中查到用户信息，则用户头像应该以前下载过，因此此处不下载。
		}
		if (cb && !already_get.empty())
		{
			assert(nbase::MessageLoop::current()->ToUIMessageLoop());
			cb(already_get); // 执行参数传入的回调
		}
		if (not_get_set.empty()) // 全部从本地db找到，直接返回
			return;

		// 有些信息本地db没有，再从服务器获取
		std::list<std::string> not_get_list(not_get_set.cbegin(), not_get_set.cend());
		nim::User::GetUserNameCardCallback cb2 = ToWeakCallback([this, not_get_list, cb](const std::list<nim::UserNameCard> &json_result)
		{
			std::list<nim::UserNameCard> last_get;

			for (auto& card : json_result)
			{
				last_get.push_back(card);
				all_user_[card.GetAccId()] = card; // 插入all_user
				if (card.ExistValue(nim::kUserNameCardKeyIconUrl))
					DownloadUserPhoto(card); // 下载头像
			}

			if (cb)
			{
				assert(nbase::MessageLoop::current()->ToUIMessageLoop());
				cb(last_get); // 执行参数传入的回调
			}
		});
		nim::User::GetUserNameCardOnline(not_get_list, cb2);
	});
	nim::User::GetUserNameCard(account_list, cb1);
}

void UserService::GetUserInfoWithEffort(const std::list<std::string>& account_list, const OnGetUserInfoCallback& cb)
{
	std::list<nim::UserNameCard> already_get;
	std::list<std::string> not_get_list;
	for (auto accid : account_list)
	{
		auto iter = all_user_.find(accid); // 先从all_user_里面找
		if (iter != all_user_.cend())
			already_get.push_back(iter->second);
		else
			not_get_list.push_back(accid);
	}
	if (cb && !already_get.empty())
	{
		assert(nbase::MessageLoop::current()->ToUIMessageLoop());
		cb(already_get); // 执行参数传入的回调
	}
	if (not_get_list.empty()) // 全部从all_user_里面找到，直接返回
		return;

	// 有些信息不在all_user_里面，再到本地db和服务器中找
	InvokeGetUserInfo(not_get_list, cb);
}

void UserService::InvokeUpdateUserInfo(const nim::UserNameCard &new_info, const OnUpdateUserInfoCallback& cb)
{
	auto update_uinfo_cb = ToWeakCallback([this, new_info, cb](nim::NIMResCode res) {
		if (res == nim::kNIMResSuccess)
		{
			assert(nbase::MessageLoop::current()->ToUIMessageLoop());
			std::list<nim::UserNameCard> lst;
			lst.push_back(new_info);
			OnUserInfoChange(lst);
		}
		if (cb != nullptr)
			cb(res);
	});
	nim::User::UpdateUserNameCard(new_info, update_uinfo_cb);
}

void UserService::InvokeChangeUserPhoto(const std::string &url, const OnUpdateUserInfoCallback& cb)
{
	nim::UserNameCard my_info;
	my_info.SetAccId(LoginManager::GetInstance()->GetAccount());
	my_info.SetIconUrl(url);
	InvokeUpdateUserInfo(my_info, cb);
}

const std::map<std::string, nim::UserNameCard>& UserService::GetAllUserInfos()
{
	assert(nbase::MessageLoop::current()->ToUIMessageLoop());
	return all_user_;
}

bool UserService::GetUserInfo(const std::string &id, nim::UserNameCard &info)
{
	auto iter = all_user_.find(id);
	if (iter != all_user_.cend())
	{
		info = iter->second;
		return true;
	}
	else
	{
		info.SetName(id);
		info.SetAccId(id);
		InvokeGetUserInfo(std::list<std::string>(1, id), nullptr);
		return false;
	}
}

nim::NIMFriendFlag UserService::GetUserType(const std::string &id)
{
	assert(nbase::MessageLoop::current()->ToUIMessageLoop());
	return (friend_list_.find(id) != friend_list_.end() ? nim::kNIMFriendFlagNormal : nim::kNIMFriendFlagNotFriend);
}

std::wstring UserService::GetUserName(const std::string &id)
{
	nim::UserNameCard info;
	GetUserInfo(id, info);
	return nbase::UTF8ToUTF16(info.GetName());
}

std::wstring UserService::GetUserPhoto(const std::string &accid)
{
	std::wstring default_photo = QPath::GetAppPath() + L"res\\faces\\default\\default.png";
	if (!nbase::FilePathIsExist(default_photo, false))
		default_photo = L"";

	nim::UserNameCard info;
	GetUserInfo(accid, info);
	if (!info.ExistValue(nim::kUserNameCardKeyIconUrl) || info.GetIconUrl().empty())
		return default_photo;

	// 检查图片是否存在
	std::wstring photo_path = GetUserPhotoDir() + nbase::UTF8ToUTF16(QString::GetMd5(info.GetIconUrl()));
	if (!nbase::FilePathIsExist(photo_path, false))
		return default_photo;

	if (!CheckPhotoOK(photo_path))
		return default_photo;

	return GetUserPhotoDir() + nbase::UTF8ToUTF16(QString::GetMd5(info.GetIconUrl()));
}

bool UserService::CheckPhotoOK(std::wstring photo_path)
{
	if (!nbase::FilePathIsExist(photo_path, false))
		return false;

	// 检查图片是否损坏
	return (Gdiplus::Image(photo_path.c_str()).GetLastStatus() == Gdiplus::Status::Ok);
}

std::wstring UserService::GetUserPhotoDir()
{
	std::wstring photo_dir = QPath::GetUserAppDataDir(LoginManager::GetInstance()->GetAccount()).append(L"photo\\");
	if (!nbase::FilePathIsExist(photo_dir, true))
		nbase::win32::CreateDirectoryRecursively(photo_dir.c_str());
	return photo_dir;
}

}
