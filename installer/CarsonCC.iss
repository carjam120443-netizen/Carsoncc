#define MyAppName "CarsonCC"
#define MyAppVersion GetEnv("CARSONCC_VERSION")
#if MyAppVersion == ""
  #define MyAppVersion "0.1.0"
#endif
#define MyAppPublisher "CarsonCC Project"
#define MyAppExeName "carsoncc.exe"

[Setup]
AppId={{B7D5E2C8-4D38-4E4A-9A7A-1C7D3C9F4E21}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={localappdata}\Programs\CarsonCC
DefaultGroupName=CarsonCC
DisableProgramGroupPage=yes
PrivilegesRequired=lowest
OutputDir=dist
OutputBaseFilename=CarsonCC-{#MyAppVersion}-Setup
Compression=lzma
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64compatible
WizardStyle=modern
UninstallDisplayName=CarsonCC

[Files]
Source: "carsoncc.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "README.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "examples\*"; DestDir: "{app}\examples"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{autoprograms}\CarsonCC\CarsonCC Documentation"; Filename: "{app}\README.md"
Name: "{autoprograms}\CarsonCC\Examples"; Filename: "{app}\examples"

[Code]
const
  EnvironmentKey = 'Software\Environment';
  EnvironmentValue = 'Path';

procedure BroadcastEnvironmentChange;
var
  ResultCode: Integer;
begin
  Exec(ExpandConstant('{cmd}'), '/C setx CARSONCC_PATH_REFRESH 1 >nul 2>&1', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
end;

function NormalizePathPart(const S: string): string;
begin
  Result := S;
  while (Length(Result) > 0) and ((Result[1] = ' ') or (Result[1] = '"')) do
    Delete(Result, 1, 1);
  while (Length(Result) > 0) and ((Result[Length(Result)] = ' ') or (Result[Length(Result)] = '"')) do
    Delete(Result, Length(Result), 1);
  Result := RemoveBackslashUnlessRoot(Result);
end;

procedure AddCarsonCCToPath;
var
  OldPath, NewPath, AppPath: string;
begin
  AppPath := ExpandConstant('{app}');
  if RegQueryStringValue(HKEY_CURRENT_USER, EnvironmentKey, EnvironmentValue, OldPath) then
  begin
    if Pos(';' + AppPath + ';', ';' + OldPath + ';') > 0 then
      exit;
    if (OldPath <> '') and (OldPath[Length(OldPath)] <> ';') then
      NewPath := OldPath + ';' + AppPath
    else
      NewPath := OldPath + AppPath;
  end
  else
    NewPath := AppPath;

  RegWriteExpandStringValue(HKEY_CURRENT_USER, EnvironmentKey, EnvironmentValue, NewPath);
  BroadcastEnvironmentChange;
end;

procedure RemoveCarsonCCFromPath;
var
  OldPath, NewPath, AppPath: string;
  P: Integer;
begin
  AppPath := NormalizePathPart(ExpandConstant('{app}'));
  if not RegQueryStringValue(HKEY_CURRENT_USER, EnvironmentKey, EnvironmentValue, OldPath) then
    exit;

  NewPath := '';
  while OldPath <> '' do
  begin
    P := Pos(';', OldPath);
    if P = 0 then
    begin
      if CompareText(NormalizePathPart(OldPath), AppPath) <> 0 then
      begin
        if NewPath <> '' then NewPath := NewPath + ';';
        NewPath := NewPath + OldPath;
      end;
      OldPath := '';
    end
    else
    begin
      if CompareText(NormalizePathPart(Copy(OldPath, 1, P - 1)), AppPath) <> 0 then
      begin
        if NewPath <> '' then NewPath := NewPath + ';';
        NewPath := NewPath + Copy(OldPath, 1, P - 1);
      end;
      Delete(OldPath, 1, P);
    end;
  end;

  RegWriteExpandStringValue(HKEY_CURRENT_USER, EnvironmentKey, EnvironmentValue, NewPath);
  BroadcastEnvironmentChange;
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssPostInstall then
    AddCarsonCCToPath;
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  if CurUninstallStep = usUninstall then
    RemoveCarsonCCFromPath;
end;
