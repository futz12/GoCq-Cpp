#include "pch.h"
#include "szx_string.h"
#include "TestCqClass.h"

#define s2s(_str)		szx_string(_str)
#define tos(t)			std::to_string(t)
#define ats				id2at(info.user_id)
#define gid				info.group_id
using ph = GoCq::ptree_helper;

int64_t TestCqClass::at2id(szx_string at, bool re)
{
	if (re)
		at.replace(" ", "");
	auto ret = _atoi64(at.c_str());
	if (ret)
		return ret;
	at.replace("[CQ:at,qq=", "");
	at.replace("]", "");
	return _atoi64(at.c_str());
}

szx_string TestCqClass::id2at(int64_t id)
{
	std::stringstream ss;
	ss << "[CQ:at,qq=" << id << "]";
	return ss.str();
}

void TestCqClass::update_lists_info()
{
	admin_list.clear();
	black_list.clear();

	std::ifstream fs(config_file);
	if (!fs.is_open())
	{
		std::cerr << "未能打开config.json文件。管理员名单与黑名单将不可用。" << std::endl;
		return;
	}
	std::stringstream ss;
	ss << fs.rdbuf();

	try
	{
		ptree pt, child;
		read_json(ss, pt);
		child = pt.get_child("admin");
		for (auto it : child)
		{
			admin_list.push_back(it.second.get_value<int64_t>());
		}
		child = pt.get_child("black");
		for (auto it : child)
		{
			black_list.push_back(it.second.get_value<int64_t>());
		}
	}
	catch (const ptree_error &e)
	{
		std::cerr << e.what() << std::endl;
	}
}

void TestCqClass::on_private_msg(const GoCq::PRIVATE_MESSAGE_INFO& info)
{
	std::cout << "收到好友 " << info.sender.nickname << "(" << info.sender.user_id << ") 的消息："
		<< info.msg << std::endl;
	std::cout << "temp_source = " << info.temp_source << std::endl;
}

void TestCqClass::on_group_msg(const GoCq::GROUP_MESSAGE_INFO& info)
{
	// TODO: 在此处添加实现代码.
	update_lists_info();
	if (is_found(black_list, info.user_id))
		return;
	szx_string msg = info.msg;
	if (msg.begin_with("禁言")) { ban(info); }
	else if (msg.begin_with("取码")) { to_code(info); }
	else if (msg.begin_with("复读")) { say_again(info); }
	else if (msg.begin_with("改名")) { change_name(info); }
	else if (msg.begin_with("反取码") || msg.begin_with("反码")) { to_message(info); }
	else if (msg == "菜单") { menu(info); }
	else if (msg.begin_with("[CQ:reply,id=") && msg.find("机器人撤回") != -1) { del_msg(info); }
	else if (msg == "查询") { query(info); }
	else if (msg == "签到") { signin(info); }
	else if (msg == "开始猜数") { begin_guessnum(info); }
	else if (msg.begin_with("猜数") && !msg.begin_with("猜数字")) { guessnum(info); }
	else if (msg.begin_with("金币排行榜")) { coinrank(info); }
	else if (msg.begin_with("碰运气")) { gamble(info); } // 赌博冻结账号！！！！！！！！！！！！！！
	else if (msg.begin_with("充值")) { recharge(info); }
	else if (msg == "抽奖") { lottery(info); }
	else if (msg.begin_with("销户")) { del_user(info); }
	//else if (msg.begin_with("发红包")) { _send_red_envelope(info); }
	//else if (msg.begin_with("Q") || msg.begin_with("q")) { _catch_red_envelope(info); }
	else if (msg.begin_with("转账")) { transfer(info); }
	else if (msg == "抽涩图" || msg == "抽色图") { getSexyPicture(info); }
}

void TestCqClass::on_group_member_increase(const GoCq::GROUP_MEMBER_INCREASE_INFO& info)
{
	auto msg =
		id2at(info.user_id) +
		"\n[CQ:face,id=144]欢迎您加入本群！若有任何问题或建议请联系裙主或管理员哦！";
	send_group_msg(info.group_id, msg);
}

