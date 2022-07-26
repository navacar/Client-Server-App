#include <sqlite3.h> 
#include <string>
#include <vector>
#include <boost/property_tree/ptree.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

class DataBase
{
public:
    DataBase();
    void addPackage(boost::property_tree::ptree pt);
    std::string getStatistic();

private:
    struct statStruct
    {
        std::string uuid;
        boost::posix_time::ptime time;
        float x;
        float y;
    };

    void createTables();

    void openDB();
    void closeDB();

    inline static int getUserData(void* data, int argc, char** argv, char** azColName);

private: 
    sqlite3 *db;

    inline static std::vector<std::string> usersVector;
    inline static std::vector<statStruct> userStatVector;
};