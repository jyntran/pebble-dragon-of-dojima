#pragma once
#include <pebble.h>

static const GPathInfo MINUTE_HAND_POINTS = {
  5,
  (GPoint []) {
  	{ 3, 0 },
    { 3, -60 },
	{ 0, -64 },
	{ -3, -60 },
	{ -3, 0 }
  }
};

static const GPathInfo HOUR_HAND_POINTS = {
  5,
  (GPoint []){
  	{ 3, 0 },
    { 3, -44 },
	{ 0, -48 },
    { -3, -44 },
	{ -3, 0 }
  }
};