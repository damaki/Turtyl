local shapes = {}

-- Draw a regular N-sided shape with the turtle's current position at
-- the center of the shape.
local function centeredshape(nsides, sidelength)
    -- Remember the turtle's position and orientation so that it can be restored
    local x,y    = pos()
    local tangle = orientation()

    local interiorangle = 180 - (360 / nsides)
    local radius        = (sidelength / 2) / math.cos(math.rad(interiorangle/2))

    -- Make shapes with an even number of sides appear non-rotated
    if nsides % 2 == 0 then
        rt(180 - (interiorangle/2))
    end

    -- Move the turtle to one of the corners
    setpos(x + (math.sin(math.rad(orientation())) * radius),
           y + (math.cos(math.rad(orientation())) * radius))

    rt(180 - interiorangle/2)
    for n=1,nsides,1 do
        fd(sidelength)
        rt(180 - interiorangle)
    end

    -- Restore the turtle's original heading and direction.
    setpos(x,y)
    setorientation(tangle)
end

-- Draw a regular N-sided shape with the first edge as a line
-- in the turtle's current direction.
local function edgeshape(nsides, sidelength)
    local interiorangle = 180 - (360 / nsides)

    for n=1,nsides,1 do
        fd(sidelength)
        rt(180 - interiorangle)
    end
end

-- Draw a regular N-sided shapes.
--
-- The shape can be drawn either with the turtle's current position
-- at the shape's center, or with the turtle's current position at
-- one of the shape's corners.
function shapes.regular(nsides, sidelength, centered)
    if centered then
        centeredshape(nsides, sidelength)
    else
        edgeshape(nsides, sidelength)
    end
end

-- Helper function to define named shape functions such as triangle
local function namedshape(nsides)
    return function(size, centered)
        shapes.regular(nsides, size, centered)
    end
end

shapes.triangle  = namedshape(3)
shapes.square    = namedshape(4)
shapes.pentagon  = namedshape(5)
shapes.hexagon   = namedshape(6)
shapes.heptagon  = namedshape(7)
shapes.octagon   = namedshape(8)
shapes.decagon   = namedshape(10)
shapes.dodecagon = namedshape(12)

return shapes
