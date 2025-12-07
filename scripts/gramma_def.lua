-- *** init function ***
-- clear lmanager, then add
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
  builder0_id = {
    foo = "hallo",
    bar = "hallo",
  },
  builder1_id = {
    foo = "hallo",
    bar = "hallo",
  },
  default = {
    foo = "hallo",
    bar = "hallo",
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
  generator1_id = {
    defaults = {
    },
    globals = {
    },
    productions = {
    },
  },
  default = {
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
      'A{x,y,z} : u < 5 : bAB ! A[-{}A][+A]',
      'A : u < 5 : bAB ! A[-{}A][+A]',
    },
  },
}
generator_configs_len = 0
for _ in pairs(generator_configs) do
  generator_configs_len  = generator_configs_len + 1
end

-- functions that can be used inside a rule
-- e.g. 'A{x} : g(u) < 5 : bAB ! A[-{f(x)}A][+A]',
function f(x)
end

function g(x)
end

