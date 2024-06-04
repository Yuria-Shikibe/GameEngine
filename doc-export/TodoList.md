<!-- TOC -->
* [Core](#core)
  * [Audio Engine Transplant](#audio-engine-transplant)
  * [Graphic API Transplant](#graphic-api-transplant)
  * [Clean up to MainLoopManager](#clean-up-to-mainloopmanager)
  * [Async Load Refactor](#async-load-refactor)
  * [Async Assets Load](#async-assets-load)
    * [Font Pack](#font-pack)
    * [Texture Pack](#texture-pack)
    * [Misc Task Manager](#misc-task-manager)
* [UI](#ui)
  * [Elements](#elements)
    * [Node Graph Editor](#node-graph-editor)
    * [Chamber Grid Editor](#chamber-grid-editor)
    * [Hitbox Editor](#hitbox-editor)
* [Game](#game)
  * [Async Manager (Maybe Basic Consumer Mode Temp)](#async-manager-maybe-basic-consumer-mode-temp)
  * [Render Pipe](#render-pipe)
    * [Light Source](#light-source)
    * [Normal Texture](#normal-texture)
    * [Fx](#fx)
    * [Transparent Draw](#transparent-draw)
  * [Entity](#entity)
    * [SpaceCraft Chamber Grid System](#spacecraft-chamber-grid-system)
    * [Chamber Grid System](#chamber-grid-system)
* [Misc](#misc)
<!-- TOC -->

# Core

## Audio Engine Transplant

## Graphic API Transplant

## Clean up to MainLoopManager

## Async Load Refactor
## Async Assets Load
* support hot reload 

### Font Pack
* Using shared texture packer

### Texture Pack
* support async bitmap load
* support AST dependency load

### Misc Task Manager
* Support staged task manage
> ig: Misc Init Load ...<tasks>... texture pack ...<tasks>... Misc End Load 

# UI
## Elements
### Node Graph Editor
### Chamber Grid Editor
### Hitbox Editor

# Game
## Async Manager (Maybe Basic Consumer Mode Temp)

## Render Pipe
### Light Source
### Normal Texture
### Fx
### Transparent Draw

## Entity
### SpaceCraft Chamber Grid System
* SpaceCraft Customization Support 
### Chamber Grid System
* Turret Customization Support

# Misc
* Remove Unnecessary Hook Call
* TextureNineRegion Scaled Draw
* Namespace & Module Rename
* Reflection Cleanup
* Json Optimization