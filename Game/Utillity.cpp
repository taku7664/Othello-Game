#include "pch.h"
#include "Utillity.h"

GUID GenerateGUID()
{
	GUID guid;
	CoCreateGuid(&guid);
	return guid;
}