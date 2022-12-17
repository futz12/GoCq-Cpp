#pragma once
#include "Cq.h"
#include "szx_string.h"

class TestCqClass :
    public GoCq::Cq
{
public:
    TestCqClass(std::string h, std::string p)
        : Cq(h, p), guessing_num(false), the_num(-1), prize_pool(0), total_count(0)
        , config_file("E:\\2017projects\\Project1\\config.json")
    { 
        //memset(red_envelopes, 0, sizeof(red_envelopes));
    }

    int64_t at2id(szx_string at, bool re = false);
    szx_string id2at(int64_t id);

    inline void open_config(ptree& pt)
    {
        return read_json(config_file, pt);
    }

    inline void save_config(const ptree& pt)
    {
        return write_json(config_file, pt);
    }

    template <class _Elem>
    auto vecfind(const std::vector<_Elem> &vec, const _Elem& e)
    {
        return std::find(vec.begin(), vec.end(), e);
    }

    template <class _Elem>
    bool is_found(const std::vector<_Elem>& vec, const _Elem& e)
    {
        return vecfind(vec, e) != vec.end();
    }

    std::vector<int64_t> admin_list;
    std::vector<int64_t> black_list;
    const std::string config_file;

    inline bool is_admin(int64_t id, bool update = true)
    {
        if (update)
            update_lists_info();
        return is_found(admin_list, id);
    }
    inline bool is_black(int64_t id, bool update = true)
    {
        if (update)
            update_lists_info();
        return is_found(black_list, id);
    }

    void update_lists_info();

protected:
    virtual void on_private_msg(const GoCq::PRIVATE_MESSAGE_INFO& info);
    virtual void on_group_msg(const GoCq::GROUP_MESSAGE_INFO& info);
    virtual void on_group_member_increase(const GoCq::GROUP_MEMBER_INCREASE_INFO& info);
private:
    bool guessing_num;
    int the_num;
    int prize_pool;
    std::map<int64_t, int> prize_map;
    int total_count;
    //std::pair<int64_t, int> red_envelopes[100];
    //int red_envelopes[100];
public: // robot features
    void ban(const GoCq::GROUP_MESSAGE_INFO& info);
    void change_name(const GoCq::GROUP_MESSAGE_INFO& info);
    void to_code(const GoCq::GROUP_MESSAGE_INFO& info);
    void to_message(const GoCq::GROUP_MESSAGE_INFO& info);
    void say_again(const GoCq::GROUP_MESSAGE_INFO& info);
    void menu(const GoCq::GROUP_MESSAGE_INFO& info);
    void del_msg(const GoCq::GROUP_MESSAGE_INFO& info);
    void query(const GoCq::GROUP_MESSAGE_INFO& info);
    void signin(const GoCq::GROUP_MESSAGE_INFO& info);
    void begin_guessnum(const GoCq::GROUP_MESSAGE_INFO& info);
    void guessnum(const GoCq::GROUP_MESSAGE_INFO& info);
    void coinrank(const GoCq::GROUP_MESSAGE_INFO& info);
    void gamble(const GoCq::GROUP_MESSAGE_INFO& info);
    void recharge(const GoCq::GROUP_MESSAGE_INFO& info);
    void lottery(const GoCq::GROUP_MESSAGE_INFO& info);
    void del_user(const GoCq::GROUP_MESSAGE_INFO& info);
    //void _send_red_envelope(const GoCq::GROUP_MESSAGE_INFO& info);
    //void _catch_red_envelope(const GoCq::GROUP_MESSAGE_INFO& info);
    void transfer(const GoCq::GROUP_MESSAGE_INFO& info);
    void getSexyPicture(const GoCq::GROUP_MESSAGE_INFO& info);
};
