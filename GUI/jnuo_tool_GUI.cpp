#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>
#include <time.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <algorithm>

#pragma comment(lib, "comctl32.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

using namespace std;

// 全局变量
HWND hMainWnd;
HFONT hFont;
int fc_using[8] = {0};
bool program_running = true;
bool flag2 = false, flag5 = false, flag6 = false;

// 控制ID定义
#define ID_BUTTON_EXIT 100
#define ID_BUTTON_KILL_JIYU 101
#define ID_BUTTON_DEEPSEEK 102
#define ID_BUTTON_HIDE_ZIP 103
#define ID_BUTTON_COMPARE 104
#define ID_BUTTON_MIHOYO 105
#define ID_BUTTON_ANTISPY 106
#define ID_BUTTON_UNLOCK 107
#define ID_BUTTON_LOG 108
#define ID_BUTTON_UPDATE 109
#define ID_STATIC_STATUS 110

// 日志函数
void log_program_start() {
    time_t now = time(NULL);
    tm* local_time = localtime(&now);
    
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%m-%d %H:%M", local_time);
    
    FILE* log_file = fopen("app.log", "a");
    if (!log_file) {
        log_file = fopen("app.log", "w");
        if (!log_file) return;
    }
    
    fprintf(log_file, "==========start========== [%s]\n", time_str);
    fclose(log_file);
}

void log_program_end() {
    time_t now = time(NULL);
    tm* local_time = localtime(&now);
    
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%m-%d %H:%M", local_time);
    
    FILE* log_file = fopen("app.log", "a");
    if (!log_file) {
        log_file = fopen("app.log", "a");
        if (!log_file) return;
    }
    
    fprintf(log_file, "==========end============ [%s]\n\n", time_str);
    fclose(log_file);
}

void log_function(int function_id, bool running) {
    time_t now = time(NULL);
    tm* local_time = localtime(&now);
    
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%m-%d %H:%M", local_time);
    
    FILE* log_file = fopen("app.log", "a");
    if (!log_file) {
        log_file = fopen("app.log", "w");
        if (!log_file) return;
    }
    
    fprintf(log_file, "[%s] used:%d, running:%d\n", time_str, function_id, running ? 1 : 0);
    fclose(log_file);
}

void RunBatInSourceFolderSimple() {
    const char* relativePath = "source\\ban_jiyu\\useU.bat";
    
    SHELLEXECUTEINFO sei = { sizeof(sei) };
    sei.lpVerb = "runas";
    sei.lpFile = relativePath;
    sei.nShow = SW_SHOW;
    
    if (!ShellExecuteEx(&sei)) {
        DWORD err = GetLastError();
        if (err == ERROR_CANCELLED) {
            MessageBoxA(hMainWnd, "用户取消了操作", "提示", MB_ICONINFORMATION);
        } else {
            char msg[100];
            sprintf(msg, "执行失败，错误代码: %d", err);
            MessageBoxA(hMainWnd, msg, "错误", MB_ICONERROR);
        }
    }
}

void update_status() {
    string status = "状态: ";
    
    if (flag2) status += "deepseek运行中  ";
    if (flag5) status += "miHoYo运行中  ";
    if (flag6) status += "防窥屏运行中  ";
    
    if (!flag2 && !flag5 && !flag6) {
        status += "所有功能未运行";
    }
    
    SetWindowTextA(GetDlgItem(hMainWnd, ID_STATIC_STATUS), status.c_str());
}

void show_message(const string& msg, const string& title = "提示") {
    MessageBoxA(hMainWnd, msg.c_str(), title.c_str(), MB_OK | MB_ICONINFORMATION);
}

