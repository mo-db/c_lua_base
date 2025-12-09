-- *** init function ***
-- clear lmanager, then add
generator0_id = 0
builder0_id = 0
builder1_id = 0
function lmanager_init(lmanager)
  generator0_id = _add_generator(lmanager);
  builder0_id = _add_builder(lmanager)
  builder1_id = _add_builder(lmanager)
end

-- *** interpreter config ***

--[[
  - interpreters use the generated lstring as source by default
  - change the string source from the config, can load from disk
--]]

builder_configs = {
  [0] = {
    pos = {700, 2100},
    angle = 3.14/2,
    segment_node_count = 3,
    generator_id = generator0_id,
  },
  default = {
    pos = {700, 2000},
    angle = 3.14/2,
    segment_node_count = 1,
    generator_id = generator0_id,
  },
}

builder_config_len = 0
for _ in pairs(builder_configs) do
  builder_config_len  = builder_config_len + 1
end

-- *** generator config ***

--[[
--]]

generator_configs = {
  [5] = {
    defaults = {
    },
    globals = {
    },
    productions = {
    },
  },
  default = {
    defaults = {
      move = 10.0,
      rotate = 0.1,
    },
    globals = {
      h = 0,
      i = 0,
    },
    productions = {
      'S ! A{}',
      'A : u < 5 : bAB ! BA[a-A][a+A]',
      'B : u < 5 : bAB ! A[a--A][a++A]',
    },
  },
}
generator_configs_len = 0
for _ in pairs(generator_configs) do
  generator_configs_len  = generator_configs_len + 1
end

productions_len = 0
for _ in pairs(generator_configs) do
  generator_configs_len  = generator_configs_len + 1
end

-- functions that can be used inside a rule
-- e.g. 'A{x} : g(u) < 5 : bAB ! A[-{f(x)}A][+A]',
function f(x)
end

function g(x)
end

