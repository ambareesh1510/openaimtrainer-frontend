config = {
    move = false;
    piercing = false;
    timer = 30.0;
}
local id = nil
local ticks = 0
local square_side = 8 
local occupied = {}
for i=1,square_side do
    for j=1,square_side do
        occupied[i * square_side + j] = 0
    end
end
local occupied_map = {}

function placeSphere()
    while (true) do
        x = math.random(square_side)
        y = math.random(square_side)
        if (occupied[x * square_side + y] == 0) then
            occupied[x * square_side + y] = 1
            break
        end
    end
    local id = addSphere(x * 0.5, y * 0.5 + 2., 0., 0.2);
    occupied_map[id] = {x, y}
end

function init()
    for i=1,4 do
        placeSphere()
    end
end

function update()
end

function onHit(id)
    removeTarget(id)
    local x = occupied_map[id][1]
    local y = occupied_map[id][2]
    occupied_map[id] = nil
    occupied[x * square_side + y] = 0
    placeSphere()
    incrementHitCount()
end

function onShoot()
    incrementShotCount()
end
