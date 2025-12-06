-- Blueprint for gramma spec file
-- *** interpreter config ***
segment_node_count = 3

-- *** generator config ***
-- defaults for x. u, w allways default to 0
move_default    = 50
rotate_default    = 0.2
width_default     = 0
palette_default   = 0

-- functions that can be used inside a rule
function f(x)
end

function g(x)
end

-- globals that can be used inside rules
h = 0
i = 0
j = 0
k = 0

-- productions
-- Symbol (opt)Parameters : (opt)condition : (opt)context -> replacement
productions = {
  [0] = 'S ! A{}',
  [1] = 'A{x} : u < 5 : bAB ! A[-{}A][+A]',
  -- ...
}

productions_size = #productions + 1
