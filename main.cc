#include "crow.h"
// #include "crow_all.h"
#include "crow/json.h"

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
            std::cout << json_data["message"] << std::endl;
            });
    //set the port, set the app to run on multiple threads, and run the app
    app.port(18080).multithreaded().run();
}