void TestCqClass::ban(const GoCq::GROUP_MESSAGE_INFO& info)
{
	int len = strlen("禁言");
	auto msg = s2s(info.msg).replace(" ", "");
	if (!is_found(admin_list, info.user_id))
	{
		send_group_msg(info.group_id, "很抱歉，您不是机器人管理员，不能使用该功能！");
		return;
	}
	int pos = msg.rfind('*');
	if (pos != -1)
	{
		int time = atoi(msg.substr(pos + 1).c_str());
		if (time <= 60 * 24 * 30 - 1)
		{
			auto qid = at2id(msg.substr(len, pos - len));
			set_group_ban(info.group_id, qid, 60 * time);
			if (time > 0)
				send_group_msg(info.group_id, id2at(qid) + "\n您被管理员禁言" + tos(time) + "分钟！");
			else
				send_group_msg(info.group_id, id2at(qid) + "\n恭喜，您被管理员解除禁言！");
			return;
		}
	}
	send_group_msg(info.group_id, "此命令为机器人管理员专用\n格式：禁言 [qq/at] * [时间(分钟)]\n时间不得大于 (60*24*30-1) 分钟");
}

void TestCqClass::change_name(const GoCq::GROUP_MESSAGE_INFO& info)
{
	int len = strlen("改名");
	auto msg = s2s(info.msg);
	if (is_found(admin_list, info.user_id))
	{
		int pos = msg.rfind('*');
		if (pos != -1)
		{
			set_group_card(info.group_id, at2id(msg.substr(len, pos - len)), msg.substr(pos + 1));
			send_group_msg(info.group_id, "[CQ:at,qq=" + tos(at2id(msg.substr(len, pos - len))) + "]\n您的新名片为：" + msg.substr(pos + 1));
			return;
		}
	}
	send_group_msg(info.group_id, "更改某人的群名片\n此命令为机器人管理员专用\n格式：改名[qq/at]*[名片]");
}

void TestCqClass::to_code(const GoCq::GROUP_MESSAGE_INFO& info)
{
	auto msg = s2s(info.msg).replace(" ", "");
	std::string s = msg.substr(strlen("取码"));
	if (s.length() == 0)
		send_group_msg(info.group_id, "格式：取码[内容]");
	else
		send_group_msg(info.group_id, s, true);

}

void TestCqClass::to_message(const GoCq::GROUP_MESSAGE_INFO& info)
{
	auto msg = s2s(info.msg)/*.replace(" ", "")*/;
	int len = msg.begin_with("反取码") ? strlen("反取码") : strlen("反码");
	std::string s = msg.substr(len);
	if (s.length() == 0)
		send_group_msg(info.group_id, "格式：反取码[内容]");
	else
	{
		msg.replace("&#91;", "[");
		msg.replace("&#93;", "]");
		send_group_msg(info.group_id, msg.substr(len));
	}
}

void TestCqClass::say_again(const GoCq::GROUP_MESSAGE_INFO& info)
{
	send_group_msg(info.group_id, info.msg.substr(strlen("复读")));
}

void TestCqClass::menu(const GoCq::GROUP_MESSAGE_INFO& info)
{
	auto str =
		"=========菜单=========\n"
		"1️⃣普通功能：\n"
		"取码[内容] 复读[内容]\n"
		"反取码[内容]   菜单\n"
		"2️⃣管理员专用：\n"
		"禁言[at/qq]*[时间/分钟]\n"
		"改名[at/qq]*[新名片]\n"
		"[消息回复] 机器人撤回\n"
		"充值[qq/at]*[钱数]\n"
		"销户[qq/at]（慎用）\n"
		"转账[qq/at]*[钱数]\n"
		"3️⃣娱乐功能（建设中...）\n"
		"查询 签到 金币排行榜[人数=5]\n"
		"开始猜数   猜数[数字]\n"
		"抽奖  碰运气(懂得都懂)\n"
		"抽色图/抽涩图\n"
		"ver 1.0.1=============\n"
		"通信开发---futz（裙主）\n"
		"交互开发---szx0427（rd）";
	send_group_msg(info.group_id, str);
}

