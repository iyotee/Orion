@echo off
REM ORION OS Wiki Deployment Script for Windows
REM This script deploys the complete Wiki to GitHub

echo ORION OS Wiki Deployment Script
echo =================================
echo.

REM Configuration
set WIKI_REPO=https://github.com/iyotee/Orion.wiki.git
set WIKI_DIR=orion-wiki
set SOURCE_DIR=.

echo [INFO] Checking prerequisites...

REM Check if git is installed
git --version >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Git is not installed. Please install Git first.
    pause
    exit /b 1
)

REM Check if we're in the right directory
if not exist "Home.md" (
    echo [ERROR] Wiki files not found. Please run this script from the Wiki source directory.
    pause
    exit /b 1
)

echo [SUCCESS] Prerequisites check passed
echo.

echo [INFO] Setting up Wiki repository...

REM Check if Wiki repository exists
git ls-remote "%WIKI_REPO%" >nul 2>&1
if errorlevel 1 (
    echo [WARNING] Wiki repository not found at %WIKI_REPO%
    echo [INFO] You need to create the Wiki repository first:
    echo [INFO] 1. Go to https://github.com/iyotee/Orion
    echo [INFO] 2. Click on "Wiki" tab
    echo [INFO] 3. Click "Create the first page"
    echo [INFO] 4. This will create the Wiki repository
    echo [INFO] 5. Then run this script again
    echo.
    pause
    exit /b 1
)

if exist "%WIKI_DIR%" (
    echo [INFO] Wiki directory exists, updating...
    cd "%WIKI_DIR%"
    git pull origin main
    cd ..
) else (
    echo [INFO] Cloning Wiki repository...
    git clone "%WIKI_REPO%" "%WIKI_DIR%"
)

echo [SUCCESS] Wiki repository setup complete
echo.

echo [INFO] Copying Wiki files...

REM List of Wiki files to copy
set WIKI_FILES=Home.md Installation-Guide.md Quick-Start.md User-Guide.md Development-Guide.md Contributing-Guidelines.md Architecture-Overview.md Wiki-README.md _wiki.yml Wiki-Deployment-Guide.md Wiki-Summary.md

REM Copy each file
for %%f in (%WIKI_FILES%) do (
    if exist "%SOURCE_DIR%\%%f" (
        copy "%SOURCE_DIR%\%%f" "%WIKI_DIR%\" >nul
        echo [INFO] Copied %%f
    ) else (
        echo [WARNING] File %%f not found, skipping...
    )
)

echo [SUCCESS] Wiki files copied successfully
echo.

echo [INFO] Deploying Wiki to GitHub...

cd "%WIKI_DIR%"

REM Check if there are changes
git diff --quiet
if errorlevel 1 (
    REM Add all files
    git add .

    REM Commit changes
    git commit -m "Complete ORION OS Wiki documentation - Added 12 comprehensive documentation pages with professional academic tone"

    REM Push to GitHub
    echo [INFO] Pushing to GitHub...
    git push origin master
) else (
    echo [WARNING] No changes detected, Wiki is already up to date
)

cd ..

echo [SUCCESS] Wiki deployed successfully to GitHub!
echo.

echo Next steps:
echo   1. Visit: https://github.com/iyotee/Orion/wiki
echo   2. Verify all pages are visible and working
echo   3. Test navigation and internal links
echo   4. Share with the ORION OS community
echo.

echo Thank you for contributing to ORION OS!
echo.
pause
