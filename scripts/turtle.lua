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

local function clip(value, min, max)
    if value < min then
        return min
    elseif value > max then
        return max
    else
        return value
    end
end

local function clipcolor(value)
    return clip(value,0,255)
end

local function parsecolor(r,g,b,a)
    local newcolor = {r=0, g=0, b=0, a=255}

    if type(r) == "table" then
        if r['r'] ~= nil then
            newcolor.r = clipcolor(r.r)
        end
        if r['g'] ~= nil then
            newcolor.g = clipcolor(r.g)
        end
        if r['b'] ~= nil then
            newcolor.b = clipcolor(r.b)
        end
        if r['a'] ~= nil then
            newcolor.a = clipcolor(r.a)
        end

    else
        if r ~= nil then
            newcolor.r = clipcolor(r)
        end
        if g ~= nil then
            newcolor.g = clipcolor(g)
        end
        if b ~= nil then
            newcolor.b = clipcolor(b)
        end
        if a ~= nil then
            newcolor.a = clipcolor(a)
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
-- Pen cap styles
flatcap   = 1
squarecap = 2
roundcap  = 3

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
    t.pencap        = roundcap

    return t
end

function Turtle:updateui()
    _ui.canvas.setturtle(self.position.x,
                         self.position.y,
                         math.deg(self.heading),
                         self.pencolor.r,
                         self.pencolor.g,
                         self.pencolor.b,
                         self.pencolor.a);
end

function Turtle:forward(distance)
    local endpos = {}
    endpos.x = self.position.x + (distance * math.sin(self.heading))
    endpos.y = self.position.y + (distance * math.cos(self.heading))

    if self.pendown then
        _ui.canvas.drawline(self.position.x, self.position.y,
                            endpos.x, endpos.y,
                            self.pencolor.r, self.pencolor.g, self.pencolor.b, self.pencolor.a,
                            self.thickness,
                            self.pencap)
    end

    self.position = endpos

    self:updateui();
end

function Turtle:backward(distance)
    self:forward(-distance)
end

function Turtle:right(degrees)
    self.heading = self.heading + math.rad(degrees)

    self:updateui();
end

function Turtle:left(degrees)
    self:right(-degrees)
end

function Turtle:arc(angle, xradius, yradius)
    if self.pendown then
        if yradius == nil then
            yradius = xradius
        end

        _ui.canvas.drawarc(self.position.x, self.position.y,
                           math.deg(self.heading),
                           angle,
                           xradius,
                           yradius,
                           self.pencolor.r, self.pencolor.g, self.pencolor.b, self.pencolor.a,
                           self.thickness,
                           self.pencap)
    end
end

function Turtle:arc2(angle, xradius, yradius)
    if yradius == nul then
        yradius = xradius
    end

    if self.pendown then
        _ui.canvas.drawarc(self.position.x, self.position.y,
                           math.deg(self.heading),
                           angle,
                           xradius,
                           yradius,
                           self.pencolor.r, self.pencolor.g, self.pencolor.b, self.pencolor.a,
                           self.thickness,
                           self.pencap)
    end

    -- Move the turtle position to the end of the arc
    local oldpos = {x=self.position.x, y=self.position.y}
    local pos = {x = yradius*math.sin(math.rad(angle)),
                 y = xradius*math.cos(math.rad(angle))}

    self.position.x = pos.x * math.cos(-self.heading) - pos.y * math.sin(-self.heading)
    self.position.y = pos.y * math.cos(-self.heading) + pos.x * math.sin(-self.heading)

    self.position.x = self.position.x + oldpos.x
    self.position.y = self.position.y + oldpos.y

    self.heading  = self.heading + math.rad(angle) + (math.pi / 2)

    self:updateui()
end

function Turtle:setpos(x,y)
    local pos = {x=x, y=y}
    self.position = pos

    self:updateui();
end

function Turtle:setorientation(degrees)
    self.heading = math.rad(degrees)

    self:updateui();
end

function Turtle:setpencolor(r,g,b,a)
    self.pencolor = parsecolor(r,g,b,a)

    self:updateui();