void TestCqClass::del_msg(const GoCq::GROUP_MESSAGE_INFO& info)
{
	if (!is_found(admin_list, info.user_id))
	{
		send_group_msg(info.group_id, "抱歉，该功能为机器人管理员专用！");
		return;
	}
	auto str = info.msg.substr(strlen("[CQ:reply,id="));
	std::string num;
	for (auto c : str)
	{
		if (c == ']') break;
		num += c;
	}
	delete_msg(atoi(num.c_str()));
	send_group_msg(info.group_id, "已尝试撤回id为 " + num + " 的消息。");
}

void TestCqClass::query(const GoCq::GROUP_MESSAGE_INFO& info)
{
	try
	{
		ptree pt, child;
		read_json(config_file, pt);
		child = pt.get_child("entertainment");
		for (auto user : child)
		{
			if (user.second.get<int64_t>("id") == info.user_id)
			{
				send_group_msg(info.group_id,
					id2at(info.user_id) + "\n💰金币：" + user.second.get<std::string>("coins", "0") +
					"\n[CQ:face,id=69]奖券：" + tos(GoCq::ptree_helper(user.second).get<int>("tickets")) +
					"\n[CQ:face,id=168]体力值：" + ph(user.second).get<std::string>("spirit", "0"));
				return;
			}
		}
		send_group_msg(info.group_id, id2at(info.user_id) + "\n您还没有签到开户，无法使用此功能！");
	}
	catch (const ptree_error& e)
	{
		std::cerr << e.what() << std::endl;
	}
}

void TestCqClass::signin(const GoCq::GROUP_MESSAGE_INFO& info)
{
	time_t now = time(NULL);
	tm tmnow; 
	memcpy(&tmnow, localtime(&now), sizeof(tm)); // 必须这么写，经验之谈。
	srand(now);
	try
	{
		ptree pt;
		read_json(config_file, pt);
		auto &child = pt.get_child("entertainment");
		auto addcoin = rand() % (800 - 100) + 100;
		auto addticket = rand() & 1;
		auto addspirit = rand() % 100;
		ptree newu;
		ptree* puser = &newu;
		for (auto &user : child)
		{
			if (user.second.get<int64_t>("id") == info.user_id)
			{
				time_t last = GoCq::ptree_helper(user.second).get<time_t>("last_signin", -1);
				if (last != -1)
				{
					tm* tmlast = localtime(&last);
					if (
						tmlast->tm_year == tmnow.tm_year &&
						tmlast->tm_mon == tmnow.tm_mon &&
						tmlast->tm_mday == tmnow.tm_mday
						)
					{
						send_group_msg(info.group_id, id2at(info.user_id) + "\n您今天已经签到过了，请勿重复签到！");
						return;
					}
				}
				puser = &user.second;
				break;
			}
		}
		ph h(*puser);
		puser->put("id", info.user_id);
		puser->put("coins", h.get<int>("coins") + addcoin);
		puser->put("tickets", h.get<int>("addticket") + addticket);
		puser->put("spirit", h.get<int>("spirit") + addspirit);
		puser->put("last_signin", now);
		if (puser == &newu)
		{
			child.push_back(std::make_pair("", newu));
		}
		write_json(config_file, pt);
		h = ph(*puser); // refresh value
		send_group_msg(info.group_id, id2at(info.user_id) + "\n签到成功，获得:\n💰 "
			+ tos(addcoin) + "金币\n[CQ:face,id=69] "
			+ tos(addticket) + "张奖券\n[CQ:face,id=168]"
			+ tos(addspirit) + "点体力"
			"\n目前拥有金币：" + h.get<std::string>("coins", "0") +
			"\n目前拥有奖券：" + h.get<std::string>("tickets", "0") +
			"\n目前体力值：" + h.get<std::string>("spirit", "0"));
	}
	catch (const ptree_error& e)
	{
		std::cerr << e.what() << std::endl;
	}
}

void TestCqClass::begin_guessnum(const GoCq::GROUP_MESSAGE_INFO& info)
{
	if (guessing_num)
	{
		send_group_msg(info.group_id, "目前正在猜数，无需开始！");
		return;
	}
	guessing_num = true;
	srand(time(NULL));
	auto my_range = rand() % (70 - 20) + 20;
	the_num = rand() % my_range;
	send_group_msg(info.group_id, id2at(info.user_id) + "\n猜数已开始！\n范围：0~" + tos(my_range));
}

