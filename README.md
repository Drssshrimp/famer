# famer UE C++ 项目运行说明

这是一个按题图需求完成的 Unreal Engine C++ 小项目：

- 每个节点倒计时 30 秒。
- 30 秒结束后，玩家操作会被暂停，并弹出 `Continue` 按钮。
- 点击 `Continue` 后进入下一个节点，并重新随机生成动物。
- 节点数量无限，直到玩家捕捉 10 只动物。
- 捕捉满 10 只动物后游戏结束。

## 操作方式

- `WASD`：移动玩家
- `Space` 或鼠标左键：靠近动物后捕捉动物
- 每 30 秒节点结束后：点击屏幕中央的 `Continue` 按钮进入下一节点

## 运行前准备

需要本机已安装：

- Unreal Engine 5
- Visual Studio 2022
- Visual Studio 中的 `使用 C++ 的游戏开发` 工作负载

仓库里的工程当前关联 UE 5.6。如果你的 UE 版本不是 5.6，打开工程时如果提示版本不一致，选择转换或切换到你本机安装的 UE5 版本即可。

## 运行步骤

1. 打开项目目录：

   `C:\Users\LZQ\Documents\GitHub\famer`

2. 找到工程文件：

   `famer.uproject`

3. 右键 `famer.uproject`，选择：

   `Generate Visual Studio project files`

   如果右键菜单里没有这个选项，可以先右键选择：

   `Switch Unreal Engine version`

   然后选择你本机安装的 UE5 版本。

4. 双击打开：

   `famer.uproject`

   或者打开生成出来的：

   `famer.sln`

5. 如果使用 Visual Studio：

   - 配置选择 `Development Editor`
   - 平台选择 `Win64`
   - 右键项目 `famer`
   - 点击 `Build`

6. 编译完成后回到 Unreal Editor，点击顶部的 `Play` 按钮即可运行游戏。

## 说明

这个仓库保留了原来的第三人称模板资源，并新增了 C++ 模块 `CatchAnimals`。运行时 C++ 会自动生成动物、界面和关卡流程，默认 GameMode 已切换到捕捉动物玩法。
