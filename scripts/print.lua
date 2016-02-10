local function step(n1, n2, v, ...)
  if n1 > n2 then
    return nil
  else
    coroutine.yield(n1, v)
    return step(n1+1, n2, ...)
  end
end

local function init(...)
  coroutine.yield()
  return step(1, select("#", ...), ...)
end

local function vararg(...)
    local c = coroutine.wrap(init)
    c(...)
    return c
end

-- Override global print function to redirect messages to the UI
function print(...)
    local msg = nil
    for i, v in vararg(...) do
        s = tostring(v)
        if msg == nil then
            msg = s
        else
            msg = msg .. "\t" .. s
        end
    end

    _ui.print(msg)
end