void TestCqClass::guessnum(const GoCq::GROUP_MESSAGE_INFO& info)
{
	if (!guessing_num)
	{
		send_group_msg(info.group_id, "目前还没有开始猜数，请发送“开始猜数”！");
		return;
	}
	auto msg = s2s(info.msg).replace(" ", "");
	auto len = strlen("猜数");
	if (msg.length() == len)
	{
		send_group_msg(info.group_id, "格式：猜数[数字]");
		return;
	}

	ptree pt;
	read_json(config_file, pt);
	auto& child = pt.get_child("entertainment");
	ptree *user = nullptr;
	bool not_found = true;
	int coins = 0;
	for (auto& u : child)
	{
		if (u.second.get<int64_t>("id") == info.user_id)
		{
			not_found = false;
			user = &u.second;
			coins = GoCq::ptree_helper(*user).get<int>("coins");
			break;
		}
	}
	if (not_found || coins < 30)
	{
		send_group_msg(info.group_id, id2at(info.user_id) + "\n抱歉，您的金币少于30个，无法使用猜数功能！");
		return;
	}

	prize_map[info.user_id]++;
	total_count++;

	auto num = atoi(msg.substr(len).c_str());
	int coin_change;
	srand(time(NULL));
	if (num == the_num)
	{
		coin_change = rand() % (300 - 100) + 100 + prize_pool * 0.8;
		auto smsg = id2at(info.user_id) + "\n恭喜猜中！\n本轮猜数奖励：\n";
		int64_t id;
		int coin_change_for_user;
		std::vector<std::pair<int64_t, int>> veccoin; // for sorting by coin count

		for (auto& u : child)
		{
			if (prize_map.find((id = u.second.get<int64_t>("id"))) != prize_map.end())
			{
				coin_change_for_user = coin_change * (prize_map[id] / (float)total_count);
				if (id == info.user_id)
					coin_change_for_user += rand() % (150 - 50) + 50;
				veccoin.push_back(std::make_pair(id, coin_change_for_user));
				u.second.put("coins", u.second.get<int>("coins") + coin_change_for_user);
			}
		}

		std::pair<int64_t, int> t;
		for (int i = 0; i < veccoin.size(); i++)
		{
			for (int j = i + 1; j < veccoin.size(); j++)
			{
				if (veccoin[i].second < veccoin[j].second)
				{
					t = veccoin[i];
					veccoin[i] = veccoin[j];
					veccoin[j] = t;
				}
			}
			smsg += tos(i + 1) + ". " + id2at(veccoin[i].first) + "---" + tos(veccoin[i].second) + "金币\n";
		}
		
		smsg += "本轮猜数结束。";
		guessing_num = false;
		prize_pool = 0;
		the_num = -1;
		total_count = 0;
		prize_map.clear();
		send_group_msg(info.group_id, smsg);
	}
	else
	{
		coin_change = -(rand() % (70 - 30)) - 30;
		prize_pool += abs(coin_change);
		user->put("coins", coins + coin_change);
		send_group_msg(
			info.group_id, id2at(info.user_id) + "\n抱歉，您猜" +
			(num > the_num ? "大" : "小") +
			"了！扣除" + tos(abs(coin_change)) + "金币！\n"
			"目前奖池共有：" + tos(prize_pool) + "金币。"
		);
	}

	write_json(config_file, pt);
}

