#include <client.h>

#include <core/net/error.h>
#include <core/net/http/client.h>
#include <core/net/http/content_type.h>
#include <core/net/http/response.h>
#include <QVariantMap>
#include <QDebug>

namespace http = core::net::http;
namespace net = core::net;

using namespace std;

const QString BASE_URI = "https://openapi.youku.com/v2/videos/by_category.json?&client_id=554ea75d941f42e4&category=TV";


Client::Client(Config::Ptr config) :
    config_(config), cancelled_(false) {
}

void Client::get( QString uri, QJsonDocument &root) {
    // Create a new HTTP client
    auto client = http::make_client();

    // Start building the request configuration
    http::Request::Configuration configuration;

    // Build the URI from its components
    configuration.uri = uri.toStdString();

    // Give out a user agent string
    configuration.header.add("User-Agent", config_->user_agent);

    // Build a HTTP request object from our configuration
    auto request = client->head(configuration);

    try {
        // Synchronously make the HTTP request
        // We bind the cancellable callback to #progress_report
        auto response = request->execute(
                    bind(&Client::progress_report, this, placeholders::_1));

        // Check that we got a sensible HTTP status code
        if (response.status != http::Status::ok) {
            throw domain_error(response.body);
        }
        // Parse the JSON from the response
        root = QJsonDocument::fromJson(response.body.c_str());

        // qDebug() << "response: " << response.body.c_str();
    } catch (net::Error &) {
    }
}


Client::DataList Client::getData(const string& query) {
    QJsonDocument root;

    QByteArray bytearray = query.c_str();
    QString query_string = QString::fromUtf8(bytearray.data(), bytearray.size());

    qDebug() << "query_string: " << query_string;

    QString uri = BASE_URI;
    qDebug() << "uri: "  << uri;
    get(uri, root);

    DataList result;

    QVariantMap variant = root.toVariant().toMap();

    // Iterate through the weather data
    for (const QVariant &i : variant["videos"].toList()) {
        QVariantMap item = i.toMap();

        QString image = item["thumbnail"].toString();
//        qDebug() << "image: " << image;

        QString link = item["link"].toString();
//        qDebug() << "link: " << link;

        QString title = item["title"].toString();
//        qDebug() << "title: " << title;

        QString view_count = item["view_count"].toString();
//        qDebug() << "view_count: " << view_count;

        QString comment_count = item["comment_count"].toString();
//        qDebug() << "comment_count: " << comment_count;

        QString favorite_count = item["favorite_count"].toString();
//        qDebug() << "favorite_count: " << favorite_count;

        QString up_count = item["up_count"].toString();
//        qDebug() << "up_count: " << up_count;

        QString down_count = item["down_count"].toString();
//        qDebug() << "down_count: " << down_count;

//        // Add a result to the weather list
        result.emplace_back(
            Data { image.toStdString(), link.toStdString(), title.toStdString(),
                   view_count.toStdString(), comment_count.toStdString(),
                   favorite_count.toStdString(), up_count.toStdString(),
                   down_count.toStdString() } );
    }

    return result;
}


http::Request::Progress::Next Client::progress_report(
        const http::Request::Progress&) {

    return cancelled_ ?
                http::Request::Progress::Next::abort_operation :
                http::Request::Progress::Next::continue_operation;
}

void Client::cancel() {
    cancelled_ = true;
}

Client::Config::Ptr Client::config() {
    return config_;
}

