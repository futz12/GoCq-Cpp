#include "pch.h"
#include "Cq.h"

using namespace locale::conv;

namespace GoCq
{
	Cq::Cq(std::string server, std::string port)
	{
		api = new Api_For_GoCQ(server, port);
		_th_get_event = new std::thread(&Cq::_get_event, this);
	}

	Cq::~Cq()
	{
		delete api;
		delete _th_get_event;
	}

	void Cq::_get_event()
	{
		while (api->IsAlive())
		{
			auto _event = api->GetEvent();
			while (!_event.empty())
			{
				ptree pt;
				std::stringstream ss(_event);
				
				try
				{
					read_json(ss, pt);
					auto post_type = pt.get<std::string>("post_type");
					if (post_type == "message")
					{
						auto msg_type = pt.get<std::string>("message_type");
						if (msg_type == "private") { _process_private_msg(pt); }
						else if (msg_type == "group") { _process_group_msg(pt); }
					}
					else if (post_type == "notice")
					{
						auto notice_type = pt.get<std::string>("notice_type");
						if (notice_type == "group_increase") { _process_group_increase(pt); }
					}
				}
				catch (const std::exception& e)
				{
					std::cerr << e.what() << std::endl;
				}

				_event = api->GetEvent();
			}
			SLEEP(50);
		}
	}

	void Cq::_process_private_msg(ptree_helper h)
	{
		PRIVATE_MESSAGE_INFO info;
		info.time = h.get<int64_t>("time");
		info.self_id = h.get<int64_t>("self_id");
		info.post_type = "message";
		info.message_type = "private";
		info.sub_type = h.get<std::string>("sub_type");
		info.temp_source = h.get<int>("temp_source", -1);
		info.message_id = h.get<int32_t>("message_id");
		info.user_id = h.get<int64_t>("user_id");
		info.msg = /*locale::conv::from_utf(*/h.get<message>("message")/*, "GBK")*/;
		info.raw_message = h.get_conv("raw_message");
		info.font = h.get<int32_t>("font");
		h = h.pt().get_child("sender");
		info.sender.user_id = h.get<int64_t>("user_id");
		info.sender.nickname = /*locale::conv::from_utf(*/h.get<std::string>("nickname")/*, "GBK")*/;
		info.sender.sex = h.get<std::string>("sex");
		info.sender.age = h.get<int32_t>("age");
		on_private_msg(info);
	}

	void Cq::_process_group_msg(ptree_helper h)
	{
		GROUP_MESSAGE_INFO info;
		info.time = h.get<int64_t>("time");
		info.self_id = h.get<int64_t>("self_id");
		info.post_type = "message";
		info.message_type = "group";
		info.sub_type = h.get<std::string>("sub_type");
		info.message_id = h.get<int32_t>("message_id");
		info.group_id = h.get<int64_t>("group_id");
		info.user_id = h.get<int64_t>("user_id");
		ptree_helper anonymous = h.pt().get_child("anonymous");
		info.anonymous.id = anonymous.get<int64_t>("id");
		info.anonymous.name = anonymous.get_conv("name");
		info.anonymous.flag = anonymous.get<std::string>("flag");
		info.msg = h.get_conv("message");
		//info.msg = h.get<std::string>("message");
		info.raw_message = h.get_conv("raw_message");
		info.font = h.get<int32_t>("font");
		ptree_helper sender = h.pt().get_child("sender");
		info.sender.user_id = sender.get<int64_t>("user_id");
		info.sender.nickname = sender.get_conv("nickname");
		info.sender.card = sender.get_conv("card");
		info.sender.sex = sender.get<std::string>("sex");
		info.sender.age = sender.get<int32_t>("age");
		info.sender.area = sender.get_conv("area");
		info.sender.level = sender.get<std::string>("level");
		info.sender.role = sender.get<std::string>("role");
		info.sender.title = sender.get_conv("title");
		on_group_msg(info);
	}

	void Cq::_process_group_increase(ptree_helper h)
	{
		GROUP_MEMBER_INCREASE_INFO info;
		h.get2(info.time, "time");
		h.get2(info.self_id, "self_id");
		h.get2(info.post_type, "post_type");
		h.get2(info.notice_type, "notice_type");
		h.get2(info.sub_type, "sub_type");
		h.get2(info.group_id, "group_id");
		h.get2(info.operator_id, "operator_id");
		h.get2(info.user_id, "user_id");
		on_group_member_increase(info);
	}

	int32_t Cq::send_private_msg(int64_t user_id, int64_t group_id, message msg, bool auto_escape)
	{
		try
		{
			//msg = locale::conv::to_utf<char>(msg, "GBK");
			ptree pt;
			pt.put("action", "send_private_msg");
			ptree param;
			param.put("user_id", user_id);
			param.put("group_id", group_id);
			param.put("message", msg);
			param.put("auto_escape", auto_escape);
			pt.add_child("params", param);
			std::stringstream json;
			write_json(json, pt);
			std::string recv = api->PostMsg(json.str());
			if (recv == "")
			{
				std::cout << "你什么垃圾网络，这都能断线？？又或是你个大聪明把服务端关了？？人才啊你真是" << std::endl;
				//你断开链接了,好自为之。
				return -1;
			}
			pt.clear(); param.clear();
			std::stringstream ss(recv);
			read_json(ss, pt);
			param = pt.get_child("data");
			return param.get<int32_t>("message_id");
		}
		catch (ptree_error e)
		{
			std::cerr << e.what() << std::endl;
			return 0;
		}
	}

