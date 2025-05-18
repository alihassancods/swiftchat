# SwiftChat

SwiftChat is a modern, real-time chat application built using C++ and the [Crow](https://github.com/CrowCpp/crow) microframework. It supports user authentication, multi-user chat over WebSockets, and even integrates with DeepL for language translation.

## Features

- **User Signup & Login**: Secure authentication using JWT tokens.
- **Real-Time Chat**: WebSocket-based chat with session management.
- **Language Translation**: Translate messages using DeepL API.
- **PostgreSQL Integration**: User data and sessions are stored in a PostgreSQL database.
- **REST API**: Endpoints for user management and translation.
- **HTML Templates**: Renders HTML pages for login, signup, chat, and communities.

## Prerequisites

- C++17 compatible compiler (e.g., `g++`)
- [Crow](https://github.com/CrowCpp/crow)
- [nlohmann/json](https://github.com/nlohmann/json)
- [jwt-cpp](https://github.com/Thalhammer/jwt-cpp)
- [libpqxx](https://github.com/jtv/libpqxx)
- [libcurl](https://curl.se/libcurl/)
- [PostgreSQL](https://www.postgresql.org/)

## Setup

1. **Clone the Repository:**

    ```sh
    git clone https://github.com/alihassancods/swiftchat.git
    cd swiftchat
    ```

2. **Install Dependencies:**

    Make sure you have installed all the required libraries mentioned above. For Ubuntu, you can use:

    ```sh
    sudo apt-get install libpqxx-dev libcurl4-openssl-dev
    ```

    You may also need to install Crow, jwt-cpp, and nlohmann/json manually or via your package manager.

3. **Set Up Your Environment Variables:**

    Create a `data.env` file in the project root with the following content:

    ```
    IP_ADDR=your_postgres_host_ip
    API_KEY=your_deepl_api_key
    ```

4. **Prepare the Database:**

    Ensure PostgreSQL is running and create the required database and table:

    ```sql
    CREATE DATABASE swiftchat;
    \c swiftchat
    CREATE TABLE users (
        id SERIAL PRIMARY KEY,
        email TEXT UNIQUE NOT NULL,
        password TEXT NOT NULL,
        username TEXT NOT NULL,
        master_token TEXT
    );
    ```

    Default database connection assumes:
    - user: `postgres`
    - password: `1234`
    - port: `5432`
    - db: `swiftchat`

    Adjust these in `main.cc` or via `data.env` as needed.

5. **Build the Project:**

    ```sh
    g++ -std=c++17 -o websocket_app main.cc -lcurl -lcrypto -pthread -lpqxx
    ```

6. **Run the Server:**

    ```sh
    ./websocket_app
    ```

    The app will run on port `18080`.

## Usage

- Visit `http://localhost:18080/signup` to create a new account.
- Visit `http://localhost:18080/login` to log in.
- Visit `http://localhost:18080/chat` for the main chat interface.
- The `/translate` endpoint can be used for text translation.

## API Endpoints

- **POST** `/signup` – Register a new user (`email`, `password`)
- **POST** `/login` – Authenticate and receive a session token (`email`, `password`)
- **GET** `/profile` – User profile page (HTML)
- **GET** `/communities` – Communities list (HTML)
- **GET** `/chat` – Main chat (HTML)
- **GET** `/translate?text=your_text` – Translate text to English (uses DeepL)

## WebSocket

Connect to: `ws://localhost:18080/ws`

Handles real-time messaging between authenticated sessions.

## License

MIT

## Credits

- [Crow C++ Framework](https://github.com/CrowCpp/crow)
- [DeepL API](https://www.deepl.com/docs-api)
- [jwt-cpp](https://github.com/Thalhammer/jwt-cpp)
- [nlohmann/json](https://github.com/nlohmann/json)
- [libpqxx](https://github.com/jtv/libpqxx)
- [libcurl](https://curl.se/libcurl/)

## Author

[Ali Hassan](https://github.com/alihassancods)
[Nabeel Raza Khan](https://github.com/nabeelrkb)