void TestCqClass::coinrank(const GoCq::GROUP_MESSAGE_INFO& info)
{
	int count = 5;
	auto msg = s2s(info.msg).replace(" ", "");
	if (msg.length() != strlen("金币排行榜"))
	{
		count = atoi(msg.substr(strlen("金币排行榜")).c_str());
		if (count <= 0 || (!is_admin(info.user_id) && count > 10))
		{
			send_group_msg(info.group_id, id2at(info.user_id) + "\n您输入的人数不正确。普通成员只能获取前1~10位，机器人管理员可获取全部。");
			return;
		}
	}

	ptree pt;
	read_json(config_file, pt);
	auto& child = pt.get_child("entertainment");
	std::vector<std::pair<int64_t, int>> veccoin;
	for (auto& u : child)
	{
		veccoin.push_back(std::make_pair(u.second.get<int64_t>("id"), u.second.get<int>("coins")));
	}
	std::sort(veccoin.begin(), veccoin.end(), 
		[](const std::pair<int64_t, int>& p1, const std::pair<int64_t, int>& p2)
		{
			return p1.second > p2.second;
		});
	veccoin.resize(__min(veccoin.size(), count));
	std::string smsg = "金币排行榜：\n";
	std::string card;
	for (int i = 0; i < veccoin.size(); i++)
	{
		auto member_info = get_group_member_info(info.group_id, veccoin[i].first);
		card = member_info.card;
		smsg +=
			tos(i + 1) + ". " + tos(veccoin[i].first) +
			"(" + (card.length() == 0 ? member_info.nickname : card) + ")"
			+ "，" + tos(veccoin[i].second) + "金币\n";
	}
	send_group_msg(info.group_id, smsg);
}

void TestCqClass::gamble(const GoCq::GROUP_MESSAGE_INFO& info)
{
	auto msg = s2s(info.msg).replace(" ", "");
	int len = strlen("碰运气");
	int money = atoi(msg.substr(len).c_str());
	if (money > 0)
	{
		ptree pt;
		read_json(config_file, pt);
		auto& child = pt.get_child("entertainment");
		ptree* user = nullptr;
		int coins;
		int spirit;
		for (auto& u : child)
		{
			if (u.second.get<int64_t>("id") == info.user_id)
			{
				if (
					(coins = GoCq::ptree_helper(u.second).get<int>("coins")) >= money &&
					(spirit = ph(u.second).get<int>("spirit")) >= 10
					) { user = &u.second; }
				break;
			}
		}

		if (user)
		{
			//srand(time(NULL));
			std::mt19937 rnd(time(NULL));
			if (/*rand() & 1*/ rnd() & 1)
			{
				coins += money;
				send_group_msg(info.group_id, id2at(info.user_id) + "\n[CQ:face,id=4]碰运气成功！\n"
					"消耗10点体力！\n目前剩余金币：" + tos(coins) + "\n目前剩余体力：" + tos(spirit -= 10));
			}
			else
			{
				coins -= money;
				send_group_msg(info.group_id, id2at(info.user_id) + "\n[CQ:face,id=20]碰运气失败！\n"
					"消耗10点体力！\n目前剩余金币：" + tos(coins) + "\n目前剩余体力：" + tos(spirit -= 10));
			}
			user->put("coins", coins);
			user->put("spirit", spirit);
			write_json(config_file, pt);
			return;
		}
	}

	send_group_msg(info.group_id, "格式：碰运气[钱数(>0)]\n碰运气1次消耗10点体力\n提示：您目前的钱数必须大于或等于碰运气的钱数；您的体力值必须>=10；成功率理论上为50%。");
}

void TestCqClass::recharge(const GoCq::GROUP_MESSAGE_INFO& info)
{
	auto msg = s2s(info.msg).replace(" ", "");
	if (is_admin(info.user_id))
	{
		auto len = strlen("充值");
		int pos = msg.rfind('*');
		if (pos != -1)
		{
			auto num = atoi(msg.substr(pos + 1).c_str());
			auto id = at2id(msg.substr(len, pos - len));
			if (id != 0 && num != 0)
			{
				ptree pt;
				read_json(config_file, pt);
				auto& child = pt.get_child("entertainment");
				bool found = false;
				for (auto& u : child)
				{
					if (u.second.get<int64_t>("id") == id)
					{
						found = true;
						u.second.put("coins", GoCq::ptree_helper(u.second).get<int>("coins") + num);
						break;
					}
				}
				if (!found)
				{
					ptree user;
					user.put("id", id);
					user.put("coins", num);
					child.push_back(std::make_pair("", user));
				}
				write_json(config_file, pt);
				send_group_msg(info.group_id, id2at(id) + "\n恭喜，管理员为您充值" + tos(num) + "金币！");
				return;
			}
		}
	}
	send_group_msg(info.group_id, id2at(info.user_id) + "\n格式：充值[qq/at]*[钱数]\n该功能为机器人管理员专用。");
}

