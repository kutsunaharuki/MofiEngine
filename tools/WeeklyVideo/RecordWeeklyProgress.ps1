# 先週の進捗動画を作るスクリプト
#   1. 先週(月曜0:00〜今週月曜0:00)のコミット一覧からタイトル画面(動画)を作る
#   2. Release版 Game.exe を起動して画面を録画する(キー入力でタイトル→インゲームへ進む)
#   3. タイトル画面 + 録画 をつなげて 1 本の mp4 にする
# タスクスケジューラから毎週月曜 10:30 に実行する(Register-WeeklyTask.ps1 で登録)
# ※ ビルドはしない(VSのビルドとぶつかるので)。最後にビルドされた Release の exe を使う

param(
    [string]$RepoDir   = "C:\Git\MofiEngine",
    [string]$GameExe   = "C:\Git\MofiEngine\Game\x64\Release\Game.exe",
    [string]$OutDir    = "$env:USERPROFILE\Videos\MofiProgress",
    [string]$Ffmpeg    = "$env:USERPROFILE\Tools\ffmpeg\ffmpeg.exe",
    [int]$RecordSeconds = 75,     # ゲーム画面を録画する秒数
    [int]$TitleSeconds  = 8       # コミット一覧を見せる秒数
)

$ErrorActionPreference = "Stop"

# ---------------- キー入力の台本 ----------------
# 録画開始からの流れ。Key: 押すキー / Hold: 押す秒数 / Wait: そのあと待つ秒数
#   J = Aボタン(決定)  W/A/S/D と 矢印キー = スティック
$InputScript = @(
    @{ Key = "None";  Hold = 0; Wait = 4 },   # タイトル画面を見せる
    @{ Key = "J";     Hold = 0.2; Wait = 8 }, # スタート → インゲームの読み込み
    @{ Key = "W";     Hold = 3; Wait = 1 },
    @{ Key = "Right"; Hold = 2; Wait = 1 },
    @{ Key = "D";     Hold = 2; Wait = 1 },
    @{ Key = "Left";  Hold = 2; Wait = 1 },
    @{ Key = "S";     Hold = 2; Wait = 1 }
)

# ---------------- 準備 ----------------
$today      = (Get-Date).Date
$sinceMon   = ([int]$today.DayOfWeek + 6) % 7          # 今週の月曜から何日たったか
$thisMonday = $today.AddDays(-$sinceMon)
$lastMonday = $thisMonday.AddDays(-7)
$lastSunday = $thisMonday.AddDays(-1)
$tag        = $lastMonday.ToString("yyyy-MM-dd")

New-Item -ItemType Directory -Force $OutDir | Out-Null
$work = Join-Path $env:TEMP "MofiWeeklyVideo_$tag"
New-Item -ItemType Directory -Force $work | Out-Null
$log  = Join-Path $OutDir "$tag`_log.txt"
function Log($msg) { $line = "[{0}] {1}" -f (Get-Date -Format "HH:mm:ss"), $msg; Add-Content -Path $log -Value $line -Encoding UTF8; Write-Host $line }

Log "=== 先週($tag 〜 $($lastSunday.ToString('yyyy-MM-dd')))の進捗動画を作ります ==="
if (-not (Test-Path $Ffmpeg))  { Log "ffmpeg が見つかりません: $Ffmpeg"; exit 1 }
if (-not (Test-Path $GameExe)) { Log "Game.exe が見つかりません: $GameExe"; exit 1 }

# ---------------- 1. コミット一覧 → タイトル動画 ----------------
$since = $lastMonday.ToString("yyyy-MM-dd 00:00")
$until = $thisMonday.ToString("yyyy-MM-dd 00:00")
$commitFile = Join-Path $work "commits_raw.txt"
# --output でファイルに直接書く(PowerShell のリダイレクトだと文字コードが崩れるため)
git -C $RepoDir -c i18n.logOutputEncoding=UTF-8 log --since="$since" --until="$until" --reverse --no-merges --date=format:"%m/%d" --pretty=format:"%ad  %s" --output="$commitFile"
$commits = @()
if (Test-Path $commitFile) { $commits = @(Get-Content $commitFile -Encoding UTF8 | Where-Object { $_.Trim() -ne "" }) }
Log "コミット数: $($commits.Count)"

