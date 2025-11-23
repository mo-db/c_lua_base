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

function MV(n_trigons, tptr)
  -- local v = ffi.cast("Vec2*", vptr)
  local t = ffi.cast("Trigon*", tptr)
  local n = tonumber(n_trigons) - 1

  local test = 0.001


  for i=0, n, 1 do
    t[i].a.x = t[i].a.x + (t[i].a.x * test)
    t[i].a.y = t[i].a.y + (t[i].a.y * test)
    t[i].b.x = t[i].b.x + (t[i].b.x * test)
    t[i].b.y = t[i].b.y + (t[i].b.y * test)
    t[i].c.y = t[i].c.y + (t[i].c.x * test)
    t[i].c.y = t[i].c.y + (t[i].c.y * test)
  end

  -- t.a.x = t.a.x + 100
  -- t.a.y = t.a.y + 400
  -- t.b.x = t.b.x + 100
  -- t.b.y = t.b.y + 100
  -- t.c.y = t.c.y - 200
  -- t.c.y = t.c.y + 100
  -- t.v[0].x = 200
  -- t.v[0].y = 400
  -- t.v[1].x = 600
  -- t.v[1].y = 600
  -- t.v[2].x = 300
  -- t.v[2].y = 200
  --
  -- v[0].x = 900
  -- v[0].y = 300
  -- ffi.C.printf("v: %f, %f\n", ffi.new("float", v.x), ffi.new("float", v.y))
end


