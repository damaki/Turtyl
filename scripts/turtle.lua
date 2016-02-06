local Turtle = {}
Turtle.__index = Turtle

function Turtle.new()
    local t = setmetatable({}, Turtle)

    t.position  = {x=0.0, y=0.0}
    t.pencolor  = {r=255, g=255, b=255}
    t.heading   = 0.0 -- turtle heading in radians
    t.thickness = 1
    t.pendown   = true

    return t
end

function Turtle:forward(distance)
    local endpos = {}
    endpos.x = self.position.x + (distance * math.sin(self.heading))
    endpos.y = self.position.y + (distance * math.cos(self.heading))

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

function Turtle:arc(radius, angle)
    if self.pendown then
        draw_arc(self.position.x, self.position.y,
                 math.deg(self.heading),
                 angle,
                 radius,
                 self.pencolor.r, self.pencolor.g, self.pencolor.b,
                 self.thickness)
    end
end

local turtle = Turtle.new()

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

function arc(radius, degrees)
    turtle:arc(radius, degrees)
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

function color(r,g,b)
    if r == nil then
        error("missing argument to color()")

    else
        -- If only the 'r' parameter is given (the others are nil) then
        -- 'r' is treated as a color table (containing RGB values).
        if r ~= nil and g == nil and b == nil then
            if r["r"] ~= nil and
               r["g"] ~= nil and
               r["b"] ~= nil then
                turtle.pencolor = r
            else
                error("invalid color")
            end

        else
            if r == nil then
               r = 0
            end
            if g == nil then
               g = 0
            end
            if b == nil then
               b = 0
            end

            turtle.pencolor = {r=r, g=g, b=b}
        end
    end
end

function home()
    turtle.position = {x=0.0, y=0.0}
    turtle.heading  = 0.0
end