$maxLines = 17
$lines = @("先週の進捗  $($lastMonday.ToString('yyyy/MM/dd')) - $($lastSunday.ToString('MM/dd'))", "")
if ($commits.Count -eq 0) { $lines += "(コミットなし)" }
else {
    $lines += $commits | Select-Object -First $maxLines
    if ($commits.Count -gt $maxLines) { $lines += "…ほか $($commits.Count - $maxLines) 件" }
}
# drawtext は UTF-8(BOMなし)のテキストを読む。改行は LF だけにする(CR も改行扱いされて行間が倍になるため)
[System.IO.File]::WriteAllText((Join-Path $work "title.txt"), ($lines -join "`n"), (New-Object System.Text.UTF8Encoding($false)))

# exe が先週のコミットより古いときは注意を出す(ビルドし忘れ)
$exeTime = (Get-Item $GameExe).LastWriteTime
$lastCommitTime = git -C $RepoDir log -1 --until="$until" --format="%cI"
if ($lastCommitTime -and $exeTime -lt [datetime]$lastCommitTime) {
    Log "注意: Release の Game.exe ($exeTime) が先週最後のコミットより古いです。Release ビルドし忘れの可能性あり"
}

Push-Location $work   # drawtext のパスをエスケープしなくて済むように、作業フォルダで実行する
try {
    $font = "C\:/Windows/Fonts/meiryo.ttc"
    $vf = "drawtext=fontfile='$font':textfile=title.txt:fontcolor=white:fontsize=28:line_spacing=6:x=120:y=60"
    & $Ffmpeg -hide_banner -loglevel error -y -f lavfi -i "color=c=0x1e1e28:s=1920x1080:r=30:d=$TitleSeconds" `
        -vf $vf -c:v libx264 -pix_fmt yuv420p title.mp4
    if ($LASTEXITCODE -ne 0) { Log "タイトル動画の作成に失敗"; exit 1 }
    Log "タイトル動画 OK"
}
finally { Pop-Location }

# ---------------- 2. ゲームを起動して録画 ----------------
Add-Type @"
using System;
using System.Runtime.InteropServices;
public static class W32 {
    [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
    [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr h, int cmd);
    [DllImport("user32.dll")] public static extern void keybd_event(byte vk, byte scan, uint flags, UIntPtr extra);
    [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L, T, R, B; }
    [StructLayout(LayoutKind.Sequential)] public struct POINT { public int X, Y; }
    [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr h, out RECT r);
    [DllImport("user32.dll")] public static extern bool ClientToScreen(IntPtr h, ref POINT p);
}
"@
[W32]::SetProcessDPIAware() | Out-Null

$vk = @{ J = 0x4A; W = 0x57; A = 0x41; S = 0x53; D = 0x44; Left = 0x25; Up = 0x26; Right = 0x27; Down = 0x28; Space = 0x20; Enter = 0x0D }
function Press($name, [double]$hold) {
    if ($name -eq "None") { return }
    $code = [byte]$vk[$name]
    [W32]::keybd_event($code, 0, 0, [UIntPtr]::Zero)          # 押す
    Start-Sleep -Milliseconds ([int]($hold * 1000))
    [W32]::keybd_event($code, 0, 2, [UIntPtr]::Zero)          # 離す(KEYEVENTF_KEYUP)
}

$gameDir = Split-Path (Split-Path (Split-Path $GameExe))    # ...\Game\x64\Release\Game.exe → ...\Game (Assets の場所)
Log "Game.exe を起動 (作業フォルダ: $gameDir)"
$game = Start-Process -FilePath $GameExe -WorkingDirectory $gameDir -PassThru

# ウィンドウが出るのを待つ
$deadline = (Get-Date).AddSeconds(60)
while ($game.MainWindowHandle -eq 0 -and -not $game.HasExited -and (Get-Date) -lt $deadline) { Start-Sleep -Milliseconds 500; $game.Refresh() }
if ($game.HasExited -or $game.MainWindowHandle -eq 0) { Log "ゲームのウィンドウが出ませんでした"; if (-not $game.HasExited) { $game.Kill() }; exit 1 }
Start-Sleep -Seconds 3   # 最初の読み込み待ち

# 最大化して前面に出す(Alt を一瞬押すと SetForegroundWindow が通りやすい)
[W32]::ShowWindow($game.MainWindowHandle, 3) | Out-Null
[W32]::keybd_event(0x12, 0, 0, [UIntPtr]::Zero); [W32]::keybd_event(0x12, 0, 2, [UIntPtr]::Zero)
[W32]::SetForegroundWindow($game.MainWindowHandle) | Out-Null
Start-Sleep -Seconds 1

# ゲームの描画部分(タイトルバーなし)の画面上の位置と大きさ。モニターが複数あってもゲームだけ撮る
$rc = New-Object W32+RECT; [W32]::GetClientRect($game.MainWindowHandle, [ref]$rc) | Out-Null
$pt = New-Object W32+POINT; [W32]::ClientToScreen($game.MainWindowHandle, [ref]$pt) | Out-Null
$capW = ($rc.R - $rc.L) -band -2; $capH = ($rc.B - $rc.T) -band -2   # x264 用に偶数にする
Log "録画範囲: x=$($pt.X) y=$($pt.Y) ${capW}x${capH}"

$rawVideo = Join-Path $work "game.mp4"
Log "録画開始 ($RecordSeconds 秒)"
$rec = Start-Process -FilePath $Ffmpeg -WindowStyle Hidden -PassThru -ArgumentList @(
    "-hide_banner", "-loglevel", "error", "-y",
    "-f", "gdigrab", "-framerate", "30", "-draw_mouse", "0",
    "-offset_x", "$($pt.X)", "-offset_y", "$($pt.Y)", "-video_size", "${capW}x${capH}", "-i", "desktop",
    "-t", "$RecordSeconds",
    "-vf", "scale=1920:1080:force_original_aspect_ratio=decrease,pad=1920:1080:(ow-iw)/2:(oh-ih)/2,fps=30",
    "-c:v", "libx264", "-preset", "veryfast", "-pix_fmt", "yuv420p", "`"$rawVideo`"")

foreach ($step in $InputScript) {
    if ($game.HasExited) { break }
    [W32]::SetForegroundWindow($game.MainWindowHandle) | Out-Null
    Press $step.Key $step.Hold
    Start-Sleep -Milliseconds ([int]($step.Wait * 1000))
}

$rec.WaitForExit()
Log "録画終了"
if (-not $game.HasExited) { $game.CloseMainWindow() | Out-Null; Start-Sleep -Seconds 3; if (-not $game.HasExited) { $game.Kill() } }
if (-not (Test-Path $rawVideo)) { Log "録画ファイルができていません"; exit 1 }

# ---------------- 3. つなげる ----------------
$outFile = Join-Path $OutDir "$tag`_進捗.mp4"
& $Ffmpeg -hide_banner -loglevel error -y -i (Join-Path $work "title.mp4") -i $rawVideo `
    -filter_complex "[0:v][1:v]concat=n=2:v=1:a=0[v]" -map "[v]" -c:v libx264 -pix_fmt yuv420p $outFile
if ($LASTEXITCODE -ne 0) { Log "結合に失敗"; exit 1 }

Log "完成: $outFile"
Remove-Item -Recurse -Force $work