end

-- Table of all current turtles
local turtles = {}

-- Index or name of current turtle
local currturtle  = 1

-- Default turtle
turtles[currturtle] = Turtle.new()

-------------------------------------------------------------------------------
-- Standard turtle commands

function fd(distance)
    assert(type(distance) == "number", "argument to fd() must be a number")
    turtles[currturtle]:forward(distance)
end

function bk(distance)
assert(type(distance) == "number", "argument to bk() must be a number")
    turtles[currturtle]:backward(distance)
end

function lt(degrees)
    assert(type(degrees) == "number", "argument to lt() must be a number")
    turtles[currturtle]:left(degrees)
end

function rt(degrees)
assert(type(degrees) == "number", "argument to rt() must be a number")
    turtles[currturtle]:right(degrees)
end

function slideright(distance)
    rt(90)
    fd(distance)
    lt(90)
end

function slideleft(distance)
    lt(90)
    fd(distance)
    rt(90)
end

function setpos(x, y)
    turtles[currturtle]:setpos(x,y)
end

function pos()
    local t = turtles[currturtle]
    return t.position.x, t.position.y
end

function setorientation(degrees)
    assert(type(degrees) == "number", "argument to setorientation() must be a number")
    turtles[currturtle]:setorientation(degrees)
end

function orientation()
    return math.deg(turtles[currturtle].heading)
end

function arc(degrees, xradius, yradius)
    assert(type(degrees) == "number", "1st argument to arc() must be a number")
    assert(type(xradius) == "number", "2nd argument to arc() must be a number")
    assert(yradius == nil or type(yradius) == "number",
           "3rd argument to arc() must be a number")

    turtles[currturtle]:arc(degrees, xradius, yradius)
end

function arc2(degrees, xradius, yradius)
    assert(type(degrees) == "number", "1st argument to arc2() must be a number")
    assert(type(xradius) == "number", "2nd argument to arc2() must be a number")
    assert(yradius == nil or type(yradius) == "number",
           "3rd argument to arc2() must be a number")

    turtles[currturtle]:arc2(degrees, xradius, yradius)
end

function pu()
    turtles[currturtle].pendown = false
end

function pd()
    turtles[currturtle].pendown = true
end

function pendown()
    return turtles[currturtle].pendown
end

function ht()
    _ui.canvas.hideturtle()
end

function st()
    _ui.canvas.showturtle()
end

function clear()
    _ui.canvas.clear()
end

function setpensize(size)
    assert(type(size) == "number", "argument to setpensize() must be a number")
    turtles[currturtle].thickness = size
end

function pensize()
    return turtle.thickness
end

function setpencolor(r,g,b,a)
    turtles[currturtle]:setpencolor(r,g,b,a)
end

function pencolor()
    local t = turtles[currturtle]
    return t.pencolor.r, t.pencolor.g, t.pencolor.b, t.pencolor.a
end

function setpencap(cap)
    assert(type(cap) == "number", "argument to setpencap() must be an integer")
    assert(cap >= 1 and cap <= 3, "argument to setpencap() must be 1, 2, or 3")
    turtles[currturtle].pencap = cap
end

function pencap()
   return turtles[currturtle].pencap
end

function setscreencolor(r,g,b)
    local newcolor = parsecolor(r,g,b)
    _ui.canvas.setbackgroundcolor(newcolor.r, newcolor.g, newcolor.b)
end

function screencolor()
    return _ui.canvas.getbackgroundcolor()
end

function home()
    setpos(0,0)
    setorientation(0)
end

function line(length)
    if turtles[currturtle].pendown then
        local x,y = pos() -- remember position
        pu()
        bk(length/2)
        pd()
        fd(length)
        setpos(x,y) -- restore position
    end
end

function setturtle(name)
    if name == nil then
        return
    end

    if turtles[name] == nil then
        turtles[name] = Turtle.new()
    end
    currturtle = name

    turtles[name]:updateui()
end

function aaon()
    _ui.canvas.setaa(true)
end

function aaoff()
    _ui.canvas.setaa(false)
end
