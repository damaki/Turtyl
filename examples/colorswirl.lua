clear()home()
setpencolor(gray)
for n=1,10000,0.05 do
  local r,g,b = pencolor()
  setpencolor(math.random(r-1,r+1),
              math.random(g-1,g+1),
              math.random(b-1,b+1))
  fd(n)
  rt(90 + (n/1000))
end