#include "test_runner.h"
#include "descriptions.h"
#include "json.h"
#include "requests.h"
#include "sphere.h"
#include "transport_catalog.h"
#include "utils.h"

#include <iostream>
#include <fstream>
#include <functional>

using namespace std;

void MapTest1() {

	fstream is("MapTest1\\in.json", ios_base::in);
	if (!is.is_open()) {
		throw runtime_error("MapTest1. failed to open in.json");
	}

	fstream os("MapTest1\\out.json", ios_base::in | ios_base::out | ios_base::trunc);
	if (!os.is_open()) {
		throw runtime_error("MapTest1. failed to open out.json");
	}

	fstream svgs("MapTest1\\map.svg", ios_base::in | ios_base::out | ios_base::trunc);
	if (!svgs.is_open()) {
		throw runtime_error("MapTest1. failed to open map.svg");
	}

	fstream control("MapTest1\\control.json", ios_base::in);
	if (!control.is_open()) {
		throw runtime_error("MapTest1. failed to open control.json");
	}

	fstream control_map("MapTest1\\control_map.svg", ios_base::in);
	if (!control.is_open()) {
		throw runtime_error("MapTest1. failed to open control_map.svg");
	}

	const auto input_doc = Json::Load(is);
	const auto& input_map = input_doc.GetRoot().AsMap();

	const TransportCatalog db(
		Descriptions::ReadDescriptions(input_map.at("base_requests").AsArray()),
		input_map.at("routing_settings").AsMap(),
		input_map.at("render_settings").AsMap()
	);

	Json::PrintValue(
		Requests::ProcessAll(db, input_map.at("stat_requests").AsArray()),
		os
	);
	os << endl;
	os.seekp(0);

	svgs << db.RenderMapDebug();

	const Json::Node dest_node = Json::Load(os).GetRoot();
	const Json::Node exp_node = Json::Load(control).GetRoot();
	if (!(dest_node == exp_node)) {
		throw runtime_error("Output and control is not equal");
	}

	const string out_map_str = db.RenderMapDebug();
	std::string cont_map_sring((std::istreambuf_iterator<char>(control_map)),
		std::istreambuf_iterator<char>());
	if (out_map_str != cont_map_sring) {
		throw runtime_error("Map and control is not equal");
	}
	
}

void MapTest2() {

	fstream is("MapTest2\\in.json", ios_base::in);
	if (!is.is_open()) {
		throw runtime_error("MapTest2. failed to open in.json");
	}

	fstream os("MapTest2\\out.json", ios_base::in | ios_base::out | ios_base::trunc);
	if (!os.is_open()) {
		throw runtime_error("MapTest2. failed to open out.json");
	}

	fstream svgs("MapTest2\\map.svg", ios_base::in | ios_base::out | ios_base::trunc);
	if (!svgs.is_open()) {
		throw runtime_error("MapTest2. failed to open map.svg");
	}

	fstream control("MapTest2\\control.json", ios_base::in);
	if (!control.is_open()) {
		throw runtime_error("MapTest2. failed to open control.json");
	}

	fstream control_map("MapTest2\\control_map.svg", ios_base::in);
	if (!control.is_open()) {
		throw runtime_error("MapTest2. failed to open control_map.svg");
	}

	const auto input_doc = Json::Load(is);
	const auto& input_map = input_doc.GetRoot().AsMap();

	const TransportCatalog db(
		Descriptions::ReadDescriptions(input_map.at("base_requests").AsArray()),
		input_map.at("routing_settings").AsMap(),
		input_map.at("render_settings").AsMap()
	);

	Json::PrintValue(
		Requests::ProcessAll(db, input_map.at("stat_requests").AsArray()),
		os
	);
	os << endl;
	os.seekp(0);

	svgs << db.RenderMapDebug();

	//const Json::Node dest_node = Json::Load(os).GetRoot();
	//const Json::Node exp_node = Json::Load(control).GetRoot();
	//if (!(dest_node == exp_node)) {
	//	throw runtime_error("Output and control is not equal");
	//}

	const string out_map_str = db.RenderMapDebug();
	std::string cont_map_sring((std::istreambuf_iterator<char>(control_map)),
		std::istreambuf_iterator<char>());
	if (out_map_str != cont_map_sring) {
		throw runtime_error("Map and control is not equal");
	}

}

void TestAll() {
	TestRunner tr;
	tr.RunTest(MapTest1, "Map test 1");
	tr.RunTest(MapTest2, "Map test 2");
}

class MyClass {
public:
	MyClass() : caller({ {"ue", std::bind(&MyClass::Foo, this)} }) {
		caller.at("ue")();
		//function<void()> fff = std::bind(&MyClass::Foo, this);
		//caller["au"] = std::bind(&MyClass::Foo, this);
		//cout << "uh";
	}
	
private:
	void Foo() {
		std::cout << "Foo!" << std::endl;
	}
	const std::map<string, function<void()>> caller;
};

int main() {

	//MyClass m;
	////std::function<void()> f = std::bind(&MyClass::Foo, &m);
	////f();
	//return 0;

	//setlocale(LC_ALL, "rus");
	//cout.precision(6);
	//TestAll();
	//return 0;

  const auto input_doc = Json::Load(cin);
  const auto& input_map = input_doc.GetRoot().AsMap();

  const TransportCatalog db(
    Descriptions::ReadDescriptions(input_map.at("base_requests").AsArray()),
    input_map.at("routing_settings").AsMap(),
	input_map.at("render_settings").AsMap()
  );
  std::cerr << "buh" << endl;
  Json::PrintValue(
    Requests::ProcessAll(db, input_map.at("stat_requests").AsArray()),
    cout
  );
  cout << endl;

  return 0;
}
