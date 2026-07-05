; Script de Inno Setup para crear un instalador ejecutable de TIM Remastered
; Para compilarlo, instala Inno Setup (https://jrsoftware.org/isdl.php) y abre este archivo.

#define MyAppName "The Incredible Machine Remastered"
#define MyAppVersion "1.0"
#define MyAppPublisher "Federico"
#define MyAppExeName "TIM_Grafica.exe"

[Setup]
; AppId identifica de forma unica la aplicacion (puedes generar uno nuevo en Tools -> Generate GUID)
AppId={{8B1D4F1C-D01F-4340-9E1D-3FC69B7083D4}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={userpf}\{#MyAppName}
DisableProgramGroupPage=yes
; Icono que tendra el instalador en si
SetupIconFile=..\icon.ico
OutputDir=..\
OutputBaseFilename=TIM_Remastered_Instalador
Compression=lzma
SolidCompression=yes
WizardStyle=modern

[Languages]
Name: "spanish"; MessagesFile: "compiler:Languages\Spanish.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "..\TIM_Grafica.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\icon.ico"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Assets\*"; DestDir: "{app}\Assets"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\fonts\*"; DestDir: "{app}\fonts"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; IconFilename: "{app}\icon.ico"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon; IconFilename: "{app}\icon.ico"

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
