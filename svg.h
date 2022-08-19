#pragma once
#include <iostream>
#include <cstddef>
#include <variant>
#include <optional>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>

namespace Svg {

    struct Point {
        Point() : x(0.0), y(0.0) {}
        Point(double ix, double iy) : x(ix), y(iy) {}
        double x;
        double y;
    };

    struct Rgb {
        Rgb() : red(0), green(0), blue(0) {}
        Rgb(uint8_t r, uint8_t g, uint8_t b) : red(r), green(g), blue(b) {}
        uint8_t red;
        uint8_t green;
        uint8_t blue;
    };

    struct Rgba {
        Rgba() : red(0), green(0), blue(0), alpha(0.0) {}
        Rgba(uint8_t r, uint8_t g, uint8_t b, double a) : red(r), green(g), blue(b), alpha(a) {}
        uint8_t red;
        uint8_t green;
        uint8_t blue;
        double alpha;
    };

    class Color {
    public:
        Color() : color_() {}
        Color(const char* str) : color_(std::string(str)) {}
        Color(const std::string& str) : color_(str) {}
        Color(const Rgb& icolor) : color_(icolor) {}
        Color(const Rgba& icolor) : color_(icolor) {}
        friend std::ostream& operator<<(std::ostream& o, const Color& col) {
            if (std::holds_alternative<std::string>(col.color_)) {
                o << std::get<std::string>(col.color_);
            }
            else if (std::holds_alternative<Rgb>(col.color_)) {
                const Rgb& rgb = std::get<Rgb>(col.color_);
                o << "rgb(" << static_cast<int>(rgb.red) << ","
                    << static_cast<int>(rgb.green) << ","
                    << static_cast<int>(rgb.blue) << ")";
            }            
            else if (std::holds_alternative<Rgba>(col.color_)) {
                const Rgba& rgb = std::get<Rgba>(col.color_);
                o << "rgba(" << static_cast<int>(rgb.red) << ","
                    << static_cast<int>(rgb.green) << ","
                    << static_cast<int>(rgb.blue) << ","
                    << (rgb.alpha) << ")";
            }
            else {
                o << "none";
            }
            return o;
        }
    private:
        std::variant<std::monostate, std::string, Rgb, Rgba> color_;
    };

    const Color NoneColor{};

    template <class T>
    class Figure {
    public:
        Figure() : line_width_(1.0) {}
        T& SetFillColor(const Color& col = NoneColor) {
            fill_color_ = col;
            return *(static_cast<T*>(this));
        }
        T& SetStrokeColor(const Color& col = NoneColor) {
            stroke_color_ = col;
            return *(static_cast<T*>(this));
        }
        T& SetStrokeWidth(double w = 1.0) {
            line_width_ = w;
            return *(static_cast<T*>(this));
        }
        T& SetStrokeLineCap(const std::string& s) {
            stroke_linecap_ = s;
            return *(static_cast<T*>(this));
        }
        T& SetStrokeLineJoin(const std::string& s) {
            stroke_linejoin_ = s;
            return *(static_cast<T*>(this));
        }
        friend std::ostream& operator<<(std::ostream& o, const Figure& f) {
            o << "fill=\"" << f.fill_color_ << "\" stroke=\"" << f.stroke_color_
                << "\" stroke-width=\"" << f.line_width_ << "\" ";

            if (!f.stroke_linecap_.empty()) {
                o << "stroke-linecap=\"" << f.stroke_linecap_ << "\" ";
            }
            if (!f.stroke_linejoin_.empty()) {
                o << "stroke-linejoin=\"" << f.stroke_linejoin_ << "\" ";
            }
            return o;
        }
    protected:
        Color fill_color_;
        Color stroke_color_;
        double line_width_;
        std::string stroke_linecap_;
        std::string stroke_linejoin_;
    };

    class Circle : public Figure<Circle> {
    public:
        Circle() : radius_(1.0) {}
        Circle& SetCenter(Point p) {
            center_ = p;
            return *this;
        }
        Circle& SetRadius(double r) {
            radius_ = r;
            return *this;
        }
        friend std::ostream& operator<<(std::ostream& o, const Circle& c) {
            o << "<circle ";
            o << "cx=\"" << c.center_.x << "\" cy=\"" << c.center_.y << "\" r=\"" << c.radius_ << "\" ";
            o << (Figure)c;
            o << "/>";
            return o;
        }
    private:
        Point center_;
        double radius_;
    };

    class Polyline : public Figure<Polyline> {
    public:
        Polyline& AddPoint(Point p) {
            coord_.push_back(p);
            return *this;
        }
        friend std::ostream& operator<<(std::ostream& o, const Polyline& p) {
            o << "<polyline points=\"";
            for (const auto& c : p.coord_) {
                o << c.x << "," << c.y << " ";
            }
            o << "\" ";
            o << (Figure)p;
            o << "/>";
            return o;
        }
    private:
        std::vector<Point> coord_;
    };

    class Text : public Figure<Text> {
    public:
        Text() : size_(1) {}
        Text& SetPoint(Point p) {
            coord_ = p;
            return *this;
        }
        Text& SetOffset(Point p) {
            offset_ = p;
            return *this;
        }
        Text& SetFontSize(uint32_t s) {
            size_ = s;
            return *this;
        }
        Text& SetFontFamily(const std::string& s) {
            family_ = s;
            return *this;
        }
        Text& SetFontWeight(const std::string& s) {
            weight_ = s;
            return *this;
        }
        Text& SetData(const std::string& s) {
            data_ = s;
            return *this;
        }
        friend std::ostream& operator<<(std::ostream& o, const Text& t) {
            o << "<text ";
            o << "x=\"" << t.coord_.x << "\" y=\"" << t.coord_.y << "\" ";
            o << "dx=\"" << t.offset_.x << "\" dy=\"" << t.offset_.y << "\" ";
            o << "font-size=\"" << t.size_ << "\" ";
            if (!t.family_.empty()) {
                o << "font-family=\"" << t.family_ << "\" ";
            }           
            if (!t.weight_.empty()) {
                o << "font-weight=\"" << t.weight_ << "\" ";
            }
            o << (Figure)t;
            o << ">";
            o << t.data_;
            o << "</text>";

            return o;
        }
    private:
        Point coord_;
        Point offset_;
        uint32_t size_;
        std::string family_;
        std::string data_;
        std::string weight_;
    };

    class Document {
    public:
        void Add(std::variant<Circle, Polyline, Text> obj) {
            objects_.push_back(obj);
        }
        void Render(std::ostream& o) {
            o << R"(<?xml version="1.0" encoding="UTF-8" ?>)";
            o << R"(<svg xmlns="http://www.w3.org/2000/svg" version="1.1">)";
            o << std::setprecision(10);
            for (const auto& obj : objects_) {
                visit([&o](auto&& arg) {o << arg; }, obj);
            }
            o << R"(</svg>)";

        }
    private:
        std::vector<std::variant<Circle, Polyline, Text>> objects_;
    };
}