void TestCqClass::lottery(const GoCq::GROUP_MESSAGE_INFO& info)
{
	ptree pt;
	open_config(pt);
	auto& child = pt.get_child("entertainment");
	ptree* user = nullptr;
	int tickets = 0;
	int coins = 0;
	for (auto& u : child)
	{
		if (u.second.get<int64_t>("id") == info.user_id)
		{
			user = &u.second;
			tickets = GoCq::ptree_helper(u.second).get<int>("tickets");
			coins = GoCq::ptree_helper(u.second).get<int>("coins");
			break;
		}
	}
	if (tickets >= 1)
	{
		srand(time(NULL));
		int num = rand() % (500 - 50) + 50;
		user->put("coins", coins += num);
		user->put("tickets", --tickets);
		save_config(pt);
		send_group_msg(info.group_id,
			id2at(info.user_id) + "\n恭喜，您抽奖获得" + tos(num) + "金币！\n"
			"目前您的剩余金币数量：" + tos(coins) + "\n您目前剩余奖券数量：" + tos(tickets));
		return;
	}

	send_group_msg(info.group_id, id2at(info.user_id) + "\n每次抽奖需要消耗1张奖券，您的奖券不足，奖券可通过每日签到获取。");
}

void TestCqClass::del_user(const GoCq::GROUP_MESSAGE_INFO& info)
{
	if (is_admin(info.user_id))
	{
		int len = strlen("销户");
		auto msg = s2s(info.msg).replace(" ", "");
		if (msg.length() > len)
		{
			auto id = at2id(msg.substr(len));
			ptree pt;
			open_config(pt);
			auto& child = pt.get_child("entertainment");
			for (auto it = child.begin(); it != child.end(); it++)
			{
				if (it->second.get<int64_t>("id") == id)
				{
					child.erase(it);
					save_config(pt);
					send_group_msg(info.group_id, id2at(info.user_id) + "\n已成功清除用户" + tos(id) + "的所有信息。");
					return;
				}
			}
			send_group_msg(info.group_id, id2at(info.user_id) + "\n抱歉，当前配置文件中未找到用户" + tos(id) + "的信息。");
			return;
		}
	}
	send_group_msg(info.group_id, id2at(info.user_id) + "\n格式：销户[qq/at]（慎用）\n该命令为机器人管理员专用。");
}

//void TestCqClass::_catch_red_envelope(const GoCq::GROUP_MESSAGE_INFO& info)
//{
//	int n = atoi(info.msg.substr(1).c_str());
//	if (n <= 0 || red_envelopes[n - 1] == 0)
//		return;
//	srand(time(NULL));
//	int coin_change = rand() % red_envelopes[n - 1] + 1; // 范围[1,最大]
//	red_envelopes[n - 1] -= coin_change;
//
//	ptree pt;
//	open_config(pt);
//	auto& child = pt.get_child("entertainment");
//	bool found = false;
//	for (auto& u : child)
//	{
//		if (u.second.get<int64_t>("id") == info.user_id)
//		{
//			found = true;
//			u.second.put("coins", u.second.get<int>("coins") + coin_change);
//			break;
//		}
//	}
//	if (!found)
//	{
//		ptree user;
//		user.put("id", info.user_id);
//		user.put("coins", coin_change);
//		child.push_back(std::make_pair("", user));
//	}
//	save_config(pt);
//
//	send_group_msg(info.group_id,
//		"恭喜" + id2at(info.user_id) + "抢到" + tos(coin_change) + "个金币！\n"
//		"序号为" + tos(n) + "的红包还剩" + tos(red_envelopes[n - 1]) + "个金币。");
//}
//
//void TestCqClass::_send_red_envelope(const GoCq::GROUP_MESSAGE_INFO& info)
//{
//	auto msg = s2s(info.msg).replace(" ", "");
//	auto len = strlen("发红包");
//	int coins = atoi(info.msg.substr(len).c_str());
//	if (coins > 0)
//	{
//		ptree pt;
//		open_config(pt);
//		auto& child = pt.get_child("entertainment");
//		ptree* user = nullptr;
//		for (auto& u : child)
//		{
//			if (u.second.get<int64_t>("id") == info.user_id)
//			{
//				user = &u.second;
//				break;
//			}
//		}
//		if (!user)
//		{
//			send_group_msg(info.group_id, id2at(info.user_id) + "\n您还没有签到开户，无法使用此功能！");
//			return;
//		}
//		auto has_coins = user->get<int>("coins");
//		if (has_coins >= coins)
//		{
//			user->put("coins", has_coins - coins);
//			srand(time(NULL));
//			int addspirit = 0.5 * coins + rand() % (100 - 50) + 50;
//			user->put("spirit", ph(*user).get<int>("spirit") + addspirit);
//			save_config(pt);
//
//			int n;
//			for (int i = 0; i < 100; i++)
//			{
//				if (red_envelopes[i] == 0)
//				{
//					n = i + 1;
//					red_envelopes[i] = coins;
//					break;
//				}
//			}
//
//			send_group_msg(info.group_id,id2at(info.user_id)+"\n感谢发红包，您获得"+tos(addspirit)+"点体力！\n大家可以发送")
//		}
//	}
//	send_group_msg(info.group_id, "格式：发红包[钱数]\n注：每次发红包您会获得一定的体力；红包钱数必须小于或等于您所拥有的金币数。");
//}

