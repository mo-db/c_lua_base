# C:
enum ObjectType {};
struct DynamicObject {
  ObjectType;
  position;
};

// using func here is polymorphism?
struct Manipulator {
  object_id;
  bool done;
  func* manipulation_func(Manipulator, elapsed_time);
};

manpi_func(object)

# Lua:
