# 临时信息：
本仓库暂时用于南京航空航天大学2023级C++程序设计课设用途

本人学号162320129

## 模块关系总览（.svg）
![svg_img](D:\projects\GameEngine\doc-export\Core_Module_Relations.svg)


# What will this project about and be

## Wrong Project Name Currently: This is not an engine at all!
> Well, this project is setup in about 2023.9, when at the beginning I do want to make my own game with c++. But I have no memory about why this project is named to be an *Engine*. 

Actually this project has nothing to do with engine, well, in general acknowledge, as it won't provide any game edit functionality in my expectation. Even at best it is only a game framework(with the source code in directory arc(abbr for architect) ). So I may rename it in the future when I have a proper name for this game, maybe ***Beyond Horizon*** or sth else. 

Anyway, remember: this project will only be an independent game, not an engine! *A Sci-fi 2D space theme game. *

## What will this game about?

Briefly speaking, this game will be a 2D *Sci-fi* **RPG-RTS** + Limited **Sandbox** game focus on Space Fleet & Station **Construction and Battle**. Survival wont be the essential part, ~~since I hate that~~.

Well, kind of mess. Just imagine a game combined FTL, Starbound(Or Space Haven), Homeworld and Event Horizon altogether. You control the fleet to blow something up and obtain goods to strengthen them. ~~Sounds sucks, right?~~

Multiple Player Is not supported in the initial stage, as I have no experience with net(esp with c++) at all.

# General 

Independent small games wont live long if it doesn't have interesting game systems and a active community. I believe the best way to reach the two point is provide a system with relatively high freedom. So I chose sandbox construction. And the whole game will go around this system.

---

## Construction Part
So how will the construction part work? This part will be sort of conventional, based on the **classic 2D tile map**, with different chambers to provide different effect and interactive with each other. But hey, don't be upset right now. I hope **all** the weapons and other facilities can be **customized** by player too. 

Imagine systems like Noita but is provided in 2D dimension! You can build your own weapon, equip it to the position you want, feed it with your own unique power system, and balance power usage between firing, shielding, life reserving and boosting during a fight with enemies with such unique equipment too.

This sounds, well at least in my opinion, is really cool. But one sad point is that the appearance cannot be customized, currently, which means only built-ins are valid. Anyway that will be the far story.

## Combat Part
Players can command a whole fleet to battle with enemies. This part is basically the combination of Command part in Homeworld and Power system in FTL. You distribute the energy to different parts, assign it a target or other actions, and reactive to enemy actions. Bullets will have real ballistics, collide with exact tiles or armors. So target the fatal core part to tear the hostile ship up. Also you may control the crew in your ship to fix the broken hull or speed up weapon reload speed or hit rate.

Noticed that command a whole fleet will be exhausting. So I expected to introduce customized **AI System**, basically organized with some built-in graphic programming language and translate them to lua, or directly work with lua. This part will be a big challenge to me, so this AI system may be done poor in the final stage. Anyway, there will be an AI system.

Crews and robots will on board to repair broken parts or speed-up systems. And I hope you can also command and teleport, or board them on hostile ships to perform a **Gang Fighting** to internally destroy the enemies.

## Cultivation Part
Survival wont be the essence, but it cannot be left aside. Any way, the main develop part is to build your own fleet, station or other things. Ranging from mining to trading and pirating. Basically you just upgrade your shipyard of your station, then upgrade your ship, your equipment, and boom other's ships. This is also a conventional part, may be optimized in the future.

You can also cultivate the relation with different factions, including accepting and completing their missions, buying their goods with discounts or suddenly announcing a war with them.

# Details

//TBD

## Machinery Parts On Your Ship
 
### Weaponry

### Defense

### Other Combat Supportive Parts

### Other Life Reserving Parts

### Industry & Storage

### Power

## Live Parts On Your Ship

### Crew

## Fleet Command

## AI

## Station

## Faction

## Map

# MOD




