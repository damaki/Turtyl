-------------------------------------------------------------------------------
-- Utilities

-- Snaps a float position to the nearest integer coordinates.
--
-- params:
--    pos - a table containing x & y keys
--
-- returns a copy of 'pos' with the x & y coordinates snapped to the nearest
--    integer coordinates.
local function snapposition(pos)
    local function snap(value)
        if value >= 0 then
            return math.floor(value + 0.5)
        else
            return math.ceil(value - 0.5)
        end
    end

    return {x=snap(pos.x), y=snap(pos.y)}
end

local function parsecolor(r,g,b,a)
    local newcolor = {r=0, g=0, b=0, a=255}

    if type(r) == "table" then
        if r['r'] ~= nil then
            newcolor.r = r.r
        end
        if r['g'] ~= nil then
            newcolor.g = r.g
        end
        if r['b'] ~= nil then
            newcolor.b = r.b
        end
        if r['a'] ~= nil then
            newcolor.a = r.a
        end

    else
        if r ~= nil then
            newcolor.r = r
        end
        if g ~= nil then
            newcolor.g = g
        end
        if b ~= nil then
            newcolor.b = b
        end
        if a ~= nil then
            newcolor.a = a
        end
    end

    return newcolor
end

-------------------------------------------------------------------------------
-- Standard colors
white       = {r=255, g=255, b=255, a=255}
black       = {r=0,   g=0,   b=0,   a=255}
red         = {r=255, g=0,   b=0,   a=255}
darkRed     = {r=128, g=0,   b=0,   a=255}
green       = {r=0,   g=255, b=0,   a=255}
darkGreen   = {r=0,   g=128, b=0,   a=255}
blue        = {r=0,   g=0,   b=255, a=255}
darkBlue    = {r=0,   g=0,   b=128, a=255}
cyan        = {r=0,   g=255, b=255, a=255}
darkCyan    = {r=0,   g=128, b=128, a=255}
magenta     = {r=255, g=0,   b=255, a=255}
darkMagenta = {r=128, g=0,   b=128, a=255}
yellow      = {r=255, g=255, b=0,   a=255}
darkYellow  = {r=128, g=128, b=0,   a=255}
gray        = {r=160, g=160, b=164, a=255}
darkGray    = {r=128, g=128, b=128, a=255}
lightGray   = {r=192, g=192, b=192, a=255}

-------------------------------------------------------------------------------
-- Turtle class.
--
-- The turtle class manages the state for a turtle:
--   * position
--   * heading
--   * pen color
--   * thickness
--   * pen state (up or down)
--   * pen thickness
local Turtle = {}
Turtle.__index = Turtle

function Turtle.new()
    local t = setmetatable({}, Turtle)

    t.position      = {x=0.0, y=0.0}
    t.pencolor      = black
    t.heading       = 0.0 -- turtle heading in radians
    t.thickness     = 1
    t.pendown       = true
    t.snapenabled   = true

    return t
end

function Turtle:forward(distance)
    local endpos = {}
    endpos.x = self.position.x + (distance * math.sin(self.heading))
    endpos.y = self.position.y + (distance * math.cos(self.heading))

    if self.snapenabled then
        endpos = snapposition(endpos)
    end

    if self.pendown then
        draw_line(self.position.x, self.position.y,
                  endpos.x, endpos.y,
                  self.pencolor.r, self.pencolor.g, self.pencolor.b, self.pencolor.a,
                  self.thickness)
    end

    self.position = endpos
end

function Turtle:backward(distance)
    self:forward(-distance)
end

function Turtle:right(degrees)
    self.heading = self.heading + math.rad(degrees)
end

function Turtle:left(degrees)
    self:right(-degrees)
end

function Turtle:arc(angle, xradius, yradius)
    if self.pendown then
        if yradius == nil then
            yradius = xradius
        end

        draw_arc(self.position.x, self.position.y,
                 math.deg(self.heading),
                 angle,
                 xradius,
                 yradius,
                 self.pencolor.r, self.pencolor.g, self.pencolor.b, self.pencolor.a,
                 self.thickness)
    end
end

function Turtle:setpos(x,y)
    local pos = {x=x, y=y}

    if self.snapenabled then
        self.position = snapposition(pos)
    else
        self.position = pos
    end
end

local turtle = Turtle.new()

-------------------------------------------------------------------------------
-- Standard Logo turtle commands

function fd(distance)
    assert(type(distance) == "number", "argument to fd() must be a number")
    turtle:forward(distance)
end

function bk(distance)
assert(type(distance) == "number", "argument to bk() must be a number")
    turtle:backward(distance)
end

function lt(degrees)
    assert(type(degrees) == "number", "argument to lt() must be a number")
    turtle:left(degrees)
end

function rt(degrees)
assert(type(degrees) == "number", "argument to rt() must be a number")
    turtle:right(degrees)
end

function setpos(x, y)
    turtle:setpos(x,y)
end

function pos()
    return turtle.position.x, turtle.position.y
end

function setorientation(degrees)
    assert(type(degrees) == "number", "argument to setorientation() must be a number")
    turtle.heading = math.rad(degrees)
end

function orientation()
    return math.deg(turtle.heading)
end

function arc(degrees, xradius, yradius)
    assert(type(degrees) == "number", "1st argument to arc() must be a number")
    assert(type(xradius) == "number", "2nd argument to arc() must be a number")
    assert(yradius == nil or type(yradius) == "number",
           "3rd argument to arc() must be a number")

    turtle:arc(degrees, xradius, yradius)
end

function pu()
    turtle.pendown = false
end

function pd()
    turtle.pendown = true
end

function setpensize(size)
    assert(type(size) == "number", "argument to setpensize() must be a number")
    turtle.thickness = size
end

function pensize()
    return turtle.thickness
end

function setpencolor(r,g,b,a)
    turtle.pencolor = parsecolor(r,g,b,a)
end

function pencolor()
    return turtle.pencolor.r, turtle.pencolor.g, turtle.pencolor.b, turtle.pencolor.a
end

function setscreencolor(r,g,b)
    local newcolor = parsecolor(r,g,b)
    set_background_color(newcolor.r, newcolor.g, newcolor.b)
end

function screencolor()
    return get_background_color()
end

function home()
    turtle.position = {x=0.0, y=0.0}
    turtle.heading  = 0.0
end

function circle(radius)
    local ispendown = turtle.pendown

    pu()
    rt(90)
    fd(radius)

    if ispendown then
        pd()
    end
    arc(360, radius)
    pu()
    bk(radius)
    lt(90)

    if ispendown then
        pd()
    end
end
