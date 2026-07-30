# LiteNgine
where is the nsfw?

<h1> 
Updates:
currently working on skeletal animation
fixed the pipeline (8th time btw)
working on stl imports and animation engine
working on engine programmable pipelines using nodesystem (thanks github node system for ImGui )
</h1>

current status : im in shanghai and my current laptop has genuinely ZERO vulkan support so expect subpar coding or me adding opengl support.ALSO, great firewall got me thinking some unpatriotic thoughts, and cloning the repo took longer than expected (500 errors) but we're finally here and the laptop is on its last legs (the screen edges are fading like a burning memory)


ok whatever

~~LiteNgine has the capacity to NOT WANT TO WORK *properly* on anything but the crappy igpu of an i5 10210U and nothing i do can change that.~~
it has been fixed and is now working on at least kepler series cards

LiteNgine is called liteNgine because i'm no longer allowed to comment on geopolitics

this engine currently had the capability to:

change engine preferences and save ~~(i'm going to have to be fully honest it does NOTHING with the data)~~ it can use the data

self diagnose

and contains:

Ui

Model Loading

*spin* ratte (rat not included)

basic first person camera movement

the ability to manipulate 3d models in the scene (transform and rotate)

smol viewport ( that can be constructed and reconstructed with any desired resolution *if you have enough vram)
fyi a 11k resolution texture takes up about 6gb of vram

image loading and display

offscreen rendering and present

multiple gpu support ( the engine has no reason to use it but you can run it in offscreen mode and do ray tracing on 5 nvidia tesla gpus)(this has not been tested yet)

includes mini profiler that can accurately make arrests (it counts frame times)

planned features: 

phone home feature (that is opt in) to show users on the cool map i have in my villain lair

opening the documentation in the browser

idk some hypnosis beam i haven't thought so far ahead

known issues

~~on nvidia gpus the main window is completely broken and only renders to 1/8th of the screen. reddit as of now has been belittling me and saying i should have used validation layers and to fix all of the bugs AS IF I HAVEN'T BEEN DOING THAT.~~

Sometimes throws a device lost error if you become too ambitious with the ui but in  my defense all ui code is written like that

~~every 30 seconds it throws a device lost error. to remedy this, im adding an autosave every TWENTY seconds~~
~~im not doing this just because i found out how to use strikethrough on markdown~~

literally 1984

67
