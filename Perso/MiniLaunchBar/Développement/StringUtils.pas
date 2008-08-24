unit StringUtils;

interface

uses Classes, SysUtils, Variants;

function IsInStringList(const iStringList: TStringList; const iString: String):Boolean;
function StringConv(const v:Variant):String;
function SplitString(const Delimiter: Char; Input: string): TStringList;

implementation


function SplitString(const Delimiter: Char; Input: string): TStringList;
begin
	result := TStringList.Create();
  result.Delimiter := Delimiter;
	result.DelimitedText := Input;
end;


function IsInStringList(const iStringList: TStringList; const iString: String):Boolean;
var i: LongWord;
begin
	result := false;
	for i := 0 to iStringList.Count - 1 do begin
  	if iStringList[i] = iString then begin
    	result := true;
      break;
    end;
  end;
end;


function StringConv(const v:Variant):String;
var t: Word;
begin
	t := VarType(v);

  if (t = varNull) or (t = varEmpty) then begin
  	result := 'nil';
    Exit;
  end;

	if t = varBoolean then begin
  	if Boolean(v) then
    	result := 'true'
    else
    	result := 'false';
    Exit;
  end;

  if t = varInteger then begin
  	result := IntToStr(Integer(v));
    Exit;
  end;

  if t = varWord then begin
  	result := IntToStr(Word(v));
    Exit;
  end;

  if t = varSingle then begin
  	result := IntToStr(Integer(v));
    Exit;
  end;

  if t = varByte then begin
  	result := IntToStr(Byte(v));
    Exit;
  end;

  if t = varString then begin
  	result := String(v);
    Exit;
  end;

  result := '<unknown type>';
end;


end.
