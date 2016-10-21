# 网易云信 DuiLib 使用说明

### DuiLib介绍

Duilib是windows平台下的一款轻量级directUI开源库（遵循BSD协议），完全免费，可用于商业软件开发，只需在软件包里附上协议文件即可。Duilib可以简单方便地实现大多数界面需求，包括换肤、换色，透明等功能，支持多种图片格式，使用XML可以方便地定制窗口，能较好地做到UI和逻辑相分离，尽量减少在代码里创建UI控件。目前，Duilib库已经渐趋稳定，目前在国内有较为广泛的使用，网络上也有很多的使用教程。

### 云信DuiLib介绍  
虽有原版Duilib简单易用，但是存在一些bug和不足，如控件种类不丰富、不支持动画、不支持半透明异形窗体、对多线程支持不好等，因此云信使用了专门增强扩展的`云信Duilib`。`云信Duilib`有一下优点：

* 优化了Duilib渲染效率
* 增强了界面渲染效果
* 增强了XML布局的功能，更方便的实现更多的布局效果
* 增强了控件功能属性
* 控件支持动画效果
* 控件支持gif图片格式
* 支持半透明异形窗体
* 支持多语言功能

 使用`云信Duilib`，配合比较高效的引擎库`base库`解决多线程问题，可以做出功能更强更稳定的客户端界面。`云信Duilib`以静态库的形式提供。

## 云信Duilib使用帮助

