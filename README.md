# Novalis

## Overview

Novalis is a 2D game engine using SDL3 rendering facilities. It uses a scene graph structure, in which scenes (either `nv::BufferedNode `or `nv::DynamicNode`) store textures, polygons, and child nodes. 
There is also node editor GUI where users can save and create nodes by dragging and dropping polygons and textures, and uploading child nodes. In addition to nodes, library features include 
n-dimensional polygon objects (either `nv::BufferedPolygon` or `nv::DynamicNode)` for pixel-perfect collision detection, and an event system in which objects called `nv::EventHandler`'s store a list of type-erased callable objects to be invoked. 

## Node Editor Features
![alt text](https://github.com/walter298/Novalis/blob/master/NodeEditor/readme_assets/node_editor_use.png)

### Texture Contour Detection
Users can upload images and click a button to draw a hitbox perfectly around the contours of the image. Users can also manually draw hitboxes. 

### Object Groups
Users can create _object groups_ that have the effect of one when one object is "changed" (moved, scaled, etc), all other objects in that object's group similarly change. Users can specify
which actions objects groups sync. For example, a user might create an object group containing a texture and its corresponding hitbox, so that when the user drags the hitbox, the texture 
moves along with it. 

### Node Serialization and Deserialization
Users can save nodes to JSON files from the node editor. They can also load them from JSON in order to continue working on them, as well as upload them as child nodes into a parent. 

## Node Structure
There are two types of nodes:  `nv::BufferedNode` or `nv::DynamicNode`. A dynamic node is simply a node from which users can add and remove objects at runtime. They store objects inside 
`plf::hive`'s, in order to get decent cache locality while avoiding many iterator invalidation bugs. `nv::DynamicNode`s tend to have very high memory overhead, as `plf::hive` allocates
new buckets with exponentially large sizes. On ther other hand, `nv::BufferedNode` does not support the dynamic adding/removal of objects. However, every single object stored therein - including
child nodes - are backed by a single monotonic buffer. This provides excellent cache locality and prevents balooning memory usage. 



