# Turtyl

Turtyl is a turtle graphics program based on the [Lua](http://www.lua.org)
scripting language.


# Commands

_Some_ of the commands available in Turtyl are described below.

## Movement

You can use ``fd(n)`` and ``bk(n)`` to move the turtle forwards and backwards.
The turtle will draw a line over the path it has moved (if the turtle's pen is
down).
  * ``fd(n)`` moves the turtle forwards by ``n`` pixels.
  * ``bk(n)`` moves the turtle backwards by ``n`` pixels.

## Turning

You can use ``lt(n)`` and ``rt(n)`` to change the turtle's direction without
changing its position.
  * ``lt(n)`` turns the turtle ``n`` degrees to the left.
  * ``rt(n)`` turns the turtle ``n`` degrees to the right.

## Changing Color

You can use ``setpencolor(r,g,b,a)`` to change the pen's color by setting the
pen's RGBA components. Each color component can take a value in the range 0 to
255. For example, you can set the pen's color to red using ``setpencolor(255,0,0)``.

Turtyl defines some standard colors that you can give to ``setpencolor(color)``
to avoid needing to use RGBA values. For example, you can set the pen to the
color red by doing: ``setpencolor(red)``. 

The standard colors defined by Turtyl are:
  * white
  * black
  * red
  * darkRed
  * green
  * darkGreen
  * blue
  * darkBlue
  * cyan
  * darkCyan
  * magenta
  * darkMagenta
  * yellow
  * darkYellow
  * gray
  * darkGray
  * lightGray

You can also define your own colors, as shown in the following example:
```lua
mycolor = {r=120, g=240, b=120, a=255}

setpencolor(mycolor)
```

### Screen Color

You can also change the screen's background color using ``setscreencolor(r,g,b,a)``.
This is used the same way as ``setpencolor``.

## Pen Control

Normally, the turtle draws lines as you move using``fd(n)`` and ``bk(n)``.
However, you can lift the turtle's pen off the canvas to move the turtle
without drawing anything. 

  * ``pu()`` lifts the pen up. While the pen is up the turtle doesn't draw anything
    as it moves.
  * ``pd()`` drops the pen onto the canvas. After using ``pd()`` the turtle
    will start to draw on the canvas again.
    
## Arcs

You can draw arcs (curves) around the turtle using ``arc(angle, radius)``.
You give ``arc`` two numbers: the angle of the arc (in degrees), and the radius
of the arc (in pixels). The arc is drawn in a circular path around the turtle,
with the turtle at the center of the arc.

A circle is simply an arc with an angle of 360 degrees. For example, you can use 
``arc`` to draw a circle with a radius of 100 pixels as: ``arc(360, 100)``.


# License

Turtyl is licensed under the GPLv3. See LICENSE for more information.

The Qt license(s) can be viewed in the file QT_LICENSE.


# Building

Turtyl is based on Qt 5. To build Turtyl you will need to download the Qt toolkit
from https://www.qt.io/download/

Once Qt is installed, use Qt Creator to open the ``turtyl.pro`` project file.
You will need to perform first time configuration of the build, but the defaults
should be OK.

Once the build configuration is set, just hit the build button in Qt Creator.

After building you will need to copy the contents of the ``scripts`` directory
to the build directory (containing the turtyl executable).

It's also recommended to copy the file ``default_settings.ini`` to the build
directory also - renaming to ``settings.ini`` - in the build directory. This
ensures the default startup Lua scripts are run when the application launches.

Copying these files can be set up automatically on each build in Qt setting
a custom build step:
  1. Select "Projects" in the Qt Creator sidebar
  2. Select "Build & Run"
  3. Under "Build Steps" select "Custom Process Step" from the "Add Build Step"
     drop-down menu.
  4. Enter the following configuration:
    * **Command:** ``cp``
    * **Arguments:** ``-r %{sourceDir}/scripts %{buildDir}``
  5. Create another custom process step:
    * **Command:** ``cp``
    * **Arguments:** ``%{sourceDir}/default_settings.ini %{buildDir}/settings.ini``
     