// 读取日志的详细版本
void read_log_detailed() {
    FILE* log_file = fopen("app.log", "r");
    if (!log_file) {
        show_message("没有找到日志文件", "错误");
        return;
    }
    
    string msg = "正在读取上次使用记录并恢复功能...\n";
    
    // 先找到文件末尾，从后往前读取
    fseek(log_file, 0, SEEK_END);
    long file_size = ftell(log_file);
    
    // 从文件末尾开始，查找最近的end标记
    long pos = file_size - 1;
    char buffer[1024];
    bool found_end = false;
    long end_pos = -1;
    long start_pos = -1;
    
    // 从后往前查找end标记
    while (pos >= 0 && pos >= file_size - 5000) {
        fseek(log_file, pos, SEEK_SET);
        fgets(buffer, sizeof(buffer), log_file);
        
        if (strstr(buffer, "==========end============")) {
            found_end = true;
            end_pos = pos;
            break;
        }
        pos--;
    }
    
    if (!found_end) {
        fclose(log_file);
        show_message("未找到完整的上次使用记录", "提示");
        return;
    }
    
    // 从end标记往前找对应的start标记
    pos = end_pos - 1;
    bool found_start = false;
    
    while (pos >= 0) {
        fseek(log_file, pos, SEEK_SET);
        fgets(buffer, sizeof(buffer), log_file);
        
        if (strstr(buffer, "==========start==========")) {
            found_start = true;
            start_pos = pos;
            break;
        }
        pos--;
    }
    
    if (!found_start) {
        fclose(log_file);
        show_message("未找到对应的开始记录", "提示");
        return;
    }
    
    // 读取整个区块的内容
    fseek(log_file, start_pos, SEEK_SET);
    long block_size = end_pos - start_pos + 100;
    char* block_content = (char*)malloc(block_size + 1);
    fread(block_content, 1, block_size, log_file);
    block_content[block_size] = '\0';
    
    fclose(log_file);
    
    // 解析区块内容
    msg += "\n找到上次使用记录：\n";
    int last_function_status[8] = {0};
    
    char* line = strtok(block_content, "\n");
    while (line) {
        // 跳过标记行
        if (strstr(line, "==========start==========") || 
            strstr(line, "==========end============")) {
            line = strtok(NULL, "\n");
            continue;
        }
        
        int function_id, running_status;
        if (sscanf(line, "[%*[^]]] used:%d, running:%d", &function_id, &running_status) == 2) {
            if (function_id >= 1 && function_id <= 7) {
                last_function_status[function_id] = running_status + 1;
                char temp[100];
                sprintf(temp, "  功能%d: %s\n", function_id, running_status ? "运行中" : "已停止");
                msg += temp;
            }
        }
        
        line = strtok(NULL, "\n");
    }
    
    // 恢复功能
    msg += "\n正在恢复功能...\n";
    int recovered_count = 0;
    
    for (int i = 1; i <= 7; i++) {
        if (last_function_status[i] == 2) {
            char temp[100];
            sprintf(temp, "恢复功能 %d: ", i);
            msg += temp;
            
            if (i == 2) {
                msg += "deepseek\n";
                system("start https://chat.deepseek.com");
                system(".\\source\\Web\\deekseep\\deekseep.html");
                fc_using[2] = 1;
                flag2 = true;
                recovered_count++;
            } else if (i == 5) {
                msg += "miHoYo\n";
                fc_using[5] = 1;
                flag5 = true;
            } else if (i == 6) {
                msg += "防窥屏\n";
                system(".\\source\\ScreenWings.exe");
                fc_using[6] = 1;
                flag6 = true;
                recovered_count++;
            } else {
                msg += "功能代码待实现\n";
            }
            
            Sleep(500);
        }
    }
    
    if (last_function_status[5] == 2) {
        msg += "\n检测到上次使用了miHoYo功能，请在主界面重新选择游戏\n";
    }
    
    char temp[100];
    sprintf(temp, "\n已恢复 %d 个功能", recovered_count);
    msg += temp;
    
    free(block_content);
    show_message(msg, "日志恢复");
    update_status();
}

