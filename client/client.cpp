#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <boost/asio.hpp>

#include <iostream>
#include <thread>
#include <random>
#include <ctime>

using namespace std;

namespace beast = boost::beast;         
namespace http = beast::http;           
namespace websocket = beast::websocket; 
namespace net = boost::asio;            
using tcp = boost::asio::ip::tcp;

bool delimHostAndPort(string &host, string &port)
{
    bool isHostAndPortValid = false;
    size_t pos = host.find(":");
    size_t posOfEnd = distance(host.begin(), host.end());

    if (pos != string::npos)
    {
        isHostAndPortValid = true;
        port = host.substr(pos + 1, posOfEnd);
        host.erase(pos, posOfEnd);
    }

    return isHostAndPortValid;
}


string getTime()
{
    string result = boost::posix_time::to_simple_string(
        boost::posix_time::microsec_clock::universal_time());
    return result;
}


float generateNumber()
{
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dist(-90, 90);

    return dist(gen);
}


string generateUUID()
{
    string uuid = boost::uuids::to_string(boost::uuids::random_generator()());
    return uuid;
}


string generateJson(string uuid)
{
    ostringstream result;

    boost::property_tree::ptree pt;
    boost::property_tree::ptree children[4];

    children[0].put("", uuid);
    children[1].put("", getTime());
    children[2].put("", generateNumber());
    children[3].put("", generateNumber());

    pt.add_child("UUID", children[0]);
    pt.add_child("Time", children[1]);
    pt.add_child("X", children[2]);
    pt.add_child("Y", children[3]);

    boost::property_tree::json_parser::write_json(result, pt);

    return result.str();
}


void timeDelay()
{
    srand(time(nullptr));
    int time = rand() % 26 + 5;
    this_thread::sleep_for(chrono::milliseconds(time * 1000));
}


void connect(string &host, string &port, bool isStatistic)
{
    try
    {
        string uuid = generateUUID();
        net::io_context ioc;

        tcp::resolver resolver{ioc};
        websocket::stream<tcp::socket> ws{ioc};

        auto const results = resolver.resolve(host, port);
        net::connect(ws.next_layer(), results.begin(), results.end());

        ws.set_option(websocket::stream_base::decorator(
            [](websocket::request_type& req)
            {
                req.set(http::field::user_agent,
                    string(BOOST_BEAST_VERSION_STRING) +
                        " websocket-client-coro");
            }));
        
        ws.handshake(host, "/");
        
        while(true)
        {
            if (isStatistic)
            {
                ws.write(net::buffer("SEND_BACK_STATISTIC"));
                beast::flat_buffer buffer;
                ws.read(buffer);
                ws.close(websocket::close_code::normal);
                cout << beast::make_printable(buffer.data()) << endl;
                break;
            }
            else 
            {
                ws.write(net::buffer(generateJson(uuid)));
            }
            
            timeDelay();
        }
    }
    catch (exception const& e)
    {
        cerr << "Error: " << e.what() << endl;
    }
}


int main(int argc, char *argv[])
{
    string host, port;
    bool isStatistic = false;
    bool isConnectionDataValid = false;

    if (argc > 1)
    {
        host = argv[1];
        isConnectionDataValid = delimHostAndPort(host, port);
    
        if (argc > 2)
        {
            string statistic = argv[2];
            isStatistic = statistic == "--statistic" ? true : false;
        }
    }

    if (isConnectionDataValid)
    {
        connect(host, port, isStatistic);   
    }
    else 
    {
        cout << "Connection data invalid" << endl;
    }
    

    return 0;
}