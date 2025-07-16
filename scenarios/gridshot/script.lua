config = {
    move = false;
    piercing = false;
    initialPosition = { 0., 2., 8. };
    initialTarget = { 0., 2., 0. };
    automatic = false;
    shotDelay = 0.0;
}

local square_side = 3 
local num_spheres = 3
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
    local offset = math.floor((square_side + 1) / 2)
    local id = addSphere((x - offset) * 0.5, (y - offset) * 0.5 + 2., 0., 0.2);
    occupied_map[id] = {x, y}
end

function init()
    for i=1,num_spheres do
        placeSphere()
    end
end

function update()
end

function onHit(id)
    placeSphere()
    removeTarget(id)
    local x = occupied_map[id][1]
    local y = occupied_map[id][2]
    occupied_map[id] = nil
    occupied[x * square_side + y] = 0
    incrementHitCount()
    score = getScore()
    setScore(score + 1)
end

function onShoot()
    incrementShotCount()
end
