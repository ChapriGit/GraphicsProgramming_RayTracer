#pragma once
#include "Vector3.h"

using namespace dae;

class Tests
{
	private:
		bool static testDotResult(Vector3 v1, Vector3 v2, float result);
		bool static testCrossResult(Vector3 v1, Vector3 v2, Vector3 result);

	public:
		int static runTests();
};

