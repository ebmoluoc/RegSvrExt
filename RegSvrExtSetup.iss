[Setup]
AppId={{DEAF1CB3-D4E5-4937-843E-53DFFA04A4A3}
SetupMutex=Global\DEAF1CB3-D4E5-4937-843E-53DFFA04A4A3
AppCopyright=Copyright (c) 2019 Philippe Coulombe
AppPublisher=Philippe Coulombe
AppVersion=1.0.0.0
VersionInfoVersion=1.0.0.0
AppVerName=RegSvrExt 1.0
AppName=RegSvrExt
DefaultDirName={commonpf}\RegSvrExt
OutputBaseFilename=RegSvrExtSetup
OutputDir=.
LicenseFile=LICENSE
DisableProgramGroupPage=yes
DisableDirPage=yes
SolidCompression=yes
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
MinVersion=6.1.7601
WizardSizePercent=120,100

[Files]
Source: "LICENSE"; DestDir: {app}; Flags: restartreplace uninsrestartdelete ignoreversion
Source: "x64\Release\RegSvrExt.dll"; DestDir: {app}; Flags: restartreplace uninsrestartdelete ignoreversion regserver

[Code]
procedure InitializeWizard();
begin
    WizardForm.LicenseMemo.Font.Name := 'Consolas';
end;
