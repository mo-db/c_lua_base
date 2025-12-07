T = {
  foo = {
    a = 5,
    b = 99,
    c = 1234.4321,
  },
  bar = {
    [0] = "func0",
    [1] = "func1",
    [2] = "func2",
  }
}
pink = {r=0.5, g=0.2, b=0.8}
pup= {[0]=0.5, [1]=0.2, [2]=0.8}
bok= {0.5, 0.2, 0.8}
pink_len = 0
for _ in pairs(pink) do
  pink_len = pink_len + 1
end

bobi = {[1]='blub', y='yoda', o='obiwan'}
bobi_len = 0
for _ in pairs(pink) do
  bobi_len = bobi_len + 1
end

-- 
T_count = 0
for _ in pairs(T) do
  T_count = T_count + 1
end
foo_count = 0
for _ in pairs(T.foo) do
  foo_count = foo_count + 1
end
bar_count = 0
for _ in pairs(T.bar) do
  bar_count = bar_count + 1
end

