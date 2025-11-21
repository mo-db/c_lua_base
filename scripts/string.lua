-- no/empty {}  -> default...
-- empty x      -> default value
-- empty y/z    -> arg not set
--
-- q,r,s,t      -> global vars

local rule = [[
  A{x; ; 0.5*sin(z-1)}
  -
  B{x}
  [
    -{; ; z}
    A{; y+1}
    [
      -
      B{x; y-1}
      B
    ]
  ]

  # condensed
  A{x;;0.5*sin(z-1)}-B{x}[-{;;z}A{;y+1}[-B{x;y-1}B] ]

  # evaluated
  A{5;;3}-{0.5}B{5}[-{0.5;;9}A{2;3}[-{0.5}B{5;8}B{2}] ]

  # finished
  A{5}-{0.5}B{5}[-{0.5}A{2}[-{0.5}B{5}B{2}] ]

  # simple-print
  A-B[-A[-BB] ]
]] -- end