void TestCqClass::transfer(const GoCq::GROUP_MESSAGE_INFO& info)
{
	auto msg = s2s(info.msg).replace(" ", "");
	auto len = strlen("转账");
	auto pos = msg.rfind('*');
	if (pos != -1)
	{
		auto num = atoi(msg.substr(pos + 1).c_str());
		auto id = at2id(msg.substr(len, pos - len));
		if (id != 0 && num > 0)
		{
			ptree pt;
			read_json(config_file, pt);
			auto& child = pt.get_child("entertainment");

			bool found = false;
			int after;
			// 1st. look for source
			for (auto& u : child)
			{
				if (u.second.get<int64_t>("id") == info.user_id)
				{
					found = true;
					int current = GoCq::ptree_helper(u.second).get<int>("coins");
					if (current < num)
					{
						send_group_msg(info.group_id, id2at(info.user_id) + "\n抱歉，您的余额不足，无法转账！\n您目前的余额为：" + tos(current));
						return;
					}
					u.second.put("coins", after = current - num);
					break;
				}
			}
			if (!found)
			{
				send_group_msg(info.group_id, ats + "\n抱歉，您还没有签到开户，无法给他人转账！");
				return;
			}

			// 2nd. look for target
			found = false;
			int after2 = num;
			for (auto& u : child)
			{
				if (u.second.get<int64_t>("id") == id)
				{
					found = true;
					u.second.put("coins", after2 = GoCq::ptree_helper(u.second).get<int>("coins") + num);
					break;
				}
			}
			if (!found)
			{
				ptree user;
				user.put("id", id);
				user.put("coins", num);
				child.push_back(std::make_pair("", user));
			}

			write_json(config_file, pt);

			send_group_msg(gid, ats + "\n转账成功！您目前余额：" + tos(after) + "\n" + id2at(id) + "\n您入账了" + tos(num) + "个金币，目前余额：" + tos(after2));
			return;
		}
	}

	send_group_msg(gid, ats + "\n格式为：转账[qq/at]*[钱数]\n您目前余额必须大于或等于转账的钱数。");
}

void TestCqClass::getSexyPicture(const GoCq::GROUP_MESSAGE_INFO& info)
{
	ptree pt;
	open_config(pt);
	auto& child = pt.get_child("entertainment");
	for (auto& u : child)
	{
		int coins;
		if (u.second.get<int64_t>("id") == info.user_id)
		{
			if ((coins = ph(u.second).get<int>("coins")) < 50)
			{
				break;
			}
			u.second.put("coins", coins -= 50);
			save_config(pt);
			send_group_msg(gid, ats + "\n获取成功，消耗50金币，目前余额：" + tos(coins) + "\n[CQ:image,cache=0,file=https://iw233.cn/API/Random.php]");
			return;
		}
	}
	send_group_msg(gid, ats + "\n抱歉，使用此功能需消耗50个金币，您的金币不足！");
}