	int32_t Cq::send_group_msg(int64_t group_id, message msg, bool auto_escape)
	{
		try
		{
			//msg = to_utf<char>(msg, "GBK");
			ptree pt, param;
			pt.put("action", "send_group_msg");
			param.put("group_id", group_id);
			param.put("message", msg);
			param.put("auto_escape", auto_escape);
			pt.add_child("params", param);
			std::stringstream ssout, ssin;
			write_json(ssout, pt);
			ssin << api->PostMsg(ssout.str());
			read_json(ssin, pt);
			param = pt.get_child("data");
			return param.get<int32_t>("message_id");
		}
		catch (const std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			return 0;
		}
	}

	void Cq::delete_msg(int32_t msg_id)
	{
		try
		{
			ptree pt, param;
			pt.put("action", "delete_msg");
			param.put("message_id", msg_id);
			pt.put_child("params", param);
			std::stringstream ssout;
			write_json(ssout, pt);
			api->PostMsg(ssout.str());
		}
		catch (const std::exception& e)
		{
			std::cerr << e.what() << std::endl;
		}
	}

	void Cq::set_group_kick(int64_t group_id, int64_t user_id, bool reject_add_request)
	{
		try
		{
			ptree pt, param;
			pt.put("action", "set_group_kick");
			param.put("group_id", group_id);
			param.put("user_id", user_id);
			param.put("reject_add_request", reject_add_request);
			pt.put_child("params", param);
			std::stringstream ssout;
			write_json(ssout, pt);
			api->PostMsg(ssout.str());
		}
		catch (const std::exception& e)
		{
			std::cerr << e.what() << std::endl;
		}
	}

	void Cq::set_group_ban(int64_t group_id, int64_t user_id, int duration)
	{
		try
		{
			ptree pt, param;
			pt.put("action", "set_group_ban");
			param.put("group_id", group_id);
			param.put("user_id", user_id);
			param.put("duration", duration);
			pt.put_child("params", param);
			std::stringstream ssout;
			write_json(ssout, pt);
			api->PostMsg(ssout.str());
		}
		catch (const std::exception& e)
		{
			std::cerr << e.what() << std::endl;
		}
	}

	void Cq::set_group_whole_ban(int64_t group_id, bool enable)
	{
		try
		{
			ptree pt, param;
			pt.put("action", "set_group_whole_ban");
			param.put("group_id", group_id);
			param.put("enable", enable);
			pt.put_child("params", param);
			std::stringstream ssout;
			write_json(ssout, pt);
			api->PostMsg(ssout.str());
		}
		catch (const std::exception& e)
		{
			std::cerr << e.what() << std::endl;
		}
	}

	void Cq::set_group_card(int64_t group_id, int64_t user_id, std::string card)
	{
		try
		{
			ptree pt, param;
			pt.put("action", "set_group_card");
			param.put("group_id", group_id);
			param.put("user_id", user_id);
			param.put("card", /*to_utf<char>(*/card/*, "GBK")*/);
			pt.put_child("params", param);
			std::stringstream ssout;
			write_json(ssout, pt);
			api->PostMsg(ssout.str());
		}
		catch (const std::exception& e)
		{
			std::cerr << e.what() << std::endl;
		}
	}

	GROUP_MEMBER_INFO Cq::get_group_member_info(int64_t group_id, int64_t user_id, bool no_cache)
	{
		GROUP_MEMBER_INFO info;
		try
		{
			ptree pt, child;
			pt.put("action", "get_group_member_info");
			child.put("group_id", group_id);
			child.put("user_id", user_id);
			child.put("no_cache", no_cache);
			pt.put_child("params", child);
			std::stringstream ssout, ssin;
			write_json(ssout, pt);
			ssin << api->PostMsg(ssout.str());
			read_json(ssin, pt);
			ptree_helper h(pt.get_child("data"));
			info.group_id = group_id;
			info.user_id = user_id;
			info.nickname = h.get<std::string>("nickname");
			info.card = h.get<std::string>("card");
			info.sex = h.get<std::string>("sex");
			info.age = h.get<int32_t>("age");
			info.area = h.get<std::string>("area");
			info.join_time = h.get<int32_t>("join_time");
			info.last_sent_time = h.get<int32_t>("last_sent_time");
			info.level = h.get<std::string>("level");
			info.role = h.get<std::string>("role");
			info.unfriendly = h.get<bool>("unfriendly");
			info.title = h.get<std::string>("title");
			info.title_expire_time = h.get<int64_t>("title_expire_time");
			info.card_changeable = h.get<bool>("card_changeable");
			info.shut_up_timestamp = h.get<int64_t>("shut_up_timestamp");
		}
		catch (const ptree_error& e)
		{
			std::cerr << e.what() << std::endl;
		}
		return info;
	}

	void Cq::on_private_msg(const PRIVATE_MESSAGE_INFO& info) { }
	void Cq::on_group_msg(const GROUP_MESSAGE_INFO& info) { }
	void Cq::on_group_member_increase(const GROUP_MEMBER_INCREASE_INFO& info) { }

}