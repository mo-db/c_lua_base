VERSION = os.time() -- or pick a literal version string you change manually
print("loaded test.lua VERSION=", VERSION)

local ffi = require("ffi")

ffi.cdef[[
int printf( const char* restrict format, ... );
typedef struct Vec2 {
	float x, y;
} Vec2;
typedef struct Trigon {
	union {
		struct {
			Vec2 a, b, c;
		};
		Vec2 v[3];
	};
} Trigon;
]]

A = 200
B = A * 50

ffi.C.printf("test: %s\n%s\n", "foo", "bar")

function MV(vptr, tptr)
  print("MV called; VERSION=", VERSION)
  local v = ffi.cast("Vec2*", vptr)
  local t = ffi.cast("Trigon*", tptr)

  -- t.a.x = t.a.x + 100
  -- t.a.y = t.a.y + 400
  -- t.b.x = t.b.x + 100
  -- t.b.y = t.b.y + 100
  -- t.c.y = t.c.y - 200
  -- t.c.y = t.c.y + 100
  -- t.v[0].x = 800
  -- t.v[0].y = 400
  -- t.v[1].x = 600
  -- t.v[1].y = 600
  -- t.v[2].x = 300
  -- t.v[2].y = 200

  v[0].x = 300
  v[0].y = 200
  -- ffi.C.printf("v: %f, %f\n", ffi.new("float", v.x), ffi.new("float", v.y))
end


