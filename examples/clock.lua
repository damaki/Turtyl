function drawclock()
    -- Get the current time in seconds
    local time = os.time()

    -- Calculate the hour, minute, and second
    local hour   = (time / 3600) % 24
    local minute = (time / 60) % 60
    local second = time % 60

    ht()
    clear()
    home()
    setpencolor(black)
    setscreencolor(white)
    setpensize(10)
    setpencap(flatcap)

    -- Draw the clock face
    arc(360, 250)

    -- Draw the quarter markers
    for n=0,3,1 do
        home()
        rt(90*n)
        pu()
        fd(230)
        pd()
        fd(20)
    end

    -- Draw the hour markers
    setpensize(7)
    for n=0,11,1 do
        home()
        rt(30*n)
        pu()
        fd(240)
        pd()
        fd(10)
    end

    -- Draw the hour hand
    home()
    setpensize(10)
    rt((360/24) * hour)
    fd(125)

    -- Draw the minute hand
    home()
    setpensize(7)
    rt((360/60) * minute)
    fd(200)

    -- Draw the second hand
    home()
    setpensize(4)
    setpencolor(red)
    rt((360/60) * second)
    fd(215)
end

while true do
    drawclock()
    sleep(1)
end