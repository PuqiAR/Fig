function fib(n) 
    if (n <= 1) then 
        return n
    else
        return fib(n - 1) + fib(n - 2) end
end

local start = os.clock()
local result = fib(30)
local endt = os.clock()
print(result, " cost: ", (endt - start) * 1000, "ms")