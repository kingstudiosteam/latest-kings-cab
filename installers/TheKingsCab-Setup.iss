[Setup]
AppId={{A7DB3D4F-9F5C-4C9D-8F2B-1F7A6E29B9B1}}
AppName=The Kings Cab
AppVersion=1.0.0
AppPublisher=King Studios
DefaultDirName={pf}\King Studios\The Kings Cab
DisableDirPage=yes
DisableProgramGroupPage=yes
OutputDir=installers
OutputBaseFilename=TheKingsCab-Windows-Setup
Compression=lzma
SolidCompression=yes
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
WizardStyle=modern

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: desktopicon; Description: "Create a &Desktop shortcut for Standalone"; GroupDescription: "Additional icons:"

[Dirs]
Name: "{commoncf}\VST3"; Flags: uninsneveruninstall
Name: "{commoncf}\Avid\Audio\Plug-Ins"; Flags: uninsneveruninstall
Name: "{commonappdata}\King Studios\The Kings Cab\IR Collections"; Flags: uninsalwaysuninstall

[Files]
; Use paths relative to script: {#SourcePath} points to installers\
Source: "{#SourcePath}..\build\TheKingsCab_artefacts\Release\VST3\The Kings Cab.vst3\*"; DestDir: "{commoncf}\VST3\The Kings Cab.vst3"; Flags: recursesubdirs ignoreversion
#ifexist "{#SourcePath}..\build\TheKingsCab_artefacts\Release\AAX\The Kings Cab.aaxplugin\"
Source: "{#SourcePath}..\build\TheKingsCab_artefacts\Release\AAX\The Kings Cab.aaxplugin\*"; DestDir: "{commoncf}\Avid\Audio\Plug-Ins\The Kings Cab.aaxplugin"; Flags: recursesubdirs ignoreversion
#endif
#ifexist "{#SourcePath}..\build\TheKingsCab_artefacts\Release\Standalone\The Kings Cab.exe"
Source: "{#SourcePath}..\build\TheKingsCab_artefacts\Release\Standalone\The Kings Cab.exe"; DestDir: "{app}"; Flags: ignoreversion
#endif
Source: "{#SourcePath}..\assets\kkheader.png"; DestDir: "{app}\assets"; Flags: ignoreversion
Source: "{#SourcePath}..\assets\kkmain.png"; DestDir: "{app}\assets"; Flags: ignoreversion
Source: "{#SourcePath}..\assets\kkfooter.png"; DestDir: "{app}\assets"; Flags: ignoreversion
Source: "{#SourcePath}..\assets\custom_background.png"; DestDir: "{app}\assets"; Flags: ignoreversion
; Also copy assets to ProgramData for DAWs
Source: "{#SourcePath}..\assets\kkheader.png"; DestDir: "{commonappdata}\King Studios\The Kings Cab\assets"; Flags: ignoreversion
Source: "{#SourcePath}..\assets\kkmain.png"; DestDir: "{commonappdata}\King Studios\The Kings Cab\assets"; Flags: ignoreversion
Source: "{#SourcePath}..\assets\kkfooter.png"; DestDir: "{commonappdata}\King Studios\The Kings Cab\assets"; Flags: ignoreversion
Source: "{#SourcePath}..\assets\custom_background.png"; DestDir: "{commonappdata}\King Studios\The Kings Cab\assets"; Flags: ignoreversion
Source: "{#SourcePath}windows\ir_collections\*"; DestDir: "{commonappdata}\King Studios\The Kings Cab\IR Collections"; Flags: recursesubdirs ignoreversion

[Icons]
Name: "{autoprograms}\King Studios\The Kings Cab (Standalone)"; Filename: "{app}\The Kings Cab.exe"
Name: "{autoprograms}\King Studios\King Studios Store"; Filename: "https://www.kingstudiospa.com/store"
Name: "{userdesktop}\The Kings Cab (Standalone)"; Filename: "{app}\The Kings Cab.exe"; Tasks: desktopicon

[UninstallDelete]
Type: filesandordirs; Name: "{commonappdata}\King Studios\The Kings Cab"

[Registry]
Root: HKLM; Subkey: "Software\Microsoft\Windows\CurrentVersion\Uninstall\The Kings Cab"; ValueType: string; ValueName: "DisplayName"; ValueData: "The Kings Cab"; Flags: uninsdeletekey
Root: HKLM; Subkey: "Software\Microsoft\Windows\CurrentVersion\Uninstall\The Kings Cab"; ValueType: string; ValueName: "Publisher"; ValueData: "King Studios"
Root: HKLM; Subkey: "Software\Microsoft\Windows\CurrentVersion\Uninstall\The Kings Cab"; ValueType: string; ValueName: "DisplayVersion"; ValueData: "1.0.0"
