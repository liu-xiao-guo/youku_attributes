#include <query.h>
#include <localization.h>

#include <unity/scopes/Annotation.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/QueryBase.h>
#include <unity/scopes/SearchReply.h>
#include <unity/scopes/SearchMetadata.h>
#include <unity/scopes/VariantBuilder.h>
#include <QDebug>

#include <iomanip>
#include <iostream>
#include <sstream>

namespace sc = unity::scopes;
using namespace sc;
using namespace std;

const std::string NORMAL_TEMPLATE = R"(
{
  "schema-version": 1,
  "template": {
    "category-layout": "grid",
    "card-size": "medium",
    "overlay": false
  },
  "components": {
    "title": "title",
    "subtitle": "subtitle",
    "art" : {
      "field": "art",
      "aspect-ratio": 2.0
    },
    "attributes": {
        "field": "attributes",
        "max-count": 2
    }
  }
}
)";

const std::string VIDEOS_TEMPLATE = R"(
{
  "schema-version": 1,
  "template": {
    "category-layout": "grid",
    "card-size": "large",
    "overlay": true
  },
  "components": {
    "title": "title",
    "subtitle": "subtitle",
    "art" : {
      "field": "art",
      "aspect-ratio": 2.0
    }
  }
}
)";

Query::Query(const sc::CannedQuery &query, const sc::SearchMetadata &metadata,
             Client::Config::Ptr config) :
    sc::SearchQueryBase(query, metadata), client_(config) {
}

void Query::cancelled() {
    client_.cancel();
}


void Query::run(sc::SearchReplyProxy const& reply) {
    auto metadata_ = search_metadata();

    if (metadata_.is_aggregated()) {
        auto keywords = metadata_.aggregated_keywords();

        if ( (keywords.find("videos") != keywords.end()) ||
             (keywords.find("video") != keywords.end()) ) {
            qDebug() << "it is a video scope";
            do_videos_search(reply);
        }

    } else {
        qDebug() << "it is a normal video scope";
        do_normal_search(reply);
    }
}

QString qstr(std::string s) {
    return QString::fromStdString(s);
}

void Query::do_videos_search(sc::SearchReplyProxy const& reply) {
    try {
        // Start by getting information about the query
        const sc::CannedQuery &query(sc::SearchQueryBase::query());

        // Get the query string
        string query_string = query.query_string();

        Client::DataList datalist;
        datalist = client_.getData(query_string);

        CategoryRenderer rdrGrid(VIDEOS_TEMPLATE);
        auto grid = reply->register_category("youku", "Videos", "", rdrGrid);

        for (const Client::Data &data : datalist) {
            CategorisedResult catres(grid);

            catres.set_uri(data.link);
            catres.set_title(data.title);
            catres.set_art(data.image);

            // Push the result
            if (!reply->push(catres)) {
                // If we fail to push, it means the query has been cancelled.
                // So don't continue;
                return;
            }
        }

    } catch (domain_error &e) {
        // Handle exceptions being thrown by the client API
        cerr << e.what() << endl;
        reply->error(current_exception());
    }
}

void Query::do_normal_search(sc::SearchReplyProxy const& reply) {
    try {
        // Start by getting information about the query
        const sc::CannedQuery &query(sc::SearchQueryBase::query());

        // Get the query string
        string query_string = query.query_string();

        // Populate current weather category

        // the Client is the helper class that provides the results
        // without mixing APIs and scopes code.
        // Add your code to retreive xml, json, or any other kind of result
        // in the client.

        Client::DataList datalist;
        datalist = client_.getData(query_string);

        CategoryRenderer rdrGrid(NORMAL_TEMPLATE);
        auto grid = reply->register_category("youku", "Normal", "", rdrGrid);

        for (const Client::Data &data : datalist) {
            CategorisedResult catres(grid);

            catres.set_uri(data.link);
            catres.set_title(data.title);
            catres.set_art(data.image);

            QString likes = QString("%1 %2").arg(qstr(u8"\u261d "),qstr(data.up_count));
            QString views = QString("%1 %2").arg(qstr(u8"   \u261f "),qstr(data.down_count));
            std::string both = qstr("%1 %2").arg(likes,views).toStdString();
            sc::VariantBuilder builder;
            builder.add_tuple({
                {"value", Variant(both)}
            });
            builder.add_tuple({
                {"value", Variant("")}
            });
            catres["attributes"] = builder.end();

            // Push the result
            if (!reply->push(catres)) {
                // If we fail to push, it means the query has been cancelled.
                // So don't continue;
                return;
            }
        }

    } catch (domain_error &e) {
        // Handle exceptions being thrown by the client API
        cerr << e.what() << endl;
        reply->error(current_exception());
    }
}


