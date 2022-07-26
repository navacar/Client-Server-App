#include <boost/property_tree/json_parser.hpp>
#include <sqlite3.h>
#include "database.h"

using namespace std;


DataBase::DataBase()
{
   	openDB();
	createTables();
}


void DataBase::openDB()
{
	char *zErrMsg = 0;
	sqlite3_open("database/Statistic.db", &db);
}


void DataBase::createTables()
{
	char *zErrMsg = 0;

	char * sql = "CREATE TABLE IF NOT EXISTS Statistics("  \
		"UUID             TEXT    NOT NULL," \
		"Time             TEXT    NOT NULL," \
		"X                REAL     NOT NULL," \
		"Y                REAL     NOT NULL );";

	sqlite3_exec(db, sql, 0, 0, &zErrMsg);
}


void DataBase::addPackage(boost::property_tree::ptree pt)
{
	char *zErrMsg = 0;

	boost::property_tree::ptree::assoc_iterator it = pt.find("UUID");
	string uuid = it->second.data();
	
	it = pt.find("Time");
	string time = it->second.data();
	
	it = pt.find("X");
	string x = it->second.data();
	
	it = pt.find("Y");
	string y = it->second.data();

	const string command = "INSERT INTO Statistics (UUID, Time, X, Y) "  \
				"VALUES ('" + uuid + "', '" + time + "', " + x + ", " + y + ");";
	
	const char *sql = command.c_str();
	sqlite3_exec(db, sql, 0, 0, &zErrMsg);
}


int DataBase::getUserData(void* data, int argc, char** argv, char** azColName)
{
	bool needToAdd = true;
		
	for (int i = 0; i < usersVector.size(); i++)
	{
		if (usersVector[i] == string(argv[0]))
		{
			needToAdd = false;
			break;
		}
	}

	if (needToAdd)
	{
		usersVector.push_back(string(argv[0]));
	}

	statStruct newStat;
	newStat.uuid = string(argv[0]);
	newStat.time = boost::posix_time::time_from_string(string(argv[1]));
	newStat.x = atof(argv[2]);
	newStat.y = atof(argv[3]);

	userStatVector.push_back(newStat);

	return 0;
}


string DataBase::getStatistic()
{
	string result;
	char *zErrMsg = 0;

	string command = "SELECT * FROM Statistics";
	const char *sql = command.c_str();
	sqlite3_exec(db, sql, getUserData, 0, &zErrMsg);

	for (int j = 0; j < usersVector.size(); j++)
	{
		float oneMinuteX = 0, oneMinuteY = 0, fiveMinuteX = 0, fiveMinuteY = 0;
		int oneMinuteDelimiter = 0, fiveMinuteDelimiter = 0;

		for (int i = 0; i < userStatVector.size(); i++)
		{
			if (userStatVector[i].uuid == usersVector[j])
			{
				boost::posix_time::time_duration td = 
					boost::posix_time::microsec_clock::universal_time() -
						userStatVector[i].time;
				
				if (td.total_seconds() <= 60)
				{
					oneMinuteX += userStatVector[i].x;
					oneMinuteY += userStatVector[i].y;

					oneMinuteDelimiter++;
				}
				
				if (td.total_seconds() <= 300)
				{
					fiveMinuteX += userStatVector[i].x;
					fiveMinuteY += userStatVector[i].y;

					fiveMinuteDelimiter++;
				}
			}
		}

		result += usersVector[j];

		if (fiveMinuteDelimiter == 0 && oneMinuteDelimiter == 0)
		{
			result += " nothing to return";
		}
		else
		{
			if (oneMinuteDelimiter != 0)
			{
				result += " " + to_string(oneMinuteX / oneMinuteDelimiter)
					+ " " + to_string(oneMinuteY / oneMinuteDelimiter) + " "
					+ to_string(fiveMinuteX / fiveMinuteDelimiter) + " "
					+ to_string(fiveMinuteY / fiveMinuteDelimiter);
			}
			else
			{
				result += " nothing to return "
					+ to_string(fiveMinuteX / fiveMinuteDelimiter) + " "
					+ to_string(fiveMinuteY / fiveMinuteDelimiter);
			}
		}

		result += "\n";
	}

	userStatVector.clear();
	usersVector.clear();

	return result;
}

