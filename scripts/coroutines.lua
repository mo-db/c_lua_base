function load_level(co, level)
  local size
  if level == 1 then
    size = {width = 9234, height = 9}
  else
    size = {width = 8, height = 4}
  end
  result = wrap_lua_create_level(co, size.width, size.height)
  print("[Lua] Called multiply_2_numbers(..) and the result is:" .. tostring(result))

  some_object = lua_create_dynamic_object(co, 500, 100)

 end
