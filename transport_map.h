#pragma once
#include "json.h"
#include "svg.h"
#include "sphere.h"
#include "descriptions.h"
#include <vector>
#include <map>
#include <string>
#include <variant>
#include <stdexcept>
#include <algorithm>
#include <limits>
#include <sstream>
#include <functional>

class TransportMap
{
public:
	TransportMap(const Descriptions::StopsDict& stops_dict,
		const Descriptions::BusesDict& buses_dict,
		const Json::Dict& routing_settings_json);
	
	std::string RenderMap();

private:
	struct RenderSettings {
		double width;
		double height;
		double padding;
		double stop_radius;
		double line_width;
		int stop_label_font_size;
		Svg::Point stop_label_offset;
		Svg::Color underlayer_color;
		double underlayer_width;
		std::vector<Svg::Color> color_palette;
		int bus_label_font_size;
		Svg::Point bus_label_offset;
		std::vector<std::string> layers;
	};

	struct Stop {
		std::string name;
		Sphere::Point position;
		Svg::Point out_coordinates;
	};

	struct Bus {
		std::string name;
		std::vector<std::string> stops;
		bool is_roundtrip;
	};

	static RenderSettings MakeRenderSettings(const Json::Dict& json);
	static Svg::Color GetColorFromNode(const Json::Node& node);
	void CalculateRelativeCoordinates(double min_lat, double max_lat, double min_lon, double max_lon);
	void PrintBusLine();
	void PrintBusLabels();
	void PrintStopPoints();
	void PrintStopLabels();
	void CreateMap();


	RenderSettings render_settings_;
	std::map<std::string, Stop> stops_;
	std::map<std::string, Bus> buses_;
	Svg::Document map_;
	const std::map<std::string, std::function<void()>> layer_processor_;
	std::vector<std::string> layers_;
};

