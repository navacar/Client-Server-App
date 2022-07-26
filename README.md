# Client-Server-App
This is client-server app developed by me on C++ with using SQLite3 and Boost lib

App was developed under G++. 
To compile server use command: g++ -std=c++17 server.cpp database/database.cpp -l sqlite3  -o server
To compile client use command: g++ -std=c++17 client.cpp -o client

To activate server u need to pass port on call e. g. ./server 2000
Client has 2 modes. 
First it connects to server and sends packages with info with timedelay 5-30 sec. Example: ./client 127.0.0.1:2000
Second it asks server to send back statistic. Example: ./client 127.0.0.1:2000 --statistic
