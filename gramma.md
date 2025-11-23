# Gramma specification file
- this is how the gramma will be implemented in a script file
- a generator program can then load the grammar,
  add the variables and functions and then generate

# Generator functionality
- final grammar must be in form: S{v}S{v}...
- rules that dont specify parameter default to {category-default, 0.0, 0.0}
- parameters get evaluated from right to left

# SYMBOLS
- A-F

f(x)
g(x)


# Syntax
- a production has access
- there must be exactly one production for S (start)
A{x,u,v} : u < 5 : -AB -> A{f(x),u,v}-{u}[-BA{f(x)}]
A{x,u,v} : u < 5 : -AB -> A{f(x),u,v}-{u}[-BA{f(x)}]



# Definition Implementation

# Symbols

category: "action"
- A-F,a-f

category: "rotate"
- +,-

category: "width"
- $,%

category: "palette"
- ^,&

# default x-values
- action:   50.0
- rotate:   pi/2
- width:    10.0
- palette:  2


# impl
- gfx, generator in c
- scripting language for gramma spec, could generate rules programmatically
- use C++ as top layer to access libs -> imgui for gui sliders?
  - or generate gui from the config, implement sliders
