# LiteNgine
liteNgine : the engine for everything but not for you

<h1> intro : </h1>
if i had to count the times i said, there’s probably a software for that and came out empty handed (or with a software that last had an active forum when club penguin’s was relevant) i’d hit the integer overflow. hence liteNgine. it’s a general purpose 3d engine written in vulkan with advanced techniques only i know well enough to use. 


demo video:

https://drive.google.com/file/d/1HSde0zeQrxcJ0TBBIv9N01f5XfMftKHP/view?usp=sharing

i’m tired boss just approve it

current status : im in shanghai and my current laptop has genuinely ZERO vulkan support so expect subpar coding or me adding opengl support.ALSO, great firewall got me thinking some unpatriotic thoughts, and cloning the repo took longer than expected (500 errors) but we're finally here and the laptop is on its last legs (the screen edges are fading like a burning memory)
ok whatever

<h2>thesis: what it is now?</h2>

nothing much really, but here’s a rundown
1. vulkan 1.4 dynamic rendering (it’s fast!)
2. cutting edge vulkan implementation
3. viewports! change size, scale, and look at whatever antialiasing you have
4. fyi, you can max out your vram if you make it too big and it would kill the program so be dumb with caution 
5. fully working ui code that’s still barely readable ( it won’t for long)
6. full model loading support ( any format except valve pack files)
7. skeletal animation (cutting edge during 1998)
8. screenshots for profiling (sometimes render doc cannot pull through)
9. preferences loading ( for those days when you are stuck with an old pc)
10. preferences saving. it’s important enough to warrant another entry
11. debug logging. very tenacious but lets you see how crap my code is
12. multiple windows and multiple layouts (also can be saved)
13. multiple model rendering and manipulation ( scale , translation , etc.)
14. phong shading (needs work)
15. performance profiler ( unfortunately unavailable to users as of now but there’s an fps tracker)
16. fps tracker . lets you see how incredibly fast vulkan is 
17. loading of texture images ( i’m sorry jd vance but someone has to be sacrificed)
18. intergrated lua support ( inaccessible from the engine as its still being implemented but you can tell it's there because the exe is a lot bigger)

<h1>planned features and technology</h1>
<h2>F.E.N.T</h2>
short for Fragmented Execution Node Tree, FENT combines the incredible flow control and readability given by node graphs with the easy going sequential execution of lua in a new way to experience speed.
<h2>Joint custody</h2>
hate the lack of atom families , star families and other families representation other than one parent and multiple children? introducing joint custody! now, children can have multiple parents, with the ability for changing influences (during weekends they stay with the other one)
<h2>Babie's first multithread</h2>
with the node graph,multithreading, mutexes, and synchronisation is a simple as chaining fence nodes together to ensure data arrives properly!

<h3> what’s in store? </h3>
check the "documentas" folder for my todo list.
great question. whatever places my adhd takes me.
1. modular loading of stuff. easy with assimp. just spin up a dll and voila
2. obscure input device support. like styluses and my palantir plasma ball
3. undo/ redo system. essential for anything really. 
4. the branching multiverse undo timeline. even more confusing for anything but its cool
4. simple model making
5. custom graphics pipeline
6. texture preview (easy thing)
7. animations ( working on it) 
8. node editor ( i just need to read someone else’s code)
9. image editor / animator (it’s for tigersoul)
10. vulkan memory allocator ( it’s really important)
11. very simple art program that will for sure balloon out of proportion 
12. blend file loading ???
13. shadow stencils > ray tr@cing 
14. unknowable descriptor heap bs
15. opengl loading option ( for my benefit really)

<h3>where are we going? no clue. </h3>
as it stands, liteNgine is heading towards murky waters as i’m heading to college, but i hope to steer it towards more flexible animation / game engine with a focus on non photorealistic rendering techniques. animation engine probably for a small series just to see how good it gets.  i’m also planning to combine 2d and 3d animation and see how well that goes

<h1> how do i see it / use it? </h1>
bad news. you probably can.
good news. it’s probably out of date. there’s a ton of stuff i was working on before i went to mainland china and out here it’s a dice throw if anything gets to happen.
liteNgine must be run on devices with vulkan 1.4 or more. this can be checked easily as it will immediately crash without it. in this case, consult a parent for a more modern graphics card.

<h2> to run, </h2>
head to the itch.io page and download the zip, extract, and click the exe. 
all files are automatically configured for this purpose to make the process as seamless as possible. 

the current build lacks many features as i am geographically disadvantaged as of now to make a new build. but it still features simple obj file loading and can work . probably. it’s very crash prone. treat it as how you would a stray cat and don’t make any sudden movements.

drag windows around and try not to do everything at once, because the ui is very crash prone , fly around a little in the viewport ( click the viewport and drag with wasd controls)

redecorate with the very dumb editor
(do NOT go negative i have no idea what happens)

and relaunch whenever to see your preferences saved! (only on work days though) 
