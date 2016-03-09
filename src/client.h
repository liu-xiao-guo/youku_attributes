#ifndef CLIENT_H_
#define CLIENT_H_

#include <atomic>
#include <deque>
#include <map>
#include <memory>
#include <string>
#include <core/net/http/request.h>
#include <core/net/uri.h>

#include <QJsonDocument>

/**
 * Provide a nice way to access the HTTP API.
 *
 * We don't want our scope's code to be mixed together with HTTP and JSON handling.
 */
class Client {
public:

    /**
     * Client configuration
     */
    struct Config {
        typedef std::shared_ptr<Config> Ptr;

        // The root of all API request URLs
        std::string apiroot { "http://api.openweathermap.org" };

        // The custom HTTP user agent string for this library
        std::string user_agent { "example-network-scope 0.1; (foo)" };
    };

    /**
     * Data structure for the quried data
     */
    struct Data {
        std::string image;
        std::string link;
        std::string title;
        std::string view_count;
        std::string comment_count;
        std::string favorite_count;
        std::string up_count;
        std::string down_count;
    };

    /**
     * A list of weather information
     */
    typedef std::deque<Data> DataList;

    /**
     * Get the query data
     */
    virtual DataList getData(const std::string &query);

    Client(Config::Ptr config);

    virtual ~Client() = default;

    /**
     * Cancel any pending queries (this method can be called from a different thread)
     */
    virtual void cancel();

    virtual Config::Ptr config();

protected:
   void get(QString uri, QJsonDocument &root);

    /**
     * Progress callback that allows the query to cancel pending HTTP requests.
     */
    core::net::http::Request::Progress::Next progress_report(
            const core::net::http::Request::Progress& progress);

    /**
     * Hang onto the configuration information
     */
    Config::Ptr config_;

    /**
     * Thread-safe cancelled flag
     */
    std::atomic<bool> cancelled_;
};

#endif // CLIENT_H_

