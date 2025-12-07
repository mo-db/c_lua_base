-- *** interpreter config ***

--[[
  - interpreters use the generated lstring as source by default
  - change the string source from the config, can load from disk
--]]

segment_node_count = 3
position_x = 0
position_y = 0

_add_inter()
_remove_inter(inter_id)
_clear_inters()


-- first clear existing interpreters, then create
function create_inters()
  _clear_inters()
  interid0 = _add_inter()
  interid1 = _add_inter()
end

inter_configs = {
  interid0 = {
    foo = "hallo",
    bar = "hallo",
  },
  {
    id = interid1,
    blub = "yolo"
  }
}

inter_config_len = 0
for _ in pairs(inter_configs) do
  inter_config_len  = inter_config_len + 1
end



-- *** generator config ***

--[[
--]]

gen_config = {
  defaults = {
    move = 50,
    rotate = 0.2,
  },
  globals = {
    h = 0,
    i = 0,
  },
  productions = {
    'S ! A{}',
    'A{x} : u < 5 : bAB ! A[-{}A][+A]',
  },
}
gen_config_len = 0
for _ in pairs(gen_config) do
  gen_config_len  = gen_config_len + 1
end


-- defaults for x. u, w allways default to 0
move_default    = 50
rotate_default    = 0.2
width_default     = 0
palette_default   = 0

-- functions that can be used inside a rule
-- e.g. 'A{x} : g(u) < 5 : bAB ! A[-{f(x)}A][+A]',
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
