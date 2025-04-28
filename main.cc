#include "crow.h"
// #include "crow_all.h"
#include "crow/json.h"

class DatabaseController{
    private:
        string username;
        string dbname;
        string password;
        string server_addr;
    public:
        DatabaseController(string uname, string dbasename,string pwd,string serveraddr){
            username = uname;
            dbname=dbasename;
            password = pwd;
            server_addr = serveraddr;
            try {
                pqxx::connection conn("host= port=5432 dbname=postgres user=postgres password=1234");
                if (conn.is_open()) {
                    std::cout << "Connected successfully 🚀\n";
                } else {
                    std::cout << "Failed to connect ❌\n";
                }
            } catch (const std::exception &e) {
                std::cerr << e.what() << std::endl;
                return 1;
            }
        }
        ~DatabaseController(){
            std::cout << "Connection Stopped" << std::endl;        
        }
        void create_table(string tableNm){
            // code to create table in database
        }
        void fetch_table_data(string tableNm){
            // code to create table in database
        }
        void insert_data(string tableNm,string data){
            // code to insert data into table in database
        }
        void delete_table(string tableNm){
            // code to create table in database
        }
};

class Auth{
    private:
        string username;
        string password;
        string sessionToken;
    public:
        Auth(){
            std::cout << "Authentication Object initialized" << std::endl;        
        }
        void register(){
            // code to register the user
        }
        void login(){
            // code to register the user
        }
        ~Auth(){
            std::cout << "Authentication Object Shutting Down." << std::endl;        
        }
};
int main()
{
    crow::SimpleApp app; //define your crow application

    CROW_ROUTE(app, "/")([](){
        return "Hello world";
    });

    CROW_ROUTE(app, "/inbox")([](){
        auto page = crow::mustache::load_text("inbox.html");
        return page;
    });

    CROW_ROUTE(app, "/greet/<string>")([](std::string name){ // 
        auto page = crow::mustache::load("fancypage.html"); // 
        crow::mustache::context ctx ({{"person", name}}); // 
        return page.render(ctx); //
    });

    CROW_ROUTE(app,"/add/<int>/<int>")
    ([](int int1,int int2){
        return std::to_string(int1+int2);
    });

    CROW_ROUTE(app,"/subtract/<int>/<int>")
    ([](int int1,int int2){
        return std::to_string(int1-int2);
    });
    CROW_ROUTE(app,"/testjson")
    (
        [](){
            crow::json::wvalue test;
            test["hi"] = "string";
            return test;
        }
    );

    CROW_WEBSOCKET_ROUTE(app, "/ws")
    .onopen([&](crow::websocket::connection& conn){
            std::cout << "Opened the connection" << std::endl;
            })
    .onclose([&](crow::websocket::connection& conn, const std::string& reason, uint16_t){
        std::cout << "Closed the connection" << std::endl; 
            })
    .onmessage([&](crow::websocket::connection& /*conn*/, const std::string& data, bool is_binary){
            std::cout << "Got the message" << std::endl;
            std::cout << data << std::endl;
            auto json_data = crow::json::load(data);
            std::cout << json_data["message"] << std::endl
            
            });
    //set the port, set the app to run on multiple threads, and run the app
    app.port(18080).multithreaded().run();
}
