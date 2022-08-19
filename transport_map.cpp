#include "transport_map.h"

void Foo() {

}

TransportMap::TransportMap(const Descriptions::StopsDict& stops_dict,
    const Descriptions::BusesDict& buses_dict,
    const Json::Dict& render_settings_json)
    : render_settings_(MakeRenderSettings(render_settings_json)),
    layer_processor_({ {"bus_lines", std::bind(&TransportMap::PrintBusLine,this)}
                     , {"bus_labels", std::bind(&TransportMap::PrintBusLabels,this)}
                     , {"stop_points", std::bind(&TransportMap::PrintStopPoints, this)}
                     , {"stop_labels", std::bind(&TransportMap::PrintStopLabels, this)} }) {

    double min_lat = 100.0, max_lat = 0.0, min_lon = 100.0, max_lon = 0.0;
    for (const auto& [stop_name, stop_pointer] : stops_dict) {
        min_lat = std::min(min_lat, stop_pointer->position.latitude);
        max_lat = std::max(max_lat, stop_pointer->position.latitude);
        min_lon = std::min(min_lon, stop_pointer->position.longitude);
        max_lon = std::max(max_lon, stop_pointer->position.longitude);
        stops_[stop_name] = { stop_name , stop_pointer->position };
    }

    for (const auto& [bus_name, bus_pointer] : buses_dict) {
        buses_[bus_name] = { bus_pointer->name, {bus_pointer->stops}, bus_pointer->is_roundtrip };
    }

    CalculateRelativeCoordinates(min_lat, max_lat, min_lon, max_lon);
    CreateMap();
}

Svg::Color TransportMap::GetColorFromNode(const Json::Node& node) {
    
    Svg::Color col;
    if (std::holds_alternative<std::vector<Json::Node>>(node)) {
        const auto& ar = node.AsArray();
        if (ar.size() == 3) {
            col = Svg::Color(Svg::Rgb(ar[0].AsInt(), ar[1].AsInt(), ar[2].AsInt()));
        }
        else if (ar.size() == 4) {
            col = Svg::Color(Svg::Rgba(ar[0].AsInt(), ar[1].AsInt(), ar[2].AsInt(), ar[3].AsDouble()));
        }
    }
    else if (std::holds_alternative<std::string>(node)) {
        col = Svg::Color(node.AsString());
    }
    else {
        throw std::runtime_error("Unknown color");
    }
    return col;
}

TransportMap::RenderSettings TransportMap::MakeRenderSettings(const Json::Dict& json) {

    return {
        .width = json.at("width").AsDouble(),
        .height = json.at("height").AsDouble(),
        .padding = json.at("padding").AsDouble(),
        .stop_radius = json.at("stop_radius").AsDouble(),
        .line_width = json.at("line_width").AsDouble(),
        .stop_label_font_size = json.at("stop_label_font_size").AsInt(),
        .stop_label_offset = {json.at("stop_label_offset").AsArray()[0].AsDouble(),
                           json.at("stop_label_offset").AsArray()[1].AsDouble()},
        .underlayer_color = GetColorFromNode(json.at("underlayer_color")),
        .underlayer_width = json.at("underlayer_width").AsDouble(),
        .color_palette = [json]() {
                std::vector<Svg::Color> col_array;
                for (const auto& json_col_node : json.at("color_palette").AsArray()) {
                    col_array.push_back(GetColorFromNode(json_col_node));
                }
                return col_array; }(),
        .bus_label_font_size = json.at("bus_label_font_size").AsInt(),
        .bus_label_offset = { json.at("bus_label_offset").AsArray()[0].AsDouble(),
                                       json.at("bus_label_offset").AsArray()[1].AsDouble() },
        .layers = [json]() {
                    std::vector<std::string> l;
                    for (const auto& n : json.at("layers").AsArray()) {
                        l.push_back(n.AsString());
                    }
                    return l;
                }()
    };
}

void TransportMap::CalculateRelativeCoordinates(double min_lat, double max_lat, double min_lon, double max_lon) {

    double width_zoom_coef = (max_lon - min_lon == 0) ? std::numeric_limits<double>::max() :
        (render_settings_.width - 2 * render_settings_.padding) / (max_lon - min_lon);
    double height_zoom_coef = (max_lat - min_lat == 0) ? std::numeric_limits<double>::max() :
        (render_settings_.height - 2 * render_settings_.padding) / (max_lat - min_lat);
    double zoom_coef = std::min(width_zoom_coef, height_zoom_coef);
    if (zoom_coef == std::numeric_limits<double>::max()) {
        zoom_coef = 0;
    }

    for (auto& [name, bus_stop] : stops_) {
        bus_stop.out_coordinates =
        { (bus_stop.position.longitude - min_lon) * zoom_coef + render_settings_.padding,
        (max_lat - bus_stop.position.latitude) * zoom_coef + render_settings_.padding };
    }
}

