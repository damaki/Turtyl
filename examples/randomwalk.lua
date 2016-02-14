clear()
home()

setscreencolor(black)
setpensize(1)
setpencolor(gray)

while true do
    -- Randomly modify the pen's color by +/- 1 for each RGB component
    local r,g,b = pencolor()
    setpencolor(r + math.random(-1,1),
                g + math.random(-1,1),
                b + math.random(-1,1))

    -- Move forwards by 1 pixel
    fd(1)

    -- Randomly change direction
    rt(math.random(0,359))
end
