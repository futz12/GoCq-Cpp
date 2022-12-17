#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/locale/encoding.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>
#include <queue>
#include <ctime>
#include <map>
#include <mutex>

using tcp = boost::asio::ip::tcp;              // from <boost/asio/ip.tcp.hpp>
namespace websocket = boost::beast::websocket; // from <boost/beast/websocket.hpp>

//#define DEBUG 1

class ws_client
{
private:
	bool connected = false;

	boost::asio::io_context ioc;
	tcp::resolver resolver{ ioc };
	websocket::stream<tcp::socket> ws{ ioc };

public:
	ws_client(std::string const &host, std::string const &port)
	{
		try
		{
			auto const results = resolver.resolve(host, port);
			boost::asio::connect(ws.next_layer(), results.begin(), results.end());
			ws.handshake(host, "/");
			connected = true;
		}
		catch (std::exception const &e)
		{
			std::cerr << "\033[0;31m Error: \033[0m" << e.what() << "\n";
#ifdef DEBUG
			abort();
#endif
		}
	}
	~ws_client()
	{
		if (!connected)
			return;
		try
		{
			ws.close(websocket::close_code::normal);
		}
		catch (std::exception const &e)
		{
			std::cerr << "\033[0;31m Error: \033[0m" << e.what() << "\n";
#ifdef DEBUG
			abort();
#endif
			return;
		}
	}

	std::string GetMsg()
	{
		if (!connected)
		{
			std::cerr << "\033[0;31m Error: No connected\033[0m"
				<< "\n";
			return "";
		}
		try
		{
			boost::beast::multi_buffer buffer;
			do
			{
				ws.read_some(buffer, 512);
			} while (!ws.is_message_done());
			std::string ret = boost::beast::buffers_to_string(buffer.data());
			buffer.consume(buffer.size());

			return ret;
		}
		catch (std::exception const &e)
		{
			std::cerr << "\033[0;31m Error: \033[0m" << e.what() << "\n";
			connected = false;
#ifdef DEBUG
			abort();
#endif
			return "";
		}
	}
	void SendMsg(std::string const &msg)
	{
		if (!connected)
		{
			std::cerr << "\033[0;31m Error: No connected\033[0m"
				<< "\n";
#ifdef DEBUG
			abort();
#endif
			return;
		}
		try
		{
			ws.write(boost::asio::buffer(msg));
		}
		catch (std::exception const &e)
		{
			connected = false;
			std::cerr
				<< "\033[0;31m Error: \033[0m" << e.what() << "\n";
#ifdef DEBUG
			abort();
#endif
		}
	}
	bool IsConnected()
	{
		return connected;
	}
};

#define SLEEP(_time) std::this_thread::sleep_for(std::chrono::milliseconds(_time))

class Api_For_GoCQ
{
private:
	ws_client *_ws_client = nullptr;
	std::queue<std::string> _msg_queue;
	std::map<int, std::string> _msg_map;
	std::mutex _mutex;

	std::queue<std::string> _event_queue;

	bool _is_running = false;
	void _Thread_RecvMsg()
	{
		while (_is_running)
		{
			if (!_ws_client || !_ws_client->IsConnected())
			{
				_is_running = false;
				return;
			}

			std::string msg = _ws_client->GetMsg(); // Json文本
			if (msg.empty())
			{
				SLEEP(50);
				continue;
			}
			boost::property_tree::ptree node;
			std::stringstream ss(msg);
			try
			{
				boost::property_tree::json_parser::read_json(ss, node);
			}
			catch (std::exception const &e)
			{
				std::cerr << msg << std::endl;
				std::cerr << "\033[0;31m Error: \033[0m" << e.what() << "\n";
#ifdef DEBUG
				abort();
#endif
				continue;
			}
			int key = -1;

			try
			{
				key = node.get<int>("echo");
			}
			catch (...)
			{
				_mutex.lock();
				_event_queue.push(msg);
				_mutex.unlock();
				continue;
			}

			_mutex.lock();
			_msg_map[key] = msg;
			_mutex.unlock();
		}
	}
	void _Thread_SendMsg()
	{
		while (_is_running)
		{
			if (!_ws_client || !_ws_client->IsConnected())
			{
				_is_running = false;
				return;
			}
			_mutex.lock();
			if (_msg_queue.empty())
			{
				_mutex.unlock();
				SLEEP(50);
				continue;
			}
			std::string msg = _msg_queue.front();
			_msg_queue.pop();
			_mutex.unlock();
			_ws_client->SendMsg(msg);
		}
	}

	std::thread *_thread_recv;
	std::thread *_thread_send;

public:
	Api_For_GoCQ(std::string const &host, std::string const &port)
	{
		_ws_client = new ws_client(host, port);
		if (!_ws_client->IsConnected())
		{
			delete _ws_client, _ws_client = nullptr;
			return;
		}
		_is_running = true;
		_thread_recv = new std::thread(&Api_For_GoCQ::_Thread_RecvMsg, this);
		_thread_send = new std::thread(&Api_For_GoCQ::_Thread_SendMsg, this);
	}
	~Api_For_GoCQ()
	{
		_is_running = false;
	
		if (_thread_recv != nullptr)
			_thread_recv->join(), _thread_recv = nullptr;
		if (_thread_send != nullptr)
			_thread_send->join(), _thread_send = nullptr;
		if(_ws_client!=nullptr)
			delete _ws_client,_ws_client=nullptr;
	}
	std::string PostMsg(const std::string &msg) // 超时自动退出
	{
		if (!_ws_client || !_ws_client->IsConnected())
		{
			std::cerr << "\033[0;31m Error: No connected\033[0m" << std::endl;
			return "";
		}
		boost::property_tree::ptree node;
		std::stringstream ss(msg);
		std::stringstream ss_out;
		int key = rand();
		try
		{
			boost::property_tree::json_parser::read_json(ss, node);

			node.put("echo", key);
			boost::property_tree::json_parser::write_json(ss_out, node);
		}
		catch (std::exception const &e)
		{
			std::cerr << "\033[0;31m Error: \033[0m" << e.what() << "\n";
#ifdef DEBUG
			abort();
#endif
			return "";
		}
		_mutex.lock();
		_msg_queue.push(ss_out.str());
		_mutex.unlock();

		int c_retry = 0;

		_mutex.lock();
		while (_msg_map.find(key) == _msg_map.end())
		{
			_mutex.unlock();
			SLEEP(50);
			c_retry++;
			if (c_retry > 100)
			{
				std::cerr << "\033[0;31m Error: \033[0m"
					<< "PostMsg Return timeout" << std::endl;
				return "";
			}
			_mutex.lock();
		}
		std::string ret = _msg_map[key];
		_msg_map.erase(key);
		_mutex.unlock();
		return ret;
	}
	std::string GetEvent()
	{
		_mutex.lock();
		if (_event_queue.empty())
		{
			_mutex.unlock();
			return "";
		}
		std::string ret = _event_queue.front();
		_event_queue.pop();
		_mutex.unlock();
		return ret;
	}
	bool IsAlive()
	{
		return _is_running;
	}
};