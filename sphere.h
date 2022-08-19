#pragma once
//Можно лучше: этот хедер надо перенести в си-часть, иначе при подключении sphere.h будут дополнительные включения
#include <cmath>

namespace Sphere {
  double ConvertDegreesToRadians(double degrees);

  struct Point {
    double latitude;
    double longitude;
    //Можно лучше: нет смысла делать статическую функцию, когда поля открытые, лучше конструктор
    static Point FromDegrees(double latitude, double longitude);
  };

  //Можно лучше: передача объекта целиком - накладно
  double Distance(Point lhs, Point rhs);
}
