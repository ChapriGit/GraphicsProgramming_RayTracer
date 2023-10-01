#include "Tests.h"

bool Tests::testDotResult(Vector3 v1, Vector3 v2, float result)
{
    float calculatedResult = Vector3::Dot(v1, v2);

    if (calculatedResult >= result - 0.001 && calculatedResult <= result + 0.001) {
        return true;
    }

    return false;
}

bool Tests::testCrossResult(Vector3 v1, Vector3 v2, Vector3 result)
{
    Vector3 calculatedResult = Vector3::Cross(v1, v2);

    if (calculatedResult.x >= result.x - 0.001 && calculatedResult.x <= result.x + 0.001 &&
        calculatedResult.y >= result.y - 0.001 && calculatedResult.y <= result.y + 0.001 &&
        calculatedResult.z >= result.z - 0.001 && calculatedResult.z <= result.z + 0.001) {
        return true;
    }

    return false;
}

int Tests::runTests()
{
    if (!testDotResult(Vector3::UnitX, Vector3::UnitX, 1))      return 1;
    if (!testDotResult(Vector3::UnitX, -Vector3::UnitX, -1))    return 1;
    if (!testDotResult(Vector3::UnitX, Vector3::UnitY, 0))      return 1;

    if (!testCrossResult(Vector3::UnitZ, Vector3::UnitX, Vector3::UnitY))     return 2;
    if (!testCrossResult(Vector3::UnitX, Vector3::UnitZ, -Vector3::UnitY))    return 2;

    return 0;
}