void execute_command(int cmd_id) {
    switch(cmd_id) {
        case ID_BUTTON_KILL_JIYU: {
            int result = MessageBoxA(hMainWnd, 
                "确定要杀死极域吗？\n杀死后需要自行寻找快捷方式\njnuo对你的行为概不负责", 
                "确认", 
                MB_YESNO | MB_ICONWARNING);
            
            if (result == IDYES) {
                system("taskkill /f /t /im studentmain.exe");
                show_message("已尝试结束极域进程");
            }
            break;
        }
        
        case ID_BUTTON_DEEPSEEK:
            if (!flag2) {
                flag2 = true;
                system("start https://chat.deepseek.com");
                system(".\\source\\Web\\deekseep\\deekseep.html");
                fc_using[2] = 1;
                log_function(2, true);
                update_status();
                show_message("deepseek 已启动");
            } else {
                int result = MessageBoxA(hMainWnd, 
                    "此功能正在运行中，是否已经关闭？", 
                    "确认", 
                    MB_YESNO | MB_ICONQUESTION);
                
                if (result == IDYES) {
                    flag2 = false;
                    fc_using[2] = 2;
                    log_function(2, false);
                    update_status();
                }
            }
            break;
            
        case ID_BUTTON_HIDE_ZIP: {
            int result = MessageBoxA(hMainWnd, 
                "请确保：\n1. 图片文件已命名为'1.png'\n2. zip文件已命名为'2.zip'\n3. 文件与程序在同一目录\n\n是否继续？", 
                "提示", 
                MB_YESNO | MB_ICONINFORMATION);
            
            if (result == IDYES) {
                system("copy /b 1.png + 2.zip end.png");
                show_message("操作完成！请查看目录中的end.png文件");
            }
            break;
        }
            
        case ID_BUTTON_COMPARE: {
            int result = MessageBoxA(hMainWnd, 
                "请确保正确输出文件已命名为'1.ans'\n程序输出文件已命名为'2.out'\n文件与程序在同一目录\n\n是否继续？", 
                "提示", 
                MB_YESNO | MB_ICONINFORMATION);
            
            if (result == IDYES) {
                system("fc 1.ans 2.out");
                show_message("对比完成，请查看命令行窗口结果");
            }
            break;
        }
            
        case ID_BUTTON_MIHOYO:
            if (!flag5) {
                flag5 = true;
                fc_using[5] += 1;
                log_function(5, true);
                
                int choice = MessageBoxA(hMainWnd, 
                    "请选择游戏：\n\n选择'是' - 原神\n选择'否' - 崩坏·星穹铁道\n选择'取消' - 放弃", 
                    "miHoYo游戏选择", 
                    MB_YESNOCANCEL | MB_ICONQUESTION);
                
                if (choice == IDYES) {
                    system("start https://ys.mihoyo.com/cloud/#");
                    system("start https://ys-api.mihoyo.com/event/download_porter/link/ys_cn/official/pc_backup316");
                } else if (choice == IDNO) {
                    system("start https://sr.mihoyo.com/cloud/#");
                    system("start https://autopatchcn.bhsr.com/client/cn/20251126183400_yvLQxEpk9CTuJjg6/gw_PC/StarRail_setup_1.12.0.exe");
                } else {
                    flag5 = false;
                    fc_using[5] = 0;
                    log_function(5, false);
                }
                update_status();
            } else {
                int result = MessageBoxA(hMainWnd, 
                    "此功能正在运行中，是否已经关闭？", 
                    "确认", 
                    MB_YESNO | MB_ICONQUESTION);
                
                if (result == IDYES) {
                    flag5 = false;
                    fc_using[5] = 2;
                    log_function(5, false);
                    update_status();
                }
            }
            break;
            
        case ID_BUTTON_ANTISPY:
            if (!flag6) {
                flag6 = true;
                system(".\\source\\ScreenWings.exe");
                fc_using[6] += 1;
                log_function(6, true);
                
                int result = MessageBoxA(hMainWnd, 
                    "防窥屏已启动！\n\n使用说明：\n1. 点击小电脑图标\n2. 窗口变黑且右上角变为'-'表示成功\n3. 可以最小化到托盘\n4. 注意：截图也会黑屏！\n\n是否查看详细教程？", 
                    "防窥屏", 
                    MB_YESNO | MB_ICONINFORMATION);
                
                if (result == IDYES) {
                    show_message("教程：\n1. 运行后会出现小电脑图标\n2. 点击它，屏幕会变黑\n3. 右上角'X'变成'-'表示成功\n4. 可以拖到系统托盘隐藏\n5. 使用此功能时截图也会是黑屏");
                }
                update_status();
            } else {
                int result = MessageBoxA(hMainWnd, 
                    "此功能正在运行中，是否已经关闭？", 
                    "确认", 
                    MB_YESNO | MB_ICONQUESTION);
                
                if (result == IDYES) {
                    flag6 = false;
                    fc_using[6] = 2;
                    log_function(6, false);
                    update_status();
                }
            }
            break;
            
        case ID_BUTTON_UNLOCK:
            RunBatInSourceFolderSimple();
            break;
            
        case ID_BUTTON_LOG:
            read_log_detailed();
            break;
            
        case ID_BUTTON_UPDATE:
            show_message("jnuo_tool 2.0.0 更新内容：\n\n"
                        "1. 更新了日志，保存了上次的使用情况（自动打开）\n"
                        "2. 优化了输入字符的部分\n"
                        "3. 加入了新选项", 
                        "更新日志");
            break;
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE: {
            // 创建按钮
            int y = 60;
            int btn_height = 35;
            int btn_width = 150;
            int spacing = 45;
            
            // 设置字体
            hFont = CreateFontA(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                              DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                              DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "微软雅黑");
            
            CreateWindowA("BUTTON", "1. 杀死极域", 
                        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                        20, y, btn_width, btn_height, hWnd, (HMENU)ID_BUTTON_KILL_JIYU, NULL, NULL);
            
            CreateWindowA("BUTTON", "2. deepseek", 
                        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                        20, y + spacing, btn_width, btn_height, hWnd, (HMENU)ID_BUTTON_DEEPSEEK, NULL, NULL);
            
            CreateWindowA("BUTTON", "3. 隐藏zip到图片", 
                        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                        20, y + spacing * 2, btn_width, btn_height, hWnd, (HMENU)ID_BUTTON_HIDE_ZIP, NULL, NULL);
            
            CreateWindowA("BUTTON", "4. 对拍", 
                        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                        20, y + spacing * 3, btn_width, btn_height, hWnd, (HMENU)ID_BUTTON_COMPARE, NULL, NULL);
            
            CreateWindowA("BUTTON", "5. miHoYo", 
                        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                        200, y, btn_width, btn_height, hWnd, (HMENU)ID_BUTTON_MIHOYO, NULL, NULL);
            
            CreateWindowA("BUTTON", "6. 防窥屏", 
                        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                        200, y + spacing, btn_width, btn_height, hWnd, (HMENU)ID_BUTTON_ANTISPY, NULL, NULL);
            
            CreateWindowA("BUTTON", "7. 解除限制", 
                        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                        200, y + spacing * 2, btn_width, btn_height, hWnd, (HMENU)ID_BUTTON_UNLOCK, NULL, NULL);
            
            CreateWindowA("BUTTON", "查看使用记录", 
                        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                        200, y + spacing * 3, btn_width, btn_height, hWnd, (HMENU)ID_BUTTON_LOG, NULL, NULL);
            
            CreateWindowA("BUTTON", "这次更新", 
                        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                        20, y + spacing * 4, btn_width, btn_height, hWnd, (HMENU)ID_BUTTON_UPDATE, NULL, NULL);
            
            CreateWindowA("BUTTON", "退出程序", 
                        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                        200, y + spacing * 4, btn_width, btn_height, hWnd, (HMENU)ID_BUTTON_EXIT, NULL, NULL);
            
            // 状态显示
            CreateWindowA("STATIC", "状态: 等待操作...", 
                        WS_CHILD | WS_VISIBLE,
                        20, 20, 400, 30, hWnd, (HMENU)ID_STATIC_STATUS, NULL, NULL);
            
            // 标题
            CreateWindowA("STATIC", "jnuo_tool 2.0.0", 
                        WS_CHILD | WS_VISIBLE | SS_CENTER,
                        150, 280, 200, 30, hWnd, NULL, NULL, NULL);
            
            // 设置所有控件字体
            HWND hChild = GetWindow(hWnd, GW_CHILD);
            while (hChild) {
                SendMessage(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);
                hChild = GetWindow(hChild, GW_HWNDNEXT);
            }
            
            update_status();
            break;
        }
        
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            if (wmId == ID_BUTTON_EXIT) {
                DestroyWindow(hWnd);
            } else {
                execute_command(wmId);
            }
            break;
        }
        
        case WM_DESTROY:
            if (program_running) {
                program_running = false;
                log_program_end();
            }
            DeleteObject(hFont);
            PostQuitMessage(0);
            break;
            
        default:
            return DefWindowProcA(hWnd, message, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // 初始化通用控件
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icex);
    
    // 注册窗口类
    WNDCLASSEXA wcex;
    wcex.cbSize = sizeof(WNDCLASSEXA);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "JnuoToolWindowClass";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    
    if (!RegisterClassExA(&wcex)) {
        MessageBoxA(NULL, "窗口类注册失败!", "错误", MB_ICONERROR);
        return 1;
    }
    
    // 创建启动窗口
    HWND hSplashWnd = CreateWindowExA(0, "STATIC", "jnuo制造...", 
                                    WS_POPUP | WS_VISIBLE | SS_CENTER,
                                    CW_USEDEFAULT, CW_USEDEFAULT, 400, 200, 
                                    NULL, NULL, hInstance, NULL);
    
    if (hSplashWnd) {
        HFONT hSplashFont = CreateFontA(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                      DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                      DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "微软雅黑");
        SendMessage(hSplashWnd, WM_SETFONT, (WPARAM)hSplashFont, TRUE);
        
        // 居中显示
        RECT rc;
        GetWindowRect(hSplashWnd, &rc);
        int x = (GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2;
        int y = (GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 2;
        SetWindowPos(hSplashWnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        
        // 显示启动动画
        ShowWindow(hSplashWnd, nCmdShow);
        UpdateWindow(hSplashWnd);
        
        // 模拟原始启动动画
        for(int i = 0; i < 2; i++) {
            SetWindowTextA(hSplashWnd, "jnuo制造.");
            Sleep(1000);
            SetWindowTextA(hSplashWnd, "jnuo制造..");
            Sleep(1000);
            SetWindowTextA(hSplashWnd, "jnuo制造...");
            Sleep(1000);
        }
        
        DestroyWindow(hSplashWnd);
        DeleteObject(hSplashFont);
    }
    
    // 记录程序开始
    log_program_start();
    
    // 创建主窗口
    hMainWnd = CreateWindowExA(0, "JnuoToolWindowClass", "jnuo_tool 2.0.0",
                             WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                             CW_USEDEFAULT, CW_USEDEFAULT, 400, 400,
                             NULL, NULL, hInstance, NULL);
    
    if (!hMainWnd) {
        MessageBoxA(NULL, "窗口创建失败!", "错误", MB_ICONERROR);
        return 1;
    }
    
    // 居中显示主窗口
    RECT rc;
    GetWindowRect(hMainWnd, &rc);
    int x = (GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 2;
    SetWindowPos(hMainWnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    
    // 显示窗口
    ShowWindow(hMainWnd, nCmdShow);
    UpdateWindow(hMainWnd);
    
    // 消息循环
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return (int)msg.wParam;
}
