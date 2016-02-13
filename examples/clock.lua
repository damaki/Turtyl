-- This example draws a Sierpinsky fractal.

function step(steps, size, radius)
  -- Termination condition for the recursion
  if steps <= 0 then
    return
  end

  -- Draw the upside-down triangle
  rt(60)
  shapes.triangle(size, true)
  lt(60)

  -- Remember the current position. We will move back here.
  local x,y = pos()

  -- Recursively draw each of the three sub-triangles
  for n=1,3,1 do
    pu()
    fd(radius)
    pd()
    
    step(steps-1, size/2, radius/2)

    setpos(x,y)
    rt(120)
  end
end

function fractal(steps, size)
  local radius = (size/2) / math.cos(math.rad(30))
  shapes.triangle(size, true)
  step(steps-1, size/2, radius/2)
end

clear()
home()
setpencolor(black)
setscreencolor(white)
setpensize(1)

fractal(11, 1000)