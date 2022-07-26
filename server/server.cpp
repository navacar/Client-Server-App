#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <sqlite3.h>

#include <thread>
#include <iostream>

#include "database/database.h"

namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;
using namespace std;


const string sendBackStatisticComand = boost::beast::buffers_to_string(net::buffer("SEND_BACK_STATISTIC"));

void sendStatistic(boost::beast::websocket::stream<tcp::socket> *ws, DataBase *db)
{
    string statistic = db->getStatistic();
    ws->write(net::buffer(statistic));
}


void deserialize(DataBase *db, string out)
{
    stringstream ss;
    ss << out;
    boost::property_tree::ptree pt;
    boost::property_tree::read_json(ss, pt);

    db->addPackage(pt);
}


int main(int argc, char *argv[])
{
    if(argc > 1)
    {
        auto const host = boost::asio::ip::make_address("127.0.0.1");
        auto const port = static_cast<unsigned short>(atoi(argv[1]));

        boost::asio::io_context ioc{1};
        tcp::acceptor acceptor{ioc, {host, port}};

        DataBase db = DataBase();

        while(true)
        {
            tcp::socket socket{ioc};
            acceptor.accept(socket);

            thread{[q {move(socket)}, &db]()
            {
                boost::beast::websocket::stream<tcp::socket> ws {move(const_cast<tcp::socket&>(q))};
                ws.accept();

                while(true)
                {    
                    try
                    {
                        boost::beast::flat_buffer buffer;
                        ws.read(buffer);

                        string out = boost::beast::buffers_to_string(buffer.data());

                        if (out == sendBackStatisticComand)
                        {
                            sendStatistic(&ws, &db);
                        }
                        else
                        {
                            deserialize(&db, out);
                        }
                    }
                    catch (boost::beast::system_error const& se)
                    {
                        if ((se.code() != boost::beast::websocket::error::closed))
                        {
                            cout << se.code().message() << endl;
                            break;
                        }
                    }
                }
            }}.detach();
        }
    }
    else
    {
        cout << "Next time enter port!" << endl;
    }


    return 0;
}