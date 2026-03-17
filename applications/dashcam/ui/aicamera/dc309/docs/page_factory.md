## 恢复出厂设置页面流程图

```mermaid
flowchart TD
       A[恢复出厂设置页面加载<br/>sysMenu_Factory] --> B[创建主页面容器]
       B --> C[创建顶部栏+返回按钮]
       C --> D[创建标题:恢复出厂设置]
       D --> E[创建设置选项容器]

       E --> F[创建两个选项按钮]
       F --> G[选项1: 确定<br/>index=0]
       F --> H[选项2: 取消<br/>index=1]

       G --> I[初始化焦点组<br/>init_focus_group]
       H --> I

       I --> J[设置焦点在第一个按钮<br/>lv_group_focus_obj]

       J --> K[等待用户操作]

       K --> L{用户操作}
       L --> M[点击确定按钮<br/>sysMenu_Factory_Select_btn_event_handler]
       L --> N[点击取消按钮]
       L --> O[按OK键确认]
       L --> P[按MENU键返回]
       L --> Q[手势向右滑动]

       M --> R[调用sysMenu_Factory_Sure<br/>显示确认对话框]

       N --> S[删除当前页面<br/>sysMenu_Factory_Delete_anim]
       S --> T[返回设置页面<br/>sysMenu_Setting]

       O --> U{当前选中哪个?}
       U -->|确定选项| R
       U -->|取消选项| S

       P --> S
       Q --> S

       R --> V[显示确认对话框<br/>sysMenu_Factory_Sure]
       V --> W[创建半透明浮层]
       W --> X[创建对话框容器]
       X --> Y[显示警告图标+标题+说明文本]
       Y --> Z[创建取消按钮+确定按钮]

       Z --> AA[设置按键回调<br/>factory_key_cb]

       AA --> AB[等待用户操作]

       AB --> AC{用户操作}
       AC --> AD[点击取消按钮<br/>sysMenu_Factory_OK_Cancel_btn_event_handler]
       AC --> AE[点击确定按钮]
       AC --> AF[按LEFT键选中取消]
       AC --> AG[按RIGHT键选中确定]
       AC --> AH[按OK键确认]
       AC --> AI[按MENU键返回]

       AD --> AJ[隐藏对话框<br/>factory_delete_float_win]
       AE --> AK[执行恢复出厂设置]

       AF --> AL[选中取消按钮<br/>设置白色边框]
       AG --> AM[选中确定按钮]

       AK --> AN{恢复结果?}
       AN -->|成功| AO[显示恢复成功<br/>str_language_factory_reset_successful]
       AN -->|失败| AP[显示恢复失败<br/>str_language_factory_reset_failed]

       AO --> AQ[1秒后自动隐藏<br/>factory_display_tips_anim_complete]
       AP --> AQ

       AQ --> AR[隐藏对话框<br/>factory_delete_float_win]

       AI --> AJ
       AH --> AS{当前选中?}
       AS -->|取消| AJ
       AS -->|确定| AK

       AJ --> T

       AR --> T

       style A fill:#e3f2fd
       style J fill:#fff9c4
       style R fill:#c8e6c9
       style S fill:#ffcdd2
       style V fill:#e1bee7
       style AK fill:#fff9c4
       style AO fill:#c8e6c9
       style AP fill:#ffcdd2
```

## 页面状态流程图

```mermaid
stateDiagram-v2
    [*] --> 主页面加载: sysMenu_Factory()

    主页面加载 --> 等待用户操作: 初始化焦点组

    等待用户操作 --> 显示确认对话框: 用户点击"确定"按钮
    等待用户操作 --> 返回设置页面: 用户点击"取消"按钮/返回/MENU键/手势

    显示确认对话框 --> 等待对话框操作: 设置factory_key_cb回调

    等待对话框操作 --> 执行恢复出厂设置: 用户点击"确定"按钮
    等待对话框操作 --> 返回设置页面: 用户点击"取消"按钮/MENU键

    执行恢复出厂设置 --> 显示操作结果: MODEMNG_SendMessage返回

    显示操作结果 --> 1秒后自动返回: 动画完成

    1秒后自动返回 --> 返回设置页面: factory_delete_float_win()

    返回设置页面 --> [*]
```

