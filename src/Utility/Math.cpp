#include "stdafx.h"

float Math::FromToRange(float fValue, float fMinStartRange, float fMaxStartRange, float fMinEndRange, float fMaxEndRange) {
    return (fValue - fMinStartRange) *
           (fMaxEndRange - fMinEndRange) / (fMaxStartRange - fMinStartRange) + fMinEndRange;
}
