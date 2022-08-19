#pragma once

#include "json.h"
#include "transport_catalog.h"

#include <string>
#include <variant>


namespace Requests {
  struct Stop {
    std::string name;
    //Можно лучше: process оперерует с открытыми полями структуры, можно вытащить вне класса
    Json::Dict Process(const TransportCatalog& db) const;
  };

  struct Bus {
    std::string name;
    //Можно лучше: тоже самое и ниже
    Json::Dict Process(const TransportCatalog& db) const;
  };

  struct Route {
    std::string stop_from;
    std::string stop_to;

    Json::Dict Process(const TransportCatalog& db) const;
  };

  struct Map {
    Json::Dict Process(const TransportCatalog& db) const;
  };

  std::variant<Stop, Bus, Route, Map> Read(const Json::Dict& attrs);

  std::vector<Json::Node> ProcessAll(const TransportCatalog& db, const std::vector<Json::Node>& requests);
}
