#include <Object/ObjectBase.hpp>
#include <iomanip>
#include <iostream>
#include <cmath>
#include <numbers>

int main()
{
    using namespace Fig;
    
    Value null;
    Value d = Value::FromDouble(-std::numbers::pi);
    Value i = Value::FromInt(-2143242);
    Value b = Value::FromBool(false);

    std::cout << null.ToString() << '\n';
    std::cout << d.ToString() << '\n';
    std::cout << i.ToString() << '\n';
    std::cout << b.ToString() << '\n';
}