* `云信Duilib`与原版的Duilib已经有了较大的不同，但是如果已经熟练使用原版Duilib的开发人员可以很快的学会`云信Duilib`，所以建议您先学习一下原版Duilib的使用方法。  
* `云信Duilib`与原版Duilib最大的不同在于控件和控件属性的增删以及XML布局的配置方法。我们提供了`云信Duilib`的控件属性列表[云信Duilib属性列表](./duilib属性列表.xml)，搭配云信Demo来理解`云信Duilib`的使用方法[NIM Demo For PC](https://github.com/netease-im/NIM_PC_Demo)。  
* 关于`云信Duilib`界面布局的教程：[云信Duilib布局指南](./nim_duilib_layout.md)  
* `云信Duilib`依赖基础引擎库`base库`，所以在使用`云信Duilib`的项目中必须要需要同时添加`base库`，`base库`在UI组件中的`tool_kits\base`目录提供。
* `云信Duilib`的所有类都位于`ui`命名空间下。

### 云信Duilib初始化

在创建并使用窗体功能之前，需要对`云信Duilib`进行初始化。一般在`wWinMain`入口函数中进行初始化。`GlobalManager`类管理了程序所有的所用公共界面资源，初始化函数`Startup`需要传入两个参数：第一个参数表示所有资源的根目录；第二个参数表示创建自定义控件的回调函数，一般不需要指定。示例代码如下：

```
std::wstring theme_dir = QPath::GetAppPath();
ui::GlobalManager::Startup(theme_dir + L"themes\\default", ui::CreateControlCallback());
```

* 初始化函数会根据指定的目录，自动搜寻目录根位置中的`global.xml`文件，`global.xml`文件中定义了全局公用资源,包含了字体、class、文字颜色。`global.xml`文件是可选的，但是无疑几乎所有程序都会使用字体和文字颜色等信息。
* 初始化函数同时搜索多语言文件，多语言文件应该位于相对于程序的`\lang\zh_CN\gdstrings.ini`位置，名字必须为`gdstrings.ini`,多语言文件是可选的。
* `global.xml`文件和`gdstrings.ini`文件具体的编写和使用请参考[云信Duilib属性列表](./duilib属性列表.xml)和云信Demo[NIM Demo For PC](https://github.com/netease-im/NIM_PC_Demo)。
* 当消息循环结束，在程序结束之前，应该调用反初始化函数来做清理工作，示例代码如下：

```
ui::GlobalManager::Shutdown();
```

### 开发窗体类

窗体类是程序UI的基本，`云信Duilib`已经提供了编写好的窗体基类`WindowImplBase`，在创建您自己的窗体类时，只需要继承`WindowImplBase`类或者其子类即可帮助您完成大部分的窗体类开发工作，您只需要重写几个必要的方法即可。一个基本的窗体类框架如下：

```
class MyForm : public ui::WindowImplBase
{
public:
	MyForm();
	~MyForm();

	virtual std::wstring GetSkinFolder() override;
	virtual std::wstring GetSkinFile() override;
	virtual std::wstring GetWindowClassName() const override;

	virtual void InitWindow() override;

	virtual UINT GetClassStyle() const override;
	virtual ui::Control* CreateControl(const std::wstring& pstrClass) override;
}
```

* `GetSkinFolder`方法**（必须重写的方法）**用于返回本窗体类所使用的资源的目录，这个目录相对于`云信Duilib`初始化时指定的目录。一般把本窗体使用的图片素材和XML布局文件都放到此目录。示例如下：

```
std::wstring MyForm::GetSkinFolder()
{
	return L"my_form";
}
```

* `GetSkinFile`方法**（必须重写的方法）**用于返回本窗体类所加载的XML布局文件，这个文件应该存放在`GetSkinFolder`方法所指定的目录中。示例如下：

```
std::wstring MyForm::GetSkinFile()
{
	return L"my_form.xml";
}
```

* `GetWindowClassName`方法**（必须重写的方法）**用于返回本窗体类的类名，不用的窗体类应该具有不同的类名，示例如下：

```
std::wstring MyForm::GetWindowClassName() const
{
	return L"NIM_LoginForm";
}
```
* `InitWindow`方法**（可选的方法）**用于初始化窗体，当窗体被创建后，会自动调用`InitWindow`方法，我们可以在这里对窗体进行初始化（比如初始化各个控件指针，为控件添加消息处理函数）,示例如下：

```
void MyForm::InitWindow()
{
	m_pRoot->AttachBubbledEvent(ui::kEventAll, nbase::Bind(&MyForm::Notify, this, std::placeholders::_1));

	btn_login = static_cast<ui::Button*>(FindControl(L"login_button"));
}
```

* `GetClassStyle`方法**（可选的方法）**用于设置本窗体类的风格，一般不用重写。示例如下：

```
UINT MyForm::GetClassStyle() const
{
	return CS_DBLCLKS;
}
```

* `CreateControl`方法**（可选的方法）**用于返回自定义控件，当库提供的基础控件不满足需求是，就需要开发自定义控件，当解析XML布局文件时遇到位置的自定义控件时就会触发此函数并传入自定义控件在XML中的名字，让您返回您的自定义控件。示例如下：

```
Control* MyForm::CreateControl(const std::wstring& pstrClass)
{
	if (pstrClass == L"MyControl")
	{
		return new MyControl;
	}
	return NULL;
}
```

#### UI组件项目的窗体类

在UI组件项目里，提供了窗体基类`nim_comp::WindowEx`类，`nim_comp::WindowEx`类继承自`WindowImplBase`。使用UI组件时，建议使用`nim_comp::WindowEx`类，其方法与`WindowImplBase`的方法相同，但额外增加了一个`GetWindowId`方法需要实现。方法声明为：

```
virtual std::wstring GetWindowId() const override;
```

此方法的作用是用来标识并区别窗体，与`GetWindowClassName`方法一样同样有区别作用。`GetWindowId`方法与`GetWindowClassName`方法配合就可以唯一的区别一个窗体。比如当前打开了N个会话窗体，N个会话窗体的`GetWindowClassName`方法返回的是用一个类名，这时就需要通过`GetWindowId`方法区别每个会话窗体，这时可以在`GetWindowId`方法中返回会话ID，这时就可以唯一的区别一个窗体了。

#### 控件的事件响应

`云信Duilib`中没有窗体类的函数可以用来直接收取到所有控件的事件，每个控件都可以单独设置自己的事件处理函数，一般在`InitWindow`方法中初始化各个控件的事件处理函数。

* 每个控件都有许多形如`Attach···`的方法，比如按钮控件`Button`有`AttachMouseEnter`、`AttachButtonDown`、`AttachClick`方法，他们分别用于指定控件鼠标进入、鼠标按下、鼠标单击的事件处理函数。`Attach···`的方法的参数需要传入使用`base库`的`Bind`函数处理过的函数，`Bind`函数是对c++11中`Bind`函数的进一步优化，如果您不了解`Bind`函数，建议可以学习C++11的相关资料。处理函数的声明格式为：

```
bool Notify(ui::EventArgs* msg);
```

`EventArgs`结构体中包含了触发事件的控件的指针、鼠标坐标、按键状态、时间戳等信息。函数的返回值，返回true表示继续传递控件消息，返回false表示停止传递控件消息。

一个完整的控件事件绑定和处理的代码如下：

```
void MyForm::InitWindow()
{
	ui::Button* btn_login = static_cast<ui::Button*>(FindControl(L"login_button"));
	btn_login->AttachClick(nbase::Bind(&MyForm::OnLoginClicked, this, std::placeholders::_1, true));
}

bool MyForm::OnLoginClicked(ui::EventArgs * msg)
{
	std::wstring name = msg->pSender->GetName();

	if (msg->Type == ui::kEventClick)
	{

		if (name == L"login_button")
		{
			DoLogin();
		}
	}
	return true;
}
```

* 对每个控件都设置一个事件处理函数是很麻烦的，很多时候我们想在一个函数里处理某个容器控件内的所有控件的所有事件，这时可以使用`云信Duilib`的容器控件特有的`AttachBubbledEvent`方法，`AttachBubbledEvent`方法的第一个参数表示所关心的事件类型（ui::kEventAll表示所有事件）,第二个参数标识事件处理函数。`AttachBubbledEvent`方法会把本容器及其所有子控件的事件都交给所绑定的事件处理函数。

很多时候，可以直接给`m_pRoot`控件调用`AttachBubbledEvent`方法，`m_pRoot`代表本窗体类的根容器，绑定它的`ui::kEventAll`事件就代表了处理窗体类的所有控件的所有事件。

示例代码如下：

```
void MyForm::InitWindow()
{
	m_pRoot->AttachBubbledEvent(ui::kEventAll, nbase::Bind(&MyForm::Notify, this, std::placeholders::_1));
}

bool MyForm::Notify(ui::EventArgs * msg)
{
	std::wstring name = msg->pSender->GetName();

	if (msg->Type == ui::kEventClick)
	{

		if (name == L"login_button")
		{
			DoLogin();;
		}
	}
	else if (msg->Type == ui::kEventTab)
	{
		//your codes
	}
	return true;
}

```