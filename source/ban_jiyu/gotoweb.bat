@echo off
echo ===================================
echo jnuo_tool          解除U盘和网络限制                 jnuo_tool
echo 正在尝试解除极域网络和U盘限制...
echo.

REM 1. 停止并禁用网络过滤服务
sc stop TDNetFilter >nul 2>&1
sc config TDNetFilter start= disabled >nul 2>&1

REM 2. 结束网络控制进程
taskkill /f /im MasterHelper.exe >nul 2>&1

REM 3. 停止并禁用U盘文件过滤服务
sc stop TDFileFilter >nul 2>&1
sc config TDFileFilter start= disabled >nul 2>&1

REM 4. 结束文件控制进程
taskkill /f /im ProcHelper64.exe >nul 2>&1

echo 基础限制解除操作完成。
echo.
pause
