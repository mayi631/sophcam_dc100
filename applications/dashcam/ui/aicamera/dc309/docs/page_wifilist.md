
## wifi 扫描流程图

```mermaid
flowchart TD
       A[WiFi列表页面加载<br/>sysMenu_WifiList] --> B{检测WiFi状态<br/>Hal_Wifi_Is_Up}
       B -->|已开启| C[显示扫描中提示]
       B -->|未开启| D[隐藏列表, 显示开关]
       C --> E[启动WiFi扫描<br/>Hal_Wpa_Scan]
       D --> F[用户点击WiFi开关<br/>wifi_switch_event_cb]

       E --> G[UI显示正在扫描...<br/>创建200ms定时器<br/>wifi_scan_check_status]
       F --> H[WiFi开启]
       H --> C

       G --> I[定时器触发]
       I --> J[先尝试获取扫描结果<br/>Hal_Wpa_GetScanResultAsync]
       J -->|成功| M[UI显示WiFi列表<br/>wifi_list_rebuild]
       J -->|失败| K{扫描超时?>10s}
       K -->|否| I
       K -->|是| L[显示扫描失败]
       L --> N[UI显示空列表<br/>删除定时器]

       M --> P[删除定时器<br/>wifi_scan_timer_stop]

       N --> Q[用户操作]
       P --> Q

       Q --> R[点击刷新按钮<br/>wifi_refresh_btn_cb]
       R --> E

       Q --> S[点击WiFi项<br/>wifi_list_item_clicked_cb]
       S --> T{需要密码?<br/>is_wifi_password_protected}
       T -->|是| U[跳转到密码输入页]
       T -->|否| V[显示连接中...]
       V --> W{连接成功?}
       W -->|是| X[显示连接成功<br/>1秒后自动刷新]
       W -->|否| Y[显示连接失败<br/>1秒后自动刷新]

       X --> Z[隐藏提示<br/>hide_label_notice]
       Y --> Z
       Z --> G

       Q --> AA[物理按键/手势返回]
       AA --> BB[停止定时器<br/>wifi_scan_timer_stop]
       BB --> CC[返回上级页面]

       Q --> DD[上下键选择WiFi<br/>select_prev_wifi/select_next_wifi]
       DD --> EE[选中高亮<br/>set_selected_style]

       Q --> FF[OK键确认<br/>confirm_selected_wifi]
       FF --> GG{已连接?}
       GG -->|是| HH[跳过]
       GG -->|否| II{已保存?}
       II -->|是| JJ[直接连接<br/>Hal_Wpa_Connect]
       II -->|否| KK{需要密码?}
       KK -->|是| U
       KK -->|否| JJ

       JJ --> LL{连接成功?}
       LL -->|是| MM[显示连接成功]
       LL -->|否| NN[显示连接失败]

       MM --> OO[1秒后刷新]
       NN --> OO
       OO --> G

       style A fill:#e3f2fd
       style G fill:#fff9c4
       style I fill:#fff9c4
       style J fill:#c8e6c9
       style M fill:#c8e6c4
       style N fill:#ffcdd2
       style U fill:#e1bee7
       style V fill:#fff9c4
       style X fill:#c8e6c9
       style Y fill:#ffcdd2
       style EE fill:#bbdefb
       style JJ fill:#fff9c4
```

## WiFi列表操作流程图

```mermaid
flowchart TD
       A[WiFi列表已加载] --> B{用户操作类型}
       B --> C[点击刷新按钮<br/>wifi_refresh_btn_cb]
       B --> D[点击WiFi项<br/>wifi_list_item_clicked_cb]
       B --> E[按上键<br/>KEY_UP]
       B --> F[按下手<br/>KEY_DOWN]
       B --> G[按OK键<br/>KEY_OK]
       B --> H[返回按钮/手势]

       C --> I[显示扫描中...<br/>Hal_Wpa_Scan]
       I --> J[创建200ms定时器]
       J --> K[进入扫描等待流程]

       D --> L{WiFi已连接?}
       L -->|是| M[跳过]
       L -->|否| N{WiFi已保存?}
       N -->|是| O[直接连接<br/>Hal_Wpa_Connect]
       N -->|否| P{需要密码?}
       P -->|是| Q[跳转密码输入页]
       P -->|否| O

       E --> R[选中上一个<br/>select_prev_wifi]
       F --> S[选中下一个<br/>select_next_wifi]

       G --> T{当前选中WiFi}
       T --> U{已连接?}
       U -->|是| V[跳过]
       U -->|否| W{已保存?}
       W -->|是| X[直接连接]
       W -->|否| Y{需要密码?}
       Y -->|是| Z[跳转密码输入页]
       Y -->|否| X

       H --> AA[停止定时器<br/>wifi_scan_timer_stop]
       AA --> BB[返回]

       style C fill:#e3f2fd
       style D fill:#fff9c4
       style E fill:#bbdefb
       style F fill:#bbdefb
       style G fill:#bbdefb
       style H fill:#ffcdd2
```

