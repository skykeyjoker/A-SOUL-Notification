# A-SOUL Notification
一个A-SOUL成员动态/直播提醒插件，仅能在Windows10-Windows11系统上运行。

* 注：现在已经实现自定义用户查询，可以编辑`member.json`文件设置想要查询的用户。

使用Qt5编写，核心实现为Request请求+Json库解析+Wintoast消息。

为应对B站接口限制，目前查询为5秒/次，因此消息推送可能会有较短延迟（最大延迟1min左右）。

注：本插件为控制台程序（Console Application），无界面无托盘。可以通过查询`ASoulNotification.exe`进程检查插件运行状态。

## 功能介绍

### 启动提醒

成功启动后会发出运行中提醒。

![](https://cdn.jsdelivr.net/gh/skykeyjoker/A-Soul-Notification@master/screenshots/start.png)



### 运行出错提醒

运行中出现错误会发出相应提醒。

![](https://cdn.jsdelivr.net/gh/skykeyjoker/A-Soul-Notification@master/screenshots/error.png)



### 插件更新提醒

提醒插件有新版本并引导前往下载。

![](https://cdn.jsdelivr.net/gh/skykeyjoker/A-Soul-Notification@master/screenshots/update.png)



### 成员动态提醒

推送成员动态（动态，专栏，视频）。

![](https://cdn.jsdelivr.net/gh/skykeyjoker/A-Soul-Notification@master/screenshots/dy1.png)

![](https://cdn.jsdelivr.net/gh/skykeyjoker/A-Soul-Notification@master/screenshots/dy2.png)



### 成员直播提醒

推送成员直播消息。

![](https://cdn.jsdelivr.net/gh/skykeyjoker/A-Soul-Notification@master/screenshots/live.png)



### 自定义查询用户

可以通过自定义程序运行目录下的`member.json`文件设置要查询的用户。

`member.json`的格式如下：

```json
{
	"member": [
		{
			"uid": 672346917,
			"nickname": "向晚",
			"avatar": "ava.jpg"
		},
		...
		{
			"uid": 703007996,
			"nickname": "A-SOUL Official",
			"avatar": "official.jpg"
		}
	]
}
```

修改`member`字段对应的数组，成员的数据格式如下：

```json
{
    "uid": ,		// 用户UID
    "nickname": ,	// 可以自己设置一个昵称
    "avatar": 		// 设置头像文件
}
```

成员数据所有字段都不可为空。

用户头像可添加到程序运行目录下的`avatar`目录内。



## 工欲善其事，必先利其器

Json库：[nlohmann/json](https://github.com/nlohmann/json)

Wintoast库：[mohabouje/WinToast](https://github.com/mohabouje/WinToast)

Qt自带的JSON库，谁用谁后悔！抓到数据后解析JSON，发现解析结果一直是空的，而且还没有出错，没 有 出 错！DEBUG半天没找出来是啥问题，反复验证返回的数据是否完整，JSON格式是否正确，毫无疑问这俩都没问题。。。最后换了一个接口符合人类直觉、强大好用的JSON库（即nlohmann/json），立马OK。。。再说一遍，Qt自带的JSON库，谁用谁后悔。



## 友情链接

[VSCode插件：A-SOUL 提醒小助手](https://github.com/luooooob/vscode-asoul-notifications)

[VSCode插件：A-SOUL鼓励师](https://github.com/as042971/vscode-asoul)

[Typora主题：Typora-theme-jiaran](https://github.com/q19980722/Typora-theme-jiaran)

