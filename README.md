<h1 align="center">A-SOUL Notification</h1>

<p align="center">
    <a href="https://github.com/skykeyjoker/A-SOUL-Notification/stargazers" style="text-decoration:none">
        <img src="https://img.shields.io/github/stars/skykeyjoker/A-SOUL-Notification.svg" alt="GitHub stars"/>
    </a>
    <a href="https://github.com/skykeyjoker/A-SOUL-Notification/network" style="text-decoration:none" >
        <img src="https://img.shields.io/github/forks/skykeyjoker/A-SOUL-Notification.svg" alt="GitHub forks"/>
    </a>
    <a href="https://github.com/skykeyjoker/A-SOUL-Notification/releases" style="text-decoration:none" >
        <img src="https://img.shields.io/github/downloads/skykeyjoker/A-SOUL-Notification/latest/total.svg" alt="GitHub release download"/>
    </a>
    <a href="https://github.com/skykeyjoker/A-SOUL-Notification/blob/master/LICENSE" style="text-decoration:none" >
        <img src="https://img.shields.io/badge/License-MIT-flat.svg" alt="GitHub license"/>
    </a>
</p>

一个A-SOUL成员动态/直播提醒插件，支持监控B站与抖音。仅能在Windows10-Windows11系统上运行。

* 注：现在已经实现自定义用户查询，可以编辑`member.json`文件设置想要查询的用户。

使用Qt5编写，核心实现为Request请求+Json库解析+Wintoast消息。

为应对B站接口限制，目前查询为30秒/次，因此消息推送可能会有较短延迟（最大延迟1min左右）。

注：本插件为控制台程序（Console Application），无界面无托盘。可以通过查询`ASoulNotification.exe`进程检查插件运行状态。

注：叔叔有风控系统，有时同一局域网请求过多，即使是30s一次查询也会因为服务器过载保护被掐。插件出错6次自动退出后，可以自行前往`logs`文件夹内的日志文件验证是否为这种情况（一般错误提示是远程关闭了访问）。此时可以**等待半小时左右**重新开启插件。

[功能介绍](#功能介绍)|[使用教程](#使用教程)|[下载地址](#下载地址)|[友情链接](#友情链接)

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

推送成员的动态（动态，专栏，视频）。

默认开启抖音动态监控，可在设置文件`member.json`中手动关闭。

![](https://cdn.jsdelivr.net/gh/skykeyjoker/A-Soul-Notification@master/screenshots/dy1.png)

![](https://cdn.jsdelivr.net/gh/skykeyjoker/A-Soul-Notification@master/screenshots/dy2.png)

![](https://cdn.jsdelivr.net/gh/skykeyjoker/A-Soul-Notification@master/screenshots/Douyin.jpg)



### 成员直播提醒

推送成员直播消息。

![](https://cdn.jsdelivr.net/gh/skykeyjoker/A-Soul-Notification@master/screenshots/live.png)



### 自定义查询用户

可以通过自定义程序运行目录下的`member.json`文件设置要查询的用户。

`member.json`的格式如下：

```json
{
  "Bilibili": {
    "member": [
      {
        "uid": 672346917,
        "nickname": "向晚",
        "avatar": "ava.jpg"
      },
		......
      {
        "uid": 703007996,
        "nickname": "A-SOUL Official",
        "avatar": "official.jpg"
      }
    ]
  },
  "Douyin": {
    "enable": true,
    "member": [
      {
        "uid": "ASOULofficial",
        "sec_uid": "MS4wLjABAAAAflgvVQ5O1K4RfgUu3k0A2erAZSK7RsdiqPAvxcObn93x2vk4SKk1eUb6l_D4MX-n",
        "nickname": "五个魂儿呀",
        "avatar": "ASOULofficial.jpg"
      }
    ]
  }
}
```

修改`member`字段对应的数组，B站成员的数据格式如下：

```json
{
    "uid": ,		// 用户UID
    "nickname": ,	// 可以自己设置一个昵称
    "avatar": 		// 设置头像文件
}
```

抖音成员的数据格式如下：

```json
{
	"uid": ,  		// 抖音用户uid
	"sec_uid": ,  	// 抖音用户sec_uid
	"nickname": ,  	// 可以自己设置一个昵称
	"avatar":   	// 设置头像文件
}
```

成员数据所有字段都不可为空。

用户头像可添加到程序运行目录下的`avatar`目录内。

若要关闭抖音监控功能，可以将`Douyin`字段中的`enable`设置为`false`：

```json
{
    ......
	"Douyin":{
		"enable": false,
	    ......
	}
}
```



## 使用教程

1. 下载本项目压缩包并解压，[下载地址](#下载地址)。

2. 运行目录内的ASoulNotification.exe可执行文件，即可运行插件。

3. 本插件为控制台程序，无界面无托盘图标。可以使用任务管理器（Ctrl+Alt+Del快捷键调出）查看ASoulNotification.exe进程检查插件运行状态。

   ![](https://cdn.jsdelivr.net/gh/skykeyjoker/A-Soul-Notification@master/screenshots/task.jpg)

4. 可以将本程序添加到系统自启动列表中，让本程序随系统启动。添加自启动教程：http://www.xitongtang.com/class/win11/27800.html

5. 可编辑目录下member.json文件自定义要监控的B站和抖音账号或关闭抖音监控，member.json格式介绍见上文。

6. 可以在插件目录下的logs文件夹内log.txt查看插件的本次运行记录（每次启动插件都会清空上次插件的运行记录）。



## 下载地址

本项目提供两种下载方式：Github Release（更新最快）和蓝奏云同步节点（更新可能有延迟）

[Github Release](https://github.com/skykeyjoker/A-SOUL-Notification/releases)：更新最快。

[蓝奏云](https://www.lanzouw.com/b02uir28d)：密码：9r25 国内备份节点，国内下载体验更好一些，更新可能有延迟。



## 工欲善其事，必先利其器

Json库：[nlohmann/json](https://github.com/nlohmann/json)

Wintoast库：[mohabouje/WinToast](https://github.com/mohabouje/WinToast)

Qt自带的JSON库，谁用谁后悔！抓到数据后解析JSON，发现解析结果一直是空的，而且还没有出错，没 有 出 错！DEBUG半天没找出来是啥问题，反复验证返回的数据是否完整，JSON格式是否正确，毫无疑问这俩都没问题。。。最后换了一个接口符合人类直觉、强大好用的JSON库（即nlohmann/json），立马OK。。。再说一遍，Qt自带的JSON库，谁用谁后悔。



## 友情链接

[VSCode插件：A-SOUL 提醒小助手](https://github.com/luooooob/vscode-asoul-notifications)

[VSCode插件：A-SOUL鼓励师](https://github.com/as042971/vscode-asoul)

[Typora主题：Typora-theme-jiaran](https://github.com/q19980722/Typora-theme-jiaran)

[IDEA插件：A-SOUL鼓励师](https://github.com/cnsky1103/A-SOUL-Reminder)