## 关键函数说明

### 🏁 初始化函数
- `sysMenu_WifiList()`: 页面入口函数，创建UI并启动扫描

### 🔍 扫描相关
- `Hal_Wpa_Scan()`: 启动异步扫描（hal_wifi层）
- `wifi_scan_check_status()`: 定时器回调，先尝试获取扫描结果，打破循环依赖
- `Hal_Wpa_GetScanState()`: 获取扫描状态（hal_wifi层）
- `Hal_Wpa_GetScanResultAsync()`: 获取扫描结果（hal_wifi层），包含早期检测机制
- `wifi_list_rebuild()`: 重建WiFi列表UI（清空+重新创建所有项）

### 🎮 事件回调
- `wifi_switch_event_cb()`: WiFi开关事件回调（打开/关闭WiFi）
- `wifi_refresh_btn_cb()`: 刷新按钮事件（手动触发扫描）
- `wifi_list_item_clicked_cb()`: WiFi列表项点击事件
- `wifi_back_cb()`: 返回按钮事件
- `sysmenu_wifilist_key_handler()`: 物理按键处理（KEY_UP/KEY_DOWN/KEY_OK/KEY_MENU）
- `gesture_event_handler()`: 手势事件处理（向右滑动返回）
- `confirm_selected_wifi()`: OK键确认选择WiFi

### 🔗 连接相关
- `Hal_Wpa_Connect()`: 连接WiFi（hal_wifi层）
- `Hal_Wpa_DisableAllNetworks()`: 禁用所有网络，禁止自动连接
- `is_wifi_password_protected()`: 判断WiFi是否需要密码
- `hide_label_notice()`: 隐藏提示并触发刷新

### 🎨 UI样式相关
- `set_selected_style()`: 设置选中样式（蓝色边框）
- `clear_selected_style()`: 清除选中样式
- `select_next_wifi()`: 选中下一个WiFi项
- `select_prev_wifi()`: 选中上一个WiFi项

### ⏱️ 定时器管理
- `wifi_scan_timer_stop()`: 停止扫描定时器
- `wifi_scan_check_status()`: 扫描状态检查定时器回调

## 📝 修改记录

### 2025-12-24：修复wifi自动连接与手动连接冲突；

**wifi自动连接与手动连接冲突**

复现方法：已保存某个wifi密码，在打开wifi的时候，手动点击连接改wifi，由于 wpa_supplicant 会自动连接该wifi，hal_wifi这边等不到连接成功的消息，就判断为超时了

解决办法：每次打开 wlan up 前，先disable all network，禁止自动连接，这样也能节省一点功耗(没有连接wifi的时候），也可解决该问题。

### 2025-12-24: 更新文档，补充物理按键和OK键确认流程

**内容更新**:
1. 更新主流程图，添加函数名称标识
2. 新增「WiFi列表操作流程图」，展示用户操作的完整流程
3. 补充物理按键处理流程（KEY_UP/KEY_DOWN/KEY_OK/KEY_MENU）
4. 补充OK键确认连接逻辑（已保存密码的WiFi直接连接）
5. 完善关键函数说明，添加UI样式相关函数说明

**相关文件**:
- `docs/page_wifilist.md`: 更新流程图和函数说明
- `src/guiguider_ui/page_sysmenu_wifilist.c`: 同步代码逻辑

---

### 2025-12-16: 修复WiFi扫描超时问题

**问题**: WiFi扫描总是超时，无法获取结果

**根本原因**: 循环依赖
- UI逻辑：先检查状态 → 如果不是COMPLETE → 不调用GetScanResultAsync
- HAL逻辑：状态只有在GetScanResultAsync成功时才会更新为COMPLETE
- 结果：状态一直是IN_PROGRESS，永不更新

**解决方案**:
1. 修改 `wifi_scan_check_status()` 逻辑：直接先调用 `Hal_Wpa_GetScanResultAsync()`
2. 只有在获取失败时才检查状态决定下一步
3. 增加扫描超时时间：3秒 → 10秒（hal层）/ 15秒（同步接口）
4. 增强早期检测机制：在GetScanResultAsync中主动检测扫描是否完成
5. 添加详细调试日志

**相关文件**:
- `src/guiguider_ui/page_sysmenu_wifilist.c`: 修改wifi_scan_check_status()逻辑
- `src/hal_wifi/hal_wifi_ctrl.c`: 增加超时时间、调试日志、早期检测机制
