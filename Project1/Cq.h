/************************/
/*    Cq.h & Cq.cpp     */
/* public APIs & events */
/*  written by szx0427  */
/************************/

#pragma once

#include <cstdint>
#include <string>
#include <vector>

using namespace boost;
using namespace boost::property_tree;
using namespace boost::property_tree::json_parser;

// API document: https://docs.go-cqhttp.org/api
namespace GoCq
{
	template <class T> T* _zero_memory(T* _dest, size_t len = sizeof(T))
	{
		return (T*)memset(_dest, 0, len);
	}

	class ptree_helper
	{
	protected:
		ptree m_pt;
	public:
		ptree_helper(const ptree &p)
			: m_pt(p) { }
		~ptree_helper() { }
		template <class T> T get(const std::string& path, const T &def_value = T()) const
		{
			if (m_pt.find(path) == m_pt.not_found())
				return def_value;	
			return m_pt.get<T>(path);
		}
		template <class T> const T& get2(T& dest, const std::string& path, const T& def_value = T()) const
		{
			return dest = get<T>(path, def_value);
		}
		std::string get_conv(const std::string& path) const
		{
			return get<std::string>(path);
			//return locale::conv::from_utf(get<std::string>(path), "GBK");
		}
		ptree& pt()
		{
			return m_pt;
		}
		const ptree& pt() const
		{
			return m_pt;
		}
		const ptree_helper &operator=(const ptree &p)
		{
			m_pt = p;
			return *this;
		}
	};

	typedef std::string message;
	// typedef int object;
	typedef std::string forward_message;

	//struct MSG_INFO
	//{
	//	int32_t msg_id;
	//	int32_t real_id;
	//	object sender;
	//	int32_t time;
	//	message msg;
	//	message raw_msg;
	//};
	//struct IMAGE_INFO
	//{
	//	int32_t size;
	//	std::string filename;
	//	std::string url;
	//};
	//struct LOGIN_INFO
	//{
	//	int64_t user_id;
	//	std::string nickname;
	//};
	//struct QIDIAN_ACCOUNT_INFO
	//{
	//	int64_t master_id;
	//	std::string ext_name;
	//	int64_t create_time;
	//};
	struct STRANGER_INFO
	{
		int64_t user_id;
		std::string nickname;
		std::string sex;
		int32_t age;
		std::string qid;
		int32_t level;
		int32_t login_days;
	};
	//struct FRIEND_INFO
	//{
	//	int64_t user_id;
	//	std::string nickname;
	//	std::string remark;
	//};
	struct GROUP_INFO
	{
		int64_t group_id;
		std::string group_name;
		std::string group_memo;
		uint32_t group_creation_time;
		uint32_t group_level;
		int32_t member_count;
		int32_t max_member_count;
	};
	struct GROUP_MEMBER_INFO
	{
		int64_t group_id;
		int64_t user_id;
		std::string nickname;
		std::string card;
		std::string sex;
		int32_t age;
		std::string area;
		int32_t join_time;
		int32_t last_sent_time;
		std::string level;
		std::string role;
		bool unfriendly;
		std::string title;
		int64_t title_expire_time;
		bool card_changeable;
		int64_t shut_up_timestamp;
	};

	struct PRIVATE_MESSAGE_INFO
	{
		int64_t time;
		int64_t self_id;
		std::string post_type;
		std::string message_type;
		std::string sub_type;
		int temp_source;
		int32_t message_id;
		int64_t user_id;
		message msg;
		std::string raw_message;
		int32_t font;
		struct
		{
			int64_t user_id;
			std::string nickname;
			std::string sex;
			int32_t age;
		} sender;
	};
	struct GROUP_MESSAGE_INFO
	{
		int64_t time;
		int64_t self_id;
		std::string post_type;
		std::string message_type;
		std::string sub_type;
		int32_t message_id;
		int64_t group_id;
		int64_t user_id;
		struct
		{
			int64_t id;
			std::string name;
			std::string flag;
		} anonymous;
		message msg;
		std::string raw_message;
		int32_t font;
		struct
		{
			int64_t user_id;
			std::string nickname;
			std::string card;
			std::string sex;
			int32_t age;
			std::string area;
			std::string level;
			std::string role;
			std::string title;
		} sender;
	};
	struct GROUP_MEMBER_INCREASE_INFO
	{
		int64_t time;
		int64_t self_id;
		std::string post_type;
		std::string notice_type;
		std::string sub_type;
		int64_t group_id;
		int64_t operator_id;
		int64_t user_id;
	};

	class Cq
	{
	public:
		Cq(std::string server, std::string port);
		~Cq();

	private:
		Api_For_GoCQ *api;
		std::thread *_th_get_event;
		void _get_event();
		void _process_private_msg(ptree_helper h);
		void _process_group_msg(ptree_helper h);
		void _process_group_increase(ptree_helper h);

	public:
		int32_t send_private_msg(int64_t user_id, int64_t group_id, message msg, bool auto_escape = false);
		int32_t send_group_msg(int64_t group_id, message msg, bool auto_escape = false);
		// void send_group_forward_msg(int64_t group_id,)
		//int32_t send_msg(std::string msg_type, int64_t user_id, int64_t group_id, message msg, bool auto_escape);
		void delete_msg(int32_t msg_id);
		//MSG_INFO get_msg(int32_t msg_id);
		//forward_message get_forward_msg(std::string msg_id);
		//IMAGE_INFO get_image(std::string file);
		void set_group_kick(int64_t group_id, int64_t user_id, bool reject_add_request = false);
		void set_group_ban(int64_t group_id, int64_t user_id, int duration = 30 * 60);
		//void set_group_anonymous_ban(int64_t group_id, object anonymous, std::string anonymous_flag, int duration = 30 * 60);
		void set_group_whole_ban(int64_t group_id, bool enable = true);
		//void set_group_admin(int64_t group_id, int64_t user_id, bool enable = true);
		//void set_group_anonymous(int64_t group_id, bool enable = true);
		void set_group_card(int64_t group_id, int64_t user_id, std::string card = "");
		void set_group_name(int64_t group_id, std::string group_name);
		void set_group_leave(int64_t group_id, bool is_dismiss = false);
		//void set_group_special_title(int64_t group_id, int64_t user_id, std::string special_title, int duration = -1);
		//void set_friend_add_request(std::string flag, bool approve = true, std::string remark = "");
		//void set_group_add_request(std::string flag, std::string sub_type, bool approve = true, std::string reason = "");
		//LOGIN_INFO get_login_info();
		//QIDIAN_ACCOUNT_INFO qidian_get_account_info();
		STRANGER_INFO get_stranger_info(int64_t user_id, bool no_cache = false);
		//std::vector<FRIEND_INFO> get_friend_list();
		//void delete_friend(int64_t friend_id);
		GROUP_INFO get_group_info(int64_t group_id, bool no_cache = false);
		//std::vector<GROUP_INFO> get_group_list();
		GROUP_MEMBER_INFO get_group_member_info(int64_t group_id, int64_t user_id, bool no_cache = false);
		//inline std::string get_group_logo_url(int64_t group_id)
		//{
		//	char buf[50];
		//	sprintf(buf, "https://p.qlogo.cn/gh/%I64d/%I64d/100", group_id, group_id);
		//	return buf;
		//}

	protected:
		virtual void on_private_msg(const PRIVATE_MESSAGE_INFO &info);
		virtual void on_group_msg(const GROUP_MESSAGE_INFO &info);
		virtual void on_group_member_increase(const GROUP_MEMBER_INCREASE_INFO& info);
	};

}