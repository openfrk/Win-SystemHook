param (
    [string]$ImagePath,
    [string]$TxtPath
)

if (Test-Path $ImagePath) {
    $bytes = [System.IO.File]::ReadAllBytes($ImagePath)
    $base64 = [System.Convert]::ToBase64String($bytes)
    $htmlCode = "<img src='data:image/png;base64,$base64' alt='icon' />"
    
    Add-Content -Path $TxtPath -Value $htmlCode
    Write-Host "[+] Base64 HTML code successfully appended to: $TxtPath"
} else {
    Write-Host "[-] Image file not found: $ImagePath"
}
