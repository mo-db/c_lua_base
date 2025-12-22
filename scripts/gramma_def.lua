-- *** init function ***
-- clear lmanager, then add
--[[
  set autowriteall
  au InsertLeavePre,TextChanged,TextChangedP * if &modifiable && !&readonly | silent! update | endif
--]]


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
    pos = {900, 800},
    segment_node_count = 3,
    generator_id = state.generator1_id,
  },
  default = {
    pos = {900, 800},
    angle = 3.141592654/2,
    segment_node_count = 1,
    generator_id = state.generator0_id,
  },
}

-- *** generator config ***

--[[
--]]

-- default applies to all, use specific id to overwrite
generator_configs = {
  [3] = {
    defaults = {
      move = 10.0,
      rotate = 0.1,
    },
    globals = {
    },
    productions = {
    },
  },
  default = {
    defaults = {
      move = 30.0,
      rotate = 0.15,
    },
    globals = {
      h = 0,
      i = 0,
    },
    productions = {
      -- *** koch curve ***
      --[[
      'S ! A',
      'A ! A+A--A+A',
      --]]
      -- *** levy C curve ***
      --[[
      'S ! A',
      'A ! +A--A+',
      --]]
      -- *** basic coral ***
      --[[
      'S ! A',
      'A ! A[+A]A[-A]A',
      --]]
      -- *** lush tree ***
      --[[
      'S ! A',
      'A ! AA-[A-A+A+A]+[+A-A-A]',
      --]]
    },
  },
}

-- functions that can be used inside a rule
-- e.g. 'A{x} : g(u) < 5 : bAB ! A[-{f(x)}A][+A]',
function f(x)
end

function g(x)
end

-- l knowledge
-- [...] center-focused shape
