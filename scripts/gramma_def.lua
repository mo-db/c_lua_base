-- *** init function ***
-- clear lmanager, then add

-- persistent state survives dofile()


_G.lmanager_state = _G.lmanager_state or {}

local state = _G.lmanager_state

-- only define IDs once
state.generator0_id = state.generator0_id
state.generator1_id = state.generator1_id
state.builder0_id   = state.builder0_id
state.builder1_id   = state.builder1_id

function lmanager_init(lmanager)
  state.generator0_id = _add_generator(lmanager)
  state.generator1_id = _add_generator(lmanager)
  state.builder0_id = _add_builder(lmanager)
  state.builder1_id = _add_builder(lmanager)
end

-- *** interpreter config ***

--[[
  - interpreters use the generated lstring as source by default
  - change the string source from the config, can load from disk
--]]

-- default applies to all, use specific id to overwrite
builder_configs = {
  [0] = {
    pos = {400, 400},
    segment_node_count = 3,
    generator_id = state.generator1_id,
  },
  default = {
    pos = {800, 800},
    angle = 3.14/4,
    segment_node_count = 3,
    generator_id = state.generator0_id,
  },
}

-- *** generator config ***

--[[
--]]

-- default applies to all, use specific id to overwrite
generator_configs = {
  [1] = {
    defaults = {
      move = 20.0,
      rotate = 0.5,
    },
    globals = {
    },
    productions = {
    },
  },
  default = {
    defaults = {
      move = 80.0,
      rotate = 0.2,
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

-- functions that can be used inside a rule
-- e.g. 'A{x} : g(u) < 5 : bAB ! A[-{f(x)}A][+A]',
function f(x)
end

function g(x)
end

