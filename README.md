# SlopeCraft
Get your 3D pixel painting in minecraft.
在minecraft中生成3d的地图画<br>
<br>

## Tutorials:
[SlopeCraftv3.1 Tutorial](./v3.1Tutorial/v3.1Tutorial.md)<br>[SlopeCraftv3.1 使用教程](./v3.1Tutorial/v3.1教程中文版.md)<br>
[SlopeCraft傻瓜级使用教程](./v3.0Tutorial/傻瓜级使用教程.md)<br>
<br>
## 软件简介<br>
SlopeCraft软件是我（TokiNoBug）用qt+Eigen+zlib开发的、用于在minecraft中生成立体地图画的软件。<br>
与spritecraft等像素画软件不同，SlopeCraft是专门面向地图的像素画，它设计的目的就是让你用地图将像素画记录下来，然后贴在物品展示框上。这里地图指游戏中可以记录地形的物品，不是存档。<br>
所以，SlopeCraft的颜色调整模块是完全面向地图的，它不是为了让像素画在玩家视角中接近原图，而是为了让像素画在地图的“视角”中接近原图。<br>
由于地图中的颜色与方块的相对高度有关，所以SlopeCraft生存的地图画往往不是平板的，而是立体的，是人工地形。我将其称为立体地图画。<br>
一言以蔽之，SlopeCraft是为地图而设计的。<br>
正是因此，利用SlopeCraft制作的地图画的“画质”，比spritecraft导出、再用地图记录的地图画要好很多——因为专业。<br>
<br>
<br>
## 地图画的游戏版本、类型、颜色空间和方块列表是怎么一回事？<br>
在SlopeCraft中，你可以调节地图画所对应的游戏版本、地图画类型、颜色空间和方块列表。我将一一简述这些参数是怎么回事。<br>
### 1.游戏版本：<br>
SlopeCraft支持的minecraft版本为1.12\~1.16。其中从1.12到1.15的地图没有什么变化，最多只有204种颜色；而1.16则为了下界树木增加了28种颜色，达到了232色。（这里的颜色忽略透明）<br>
### 2.地图画类型：<br>
Creative类型的地图画可以使用到完整的204/232种颜色，享受最高的画质；但它不能导出为立体地图画，
只能导出为地图文件，利用作弊的方法在存档中呈现。<br>关于这种方法，可以使用 "mc-map.djfun.de" 这个网站。<br>
Survival类型的地图画则牺牲了1/4的颜色，换来可以在原版生存实装的优点。这种地图画可以生成为三维地图画，也是这个软件的核心功能。<br>
Flat类型的地图画是平板的。由于立体地图画实装的难度很高，我特意开发了平板地图画的功能。<br>平板地图画顾名思义，是传统平坦的像素画，它最容易实装，但只有Creative类型1/4的颜色：51/58色。<br>
这里需要注明，Survival类型与Creative类型冲突，地图画只能为其中一种；但Flat类型的地图画与它们平行（虽然平坦的Creative没什么意义）。<br>
### 3.颜色空间：<br>
这是比较专业的部分。简单来说不同的颜色空间是用不同的方式去描述颜色。在SlopeCraft中，它代表着不同的调整颜色的方式。<br>
SlopeCraft中使用了4种颜色空间：RGB、HSV、Lab、XYZ。<br>
其中Lab模式对大多数图片的调整效果最好，我将它设为默认。但具体到每个像素画，你最好依次尝试一下，选择视觉效果最好的调整方案。<br>
### 4.方块列表：<br>
这是最为重要的部分，它与最终生成的地图画息息相关。简单来说，地图中的每个基础颜色往往对应着多种方块，但导出的时候我们只能给每个基础颜色选择一种对应的方块。<br>
所以方块列表就是建造像素画的“材料表”，称之为方块列表。我预设了4种方块列表，分别为：Vanilla，Cheap，Elegent和Shiny。<br>
Vanilla方块列表尽量还原每一种基础颜色的“原汁原味”，如沙子的颜色就对应砂岩、钻石块的颜色就对应钻石块等。
它往往不太适合生存实装（废话，3000个钻石块爽死你），只适合创造模式演示，因此我命名为Creative。<br>
Cheap是极度优化的方块列表，一切基础颜色都尽全力选择生存中最容易获得的、最廉价的方块，比如“下界”的颜色使用地狱岩、“白色”使用雪块。它适合生存的早期和中期。<br>
Elegent是“最优雅”的方块列表，它尽量选择了生存中可以量产，又“足够优雅”的方块，如“白色”使用白色混凝土。它适合生存的后期，是首选，也是默认。<br>
Shiny专为光影而生，它尽量选择可以发光的方块。<br>
Custom是自定义的方块列表，你可以自由的配置一切。<br>
<br>
<br>
## SlopeCraft的功能<br>
SlopeCraft包含以下功能：<br>
导入图片、调整颜色、生成立体地图画、导出为投影文件。<br>
<br>
## 安装方法：<br>
无需安装。<br>
<br>
## 使用方法：<br>
1.准备好你要转换的图片。最好事先用ps等编辑图片的软件（甚至是画图也可以）将图片裁剪、缩放，使图片的长和宽均为128像素的整数倍，这样最适合mc中的地图<br>
2.打开SlopeCraft.exe，在开始界面选择地图画的类别。<br>
3.点击 导入图片 按钮，选择你裁剪后的图片，程序会将这个图片导入。<br>
4.选择适当的游戏版本地图画类型，点击确认后，点击下一页。<br>
5.在方块列表页面中配置这幅地图画的方块列表。一般情况下你可以什么都不做，只点击确认，然后进入下一页。<br>
6.在调整颜色页面，选择任何一种颜色空间，点击调整颜色。完成这一步之后，点击导出为投影。（另外两个导出选项还没有完成）<br>
7.在导出为投影页面，设置投影名称、投影作者和投影区域名称。（你也可以不填）。然后先点击构建三维结构，待进度条完成后，点击导出为投影。导出完毕后，点击完成按钮。<br>
8.点击左下角的查看导出的文件按钮，或点击右下角的退出按钮，结束程序。<br>
