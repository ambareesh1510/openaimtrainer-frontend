config = {
    move = true;
    piercing = false;
    initialPosition = { 0., 2., 8. };
    initialTarget = { 0., 2., 0. };
    automatic = false;
    shotDelay = 1.0;
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
    drawCuboid(0, 0, 0, 100, 1.5, 100);
    drawCuboid(0, 0, -10, 100, 100, 1.5);
    drawCuboid(10, 0, 0, 1.5, 100, 100);
    drawCuboid(-10, 0, 0, 1.5, 100, 100);
    for i=1,num_spheres do
        placeSphere()
    end
end

local ammo = 6
local reloading = false
function update()
    if ammo == 0 then
        if getShotCooldown() == 0 then
            ammo = 6
        end
    end
    addUserInfo("Ammo", string.format("%d", ammo))
    addUserInfo("Remaining", string.format("%.2f", totalTime - getTime()))
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
    if ammo ~= 0 then
        ammo = ammo - 1
    end
    if ammo ~= 0 then
        setShotCooldown(0.0)
    end
end
