local dynamics = {}

function dyn_behaviour1(co, dyn_object_id)
  print("inside dynamic behaviour")
  while true do
    lua_move_object(co, dyn_object_id, 100, 100, 1000.0)
    coroutine.yield();
    lua_move_object(co, dyn_object_id, 100, 500, 1000.0)
    coroutine.yield();
    lua_move_object(co, dyn_object_id, 500, 500, 1000.0)
    coroutine.yield();
    lua_move_object(co, dyn_object_id, 500, 100, 1000.0)
    coroutine.yield();
  end
end


function issue_next_task(co, dyn_object_id)
  print("inside issue next")
  if coroutine.status(dynamics[dyn_object_id].behaviour) ~= 'dead' then
    coroutine.resume(dynamics[dyn_object_id].behaviour, co, dyn_object_id)
  end
end

-- this is run at app startup once
function load_level(co, level)
  local size
  if level == 1 then
    size = {width = 9234, height = 9}
  else
    size = {width = 8, height = 4}
  end
  result = wrap_lua_create_level(co, size.width, size.height)
  print("[Lua] Called multiply_2_numbers(..) and the result is:" .. tostring(result))

  player_id = lua_create_dynamic_object(co, 500, 100)
  platform_id = lua_create_dynamic_object(co, 200, 700)
  platform_id1 = lua_create_dynamic_object(co, 400, 700)
  platform_id2 = lua_create_dynamic_object(co, 600, 300)

  lua_assign_player_control(co, player_id)

  dynamics[platform_id] = {behaviour = coroutine.create(dyn_behaviour1)}
  dynamics[platform_id1] = {behaviour = coroutine.create(dyn_behaviour1)}
  dynamics[platform_id2] = {behaviour = coroutine.create(dyn_behaviour1)}

  -- TODO: ArrList remove error
  issue_next_task(co, platform_id)
  -- issue_next_task(co, platform_id1)
  -- issue_next_task(co, platform_id2)

 end