## 关键函数说明

### 🏁 页面入口函数
- `sysMenu_Factory(lv_ui_t* ui)`: 页面初始化函数，创建完整UI
  - 创建主页面容器（640x480）
  - 创建顶部栏和返回按钮
  - 创建"确定"和"取消"两个选项按钮
  - 初始化焦点组，支持物理按键导航

### ✅ 确认对话框相关
- `sysMenu_Factory_Sure(void)`: 显示恢复出厂设置确认对话框
  - 创建半透明浮层（70%透明度）
  - 创建居中对话框容器（500x300）
  - 显示警告图标和说明文本
  - 创建取消/确定按钮，设置`factory_key_cb`为按键处理器

### 🎮 事件回调函数
- `sysMenu_Factory_Select_btn_event_handler(lv_event_t* e)`: 主页面选项按钮点击事件
  - index=0: 显示确认对话框
  - index=1: 删除页面并返回设置页
- `sysMenu_Factory_OK_Cancel_btn_event_handler(lv_event_t* e)`: 对话框按钮点击事件
  - 取消: 调用`factory_delete_float_win()`隐藏对话框
  - 确定: 发送`EVENT_MODEMNG_SETTING`消息执行恢复出厂设置
- `sysmenu_factory_click_callback(lv_obj_t* obj)`: 焦点组点击回调（触摸/OK键）
- `factory_key_cb(int key_code, int key_value)`: 物理按键处理
  - KEY_MENU: 隐藏对话框
  - KEY_LEFT: 选中取消按钮
  - KEY_RIGHT: 选中确定按钮
  - KEY_OK: 确认当前选择

### 🔄 动画与清理函数
- `sysMenu_Factory_Delete_anim(void)`: 页面删除动画（透明度渐隐）
- `factory_delete_float_win(void)`: 删除浮层并恢复焦点组
- `sysMenu_factory_Delete_Complete_anim_cb(lv_anim_t* a)`: 页面删除完成回调，返回设置页
- `factory_display_tips_anim_complete(lv_anim_t* a)`: 结果提示动画完成，隐藏对话框

### 📋 语言字符串
- `str_language_factory_reset`: "恢复出厂设置"标题
- `str_language_factory_settings`: "恢复出厂设置"对话框标题
- `str_language_are_you_sure_to_reset_to_factory_settings`: 确认提示文本
- `str_language_data_cannot_be_recovered_after_factory_reset`: 数据不可恢复警告
- `str_language_factory_reset_successful`: 恢复成功提示
- `str_language_factory_reset_failed`: 恢复失败提示

## 全局变量

| 变量名 | 类型 | 说明 |
|--------|------|------|
| `obj_sysMenu_Factory_s` | lv_obj_t* | 主页面底层窗口 |
| `obj_sysMenu_Factory_Float_s` | lv_obj_t* | 确认对话框浮层 |
| `obj_factory_dialog_s` | lv_obj_t* | 对话框内容容器 |
| `focusable_objects[]` | lv_obj_t[] | 焦点组可聚焦对象数组 |

## 按键映射

| 按键 | 主页面 | 对话框 |
|------|--------|--------|
| KEY_UP/KEY_DOWN | 切换选项 | 切换按钮选中 |
| KEY_OK | 确认当前选项 | 确认当前按钮 |
| KEY_LEFT | - | 选中取消 |
| KEY_RIGHT | - | 选中确定 |
| KEY_MENU | 返回 | 隐藏对话框 |
| 手势右滑 | 返回 | - |

## 修改记录

### 2026-01-04: 添加页面文档

**内容更新**:
1. 添加主流程图，展示页面加载和用户操作流程
2. 添加页面状态流程图，展示状态转换
3. 补充关键函数说明
4. 补充按键映射表
5. 补充全局变量说明

**相关文件**:
- `docs/page_factory.md`: 新建流程图和函数说明文档
- `src/guiguider_ui/page_sysmenu_factory.c`: 同步代码逻辑
