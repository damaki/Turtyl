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

local function parsecolor(r,g,b, funcname)
    -- If only the 'r' parameter is given (the others are nil) then
    -- 'r' is treated as a color table (containing RGB values).
    if r ~= nil and g == nil and b == nil then
        if r["r"] ~= nil and
           r["g"] ~= nil and
           r["b"] ~= nil then
            return r.r, r.g, r.b
        else
            error("The color passed to " .. funcname .. "() must have 'r', 'g', and 'b' values")
        end

    else
       assert(g ~= nil and b ~= nil, "expected 3 number arguments to " .. funcname .. "()")

       return r,g,b
    end
end

-------------------------------------------------------------------------------
-- Standard colors
white       = {r=255, g=255, b=255}
black       = {r=0,   g=0,   b=0  }
red         = {r=255, g=0,   b=0  }
darkRed     = {r=128, g=0,   b=0  }
green       = {r=0,   g=255, b=0  }
darkGreen   = {r=0,   g=128, b=0  }
blue        = {r=0,   g=0,   b=255}
darkBlue    = {r=0,   g=0,   b=128}
cyan        = {r=0,   g=255, b=255}
darkCyan    = {r=0,   g=128, b=128}
magenta     = {r=255, g=0,   b=255}
darkMagenta = {r=128, g=0,   b=128}
yellow      = {r=255, g=255, b=0  }
darkYellow  = {r=128, g=128, b=0  }
gray        = {r=160, g=160, b=164}
darkGray    = {r=128, g=128, b=128}
lightGray   = {r=192, g=192, b=192}

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
                  self.pencolor.r, self.pencolor.g, self.pencolor.b,
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
                 self.pencolor.r, self.pencolor.g, self.pencolor.b,
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
    turtle:forward(distance)
end

function bk(distance)
    turtle:backward(distance)
end

function lt(degrees)
    turtle:left(degrees)
end

function rt(degrees)
    turtle:right(degrees)
end

function setpos(x, y)
    turtle:setpos(x,y)
end

function pos()
    return turtle.position.x, turtle.position.y
end

function setorientation(degrees)
    turtle.heading = math.rad(degrees)
end

function orientation()
    return math.deg(turtle.heading)
end

function arc(degrees, xradius, yradius)
    turtle:arc(degrees, xradius, yradius)
end

function pu()
    turtle.pendown = false
end

function pd()
    turtle.pendown = true
end

function setsize(size)
    turtle.thickness = size
end

function setpencolor(r,g,b)
    assert(r ~= nil, "missing argument to setpencolor()")

    r,g,b = parsecolor(r,g,b,"setpencolor")
    turtle.pencolor = {r=r, g=g, b=b}
end

function setscreencolor(r,g,b)
    assert(r ~= nil, "missing argument to setscreencolor()")

    r,g,b = parsecolor(r,g,b,"setscreencolor")
    set_background_color(r,g,b)
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