void TransportMap::PrintBusLine() {

    // 0. Маршруты автобусов.
    size_t cnt = 0;
    for (const auto& [bus_name, bus_param] : buses_) {
        Svg::Polyline bus_route;
        size_t col_index = cnt % render_settings_.color_palette.size();
        bus_route.SetStrokeColor(render_settings_.color_palette[col_index]);
        bus_route.SetStrokeWidth(render_settings_.line_width);
        bus_route.SetStrokeLineCap("round");
        bus_route.SetStrokeLineJoin("round");
        for (const auto& bus_stop_name : bus_param.stops) {
            bus_route.AddPoint(stops_.at(bus_stop_name).out_coordinates);
        }
        map_.Add(std::move(bus_route));
        cnt++;
    }

}
void TransportMap::PrintBusLabels() {

    // 1. Номера автобусов
    size_t cnt = 0;
    for (const auto& [bus_name, bus_param] : buses_) {
        Svg::Text bus_name_pdl;
        bus_name_pdl.SetPoint(stops_[bus_param.stops.front()].out_coordinates);
        bus_name_pdl.SetOffset(render_settings_.bus_label_offset);
        bus_name_pdl.SetFontSize(render_settings_.bus_label_font_size);
        bus_name_pdl.SetFontFamily("Verdana");
        bus_name_pdl.SetFontWeight("bold");
        bus_name_pdl.SetData(bus_name);

        auto bus_name_txt = bus_name_pdl;

        bus_name_pdl.SetFillColor(render_settings_.underlayer_color);
        bus_name_pdl.SetStrokeColor(render_settings_.underlayer_color);
        bus_name_pdl.SetStrokeWidth(render_settings_.underlayer_width);
        bus_name_pdl.SetStrokeLineCap("round");
        bus_name_pdl.SetStrokeLineJoin("round");

        size_t col_index = cnt % render_settings_.color_palette.size();
        bus_name_txt.SetFillColor(render_settings_.color_palette[col_index]);

        map_.Add(bus_name_pdl);
        map_.Add(bus_name_txt);

        size_t middle_index = bus_param.stops.size() / 2;
        if ((!bus_param.is_roundtrip) && (bus_param.stops.front() != bus_param.stops[middle_index])) {
            bus_name_pdl.SetPoint(stops_[bus_param.stops[middle_index]].out_coordinates);
            bus_name_txt.SetPoint(stops_[bus_param.stops[middle_index]].out_coordinates);
            map_.Add(std::move(bus_name_pdl));
            map_.Add(std::move(bus_name_txt));
        }

        cnt++;
    }
}

void TransportMap::PrintStopPoints() {
    
    // 2. Круги автобусных остановок
    for (const auto& [stop_name, stop_param] : stops_) {
        Svg::Circle stop_circle;
        stop_circle.SetRadius(render_settings_.stop_radius);
        stop_circle.SetFillColor("white");
        stop_circle.SetCenter(stop_param.out_coordinates);
        map_.Add(std::move(stop_circle));
    }
}
void TransportMap::PrintStopLabels() {
    
    // 3. Названия автобусных остановок
    for (const auto& [stop_name, stop_param] : stops_) {
        Svg::Text nadpis;
        nadpis.SetPoint(stop_param.out_coordinates);
        nadpis.SetOffset(render_settings_.stop_label_offset);
        nadpis.SetFontSize(render_settings_.stop_label_font_size);
        nadpis.SetFontFamily("Verdana");
        nadpis.SetData(stop_name);

        auto podlozhka = nadpis;
        podlozhka.SetFillColor(render_settings_.underlayer_color);
        podlozhka.SetStrokeColor(render_settings_.underlayer_color);
        podlozhka.SetStrokeWidth(render_settings_.underlayer_width);
        podlozhka.SetStrokeLineCap("round");
        podlozhka.SetStrokeLineJoin("round");

        nadpis.SetFillColor("black");
        map_.Add(std::move(podlozhka));
        map_.Add(std::move(nadpis));
    }
}

void TransportMap::CreateMap() {
	
   /* PrintBusLine();
    PrintBusLabels();
    PrintStopPoints();
    PrintStopLabels();*/
    //layer_processor_.at("bus_lines")();
    //layer_processor_.at("bus_labels")();
    //layer_processor_.at("stop_points")();
    //layer_processor_.at("stop_labels")();
    for (const std::string layer : render_settings_.layers) {
        layer_processor_.at(layer)();
    }
     
}

std::string TransportMap::RenderMap() {
    std::stringstream ss;
    map_.Render(ss);
    return ss.str();
}

