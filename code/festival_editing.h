/* date = December 2nd 2023 1:45 pm */

#ifndef FESTIVAL_EDITING_H
#define FESTIVAL_EDITING_H


void
MoveCursorPos(program_state *ProgramState, view *View, buffer_pos dPos)
{
    // TODO: set ideal cursor pos
    ProgramState->UserMovedCursor = true;
    View->CursorPos += dPos;
    Clamp(View->CursorPos.l, 0, LineCount(View));
    Clamp(View->CursorPos.c, 0, LineLength(View, View->CursorPos.l));
}


void
MoveBackNonWhitespace(program_state *ProgramState, view *View)
{
    b32 StartedAtSpace = false;
    if(CharAt(View, View->CursorPos) == ' ' || CharAt(View, View->CursorPos - BufferPos(0, 1)) == ' ')
        StartedAtSpace = true;
    
    if(StartedAtSpace)
    {
        do {
            View->CursorPos -= BufferPos(0, 1);
        }while(CharAt(View, View->CursorPos) == ' ' && View->CursorPos.c > 0);
    }
    
    while(CharAt(View, View->CursorPos) != ' ' && View->CursorPos.c > 0)
    {
        View->CursorPos -= BufferPos(0, 1);
    }
}

void
MoveForwardNonWhitespace(program_state *ProgramState, view *View)
{
    b32 StartedAtSpace = false;
    if(CharAt(View, View->CursorPos) == ' ' || CharAt(View, View->CursorPos + BufferPos(0, 1)) == ' ')
        StartedAtSpace = true;
    
    if(StartedAtSpace)
    {
        do {
            //View->CursorPos += BufferPos(0, 1);
            MoveCursorPos(ProgramState, View, BufferPos(0, 1));
        }while(CharAt(View, View->CursorPos) == ' ' && View->CursorPos.c < LineLength(View, View->CursorPos.l));
    }
    
    while(CharAt(View, View->CursorPos) != ' ' && View->CursorPos.c < LineLength(View, View->CursorPos.l))
    {
        //View->CursorPos += BufferPos(0, 1);
        MoveCursorPos(ProgramState, View, BufferPos(0, 1));
    }
}

buffer_pos
SeekBackBorder(view *View, buffer_pos From)
{
    buffer_pos Result = From;
    if(CharAt(View, Result) == 0)
        Result.c--;
    if(Result.c < 0 || (Result.c == 0 && Result.l == 0))
        return From;
    
    b32 StartedAtSpace = false;
    /*if(CharAt(View, Result) == ' ' ||*/ if(CharAt(View, Result + BufferPos(0, -1)) == ' ')
        StartedAtSpace = true;
    
    if(StartedAtSpace)
    {
        Print("Started at space");
        do {
            Result.c--;
        }while(CharAt(View, Result) == ' ' && Result.c < LineLength(View, Result.l));
        
        return Result;
    }
    
    b32 StartedAtSpecial = false;
    if(!IsNonSpecial(CharAt(View, Result)))
    {
        Print("Special");
        StartedAtSpecial = true;
    }
    
    if(StartedAtSpecial)
    {
        while(!IsNonSpecial(CharAt(View, Result)) && 
              Result.c < LineLength(View, Result.l))
            Result.c--;
        return Result;
    }
    
    char c = CharAt(View, Result);
    while(( c != ' ' && (IsNonSpecial(c)) )
          && Result.c < LineLength(View, Result.l))
    {
        Result.c--;
        c = CharAt(View, Result);
    }
    
    return Result;
}

buffer_pos
SeekForwardBorder(view *View, buffer_pos From)
{
    buffer_pos Result = From;
    
    b32 StartedAtSpace = false;
    if(CharAt(View, Result) == ' ' || CharAt(View, Result + BufferPos(0, 1)) == ' ')
        StartedAtSpace = true;
    
    if(StartedAtSpace)
    {
        do {
            Result.c++;
        }while(CharAt(View, Result) == ' ' && Result.c < LineLength(View, Result.l));
        
        return Result;
    }
    
    b32 StartedAtSpecial = false;
    if(!IsNonSpecial(CharAt(View, Result)))
    {
        StartedAtSpecial = true;
    }
    
    if(StartedAtSpecial)
    {
        while(!IsNonSpecial(CharAt(View, Result))
              && Result.c < LineLength(View, Result.l))
            Result.c++;
        return Result;
    }
    
    char c = CharAt(View, Result);
    while(( c != ' ' && (IsNonSpecial(c)) )
          && Result.c < LineLength(View, Result.l))
    {
        Result.c++;
        c = CharAt(View, Result);
    }
    
    return Result;
}

b32
AtLineBeginning(view *View, buffer_pos Pos)
{
    return Pos.c == 0;
}
b32
AtLineEnd(view *View, buffer_pos Pos)
{
    return Pos.c == LineLength(View, Pos.l);
}

buffer_pos
SeekLineBeginning(view *View, buffer_pos From)
{
    return BufferPos(From.l, 0);
}
buffer_pos
SeekLineEnd(view *View, buffer_pos From)
{
    return BufferPos(From.l, LineLength(View, From.l));
}

buffer_pos
SeekPrevEmptyLine(view *View, buffer_pos From)
{
    int ResultLine = From.l;
    
    while(LineLength(View, ResultLine) == 0 && ResultLine > 0)
    {
        ResultLine--;
        if(LineLength(View, ResultLine) != 0)
            break;
    }
    
    while(ResultLine > 0)
    {
        ResultLine--;
        if(LineLength(View, ResultLine) == 0)
            break;
    }
    return BufferPos(ResultLine, 0);
}
buffer_pos
SeekNextEmptyLine(view *View, buffer_pos From)
{
    int ResultLine = From.l;
    
    while(LineLength(View, ResultLine) == 0 && ResultLine < LineCount(View) - 1)
    {
        ResultLine++;
    }
    
    while(ResultLine < LineCount(View) - 1)
    {
        ResultLine++;
        if(LineLength(View, ResultLine) == 0)
            break;
    }
    return BufferPos(ResultLine, LineLength(View, ResultLine));
}

void
SetCursorPos(program_state *ProgramState, view *View, buffer_pos Pos)
{
    ProgramState->UserMovedCursor = true;
    View->CursorPos = Pos;
    Clamp(View->CursorPos.l, 0, LineCount(View));
    Clamp(View->CursorPos.c, 0, LineLength(View, View->CursorPos.l));
}

void
HandleInput_Nav(program_state *ProgramState)
{
    if(KeyShouldExecute(ProgramState->FKey) && !IsAnyShiftKeyDown && !IsAnyControlKeyDown)
    {
        ProgramState->InputMode = InputMode_Insert;
        return;
    }
    
    if(KeyShouldExecute(ProgramState->Slash_Key) && IsAnyShiftKeyDown)
    {
        ProgramState->ShowSuperDebugMenu = true;
    }
    
    if(KeyShouldExecute(ProgramState->NKey))
    {
        if(IsAnyShiftKeyDown)
        {
            ListAdd(&ProgramState->Views, View(ProgramState, &ProgramState->Buffers[0], ProgramState->Views.Data[ProgramState->SelectedViewIndex].Id, Location_Below));
            printf("splitting view vertically\n");
        }
        else
        {
            ListAdd(&ProgramState->Views, View(ProgramState, &ProgramState->Buffers[0], ProgramState->Views.Data[ProgramState->SelectedViewIndex].Id, Location_Right));
            printf("splitting view horizontally\n");
        }
    }
    
    view *View = &ProgramState->Views[ProgramState->SelectedViewIndex];
    
    if(KeyShouldExecute(ProgramState->SKey))
    {
        View->Selecting = !View->Selecting;
        View->SelectionStart = View->CursorPos;
    }
    
    if(KeyShouldExecute(ProgramState->XKey))
    {
        buffer_pos Start = BufferPos(View->CursorPos.l, 0);
        buffer_pos End = BufferPos(View->CursorPos.l+1, 0);
        action Action = ActionForDeleteRange(Start, End);
        ListAdd(&View->Buffer->ActionStack, Action);
        
        RedoAction(View->Buffer, Action);
        //ListRemoveAt(&View->Buffer->Lines, View->CursorPos.l);
    }
    
    if(KeyShouldExecute(ProgramState->EKey)) {
        OpenEditFileLister(ProgramState, View, "./");
    }
    else if(KeyShouldExecute(ProgramState->GKey)) {
        OpenSwitchBufferLister(ProgramState, View);
    }
    else if(KeyShouldExecute(ProgramState->FKey) && IsAnyShiftKeyDown) {
        OpenSwitchFontTypeLister(ProgramState, View);
    }
    else if(KeyShouldExecute(ProgramState->AKey)) {
        OpenCommandLister(ProgramState, View);
    }
    
    if(KeyShouldExecute(ProgramState->UpKey))
    {
        MoveCursorPos(ProgramState, View, BufferPos(-1, 0));
    }
    if(KeyShouldExecute(ProgramState->DownKey))
    {
        MoveCursorPos(ProgramState, View, BufferPos(1, 0));
    }
    if(KeyShouldExecute(ProgramState->LeftKey))
    {
        ProgramState->ShouldChangeIdealCursorCol = true;
        MoveCursorPos(ProgramState, View, BufferPos(0, -1));
    }
    if(KeyShouldExecute(ProgramState->RightKey))
    {
        ProgramState->ShouldChangeIdealCursorCol = true;
        MoveCursorPos(ProgramState, View, BufferPos(0, 1));
    }
    
    if(KeyShouldExecute(ProgramState->IKey))
    {
        MoveCursorPos(ProgramState, View, BufferPos(-1, 0));
    }
    if(KeyShouldExecute(ProgramState->KKey))
    {
        MoveCursorPos(ProgramState, View, BufferPos(1, 0));
    }
    if(KeyShouldExecute(ProgramState->JKey))
    {
        ProgramState->ShouldChangeIdealCursorCol = true;
        MoveCursorPos(ProgramState, View, BufferPos(0, -1));
    }
    if(KeyShouldExecute(ProgramState->LKey))
    {
        ProgramState->ShouldChangeIdealCursorCol = true;
        MoveCursorPos(ProgramState, View, BufferPos(0, 1));
    }
    
    if(KeyShouldExecute(ProgramState->UKey))
    {
        ProgramState->ShouldChangeIdealCursorCol = true;
        SetCursorPos(ProgramState, View, SeekBackBorder(View, View->CursorPos));
    }
    if(KeyShouldExecute(ProgramState->OKey))
    {
        ProgramState->ShouldChangeIdealCursorCol = true;
        SetCursorPos(ProgramState, View, SeekForwardBorder(View, View->CursorPos));
    }
    
    if(KeyShouldExecute(ProgramState->HKey))
    {
        ProgramState->ShouldChangeIdealCursorCol = true;
        if(!AtLineBeginning(View, View->CursorPos))
            SetCursorPos(ProgramState, View, SeekLineBeginning(View, View->CursorPos));
        else
            SetCursorPos(ProgramState, View, SeekPrevEmptyLine(View, View->CursorPos));
    }
    if(KeyShouldExecute(ProgramState->Semicolon_Key))
    {
        ProgramState->ShouldChangeIdealCursorCol = true;
        if(!AtLineEnd(View, View->CursorPos))
            SetCursorPos(ProgramState, View, SeekLineEnd(View, View->CursorPos));
        else
            SetCursorPos(ProgramState, View, SeekNextEmptyLine(View, View->CursorPos));
    }
    
    if(KeyShouldExecute(ProgramState->WKey))
    {// TODO: write current buffer to file
    }
    
    if(KeyShouldExecute(ProgramState->QKey))
    {
        if(IsAnyControlKeyDown)
        {
            if(IsAnyShiftKeyDown)
            {// Exit program
                ProgramState->ShouldExit = true;
            }
            else
            {// Close current view
                RemoveView(ProgramState, ProgramState->SelectedViewIndex);
            }
        }
        else
        {// TODO: Close current buffer
        }
    }
    
}

void
HandleInput_SuperDebugMenu(program_state *ProgramState)
{
    if(KeyShouldExecute(ProgramState->Slash_Key) && IsAnyShiftKeyDown)
    {
        ProgramState->ShowSuperDebugMenu = false;
    }
}


void
HandleInput_Insert(program_state *ProgramState)
{
    buffer *Buffer = ProgramState->Views[ProgramState->SelectedViewIndex].Buffer;
    view *View = &ProgramState->Views[ProgramState->SelectedViewIndex];
    
    if(KeyShouldExecute(ProgramState->Escape_Key))
    {
        ProgramState->InputMode = InputMode_Nav;
    }
    
    if(KeyShouldExecute(ProgramState->UpKey))
    {
        MoveCursorPos(ProgramState, View, BufferPos(-1, 0));
    }
    if(KeyShouldExecute(ProgramState->DownKey))
    {
        MoveCursorPos(ProgramState, View, BufferPos(1, 0));
    }
    if(KeyShouldExecute(ProgramState->LeftKey))
    {
        ProgramState->ShouldChangeIdealCursorCol = true;
        MoveCursorPos(ProgramState, View, BufferPos(0, -1));
    }
    if(KeyShouldExecute(ProgramState->RightKey))
    {
        ProgramState->ShouldChangeIdealCursorCol = true;
        MoveCursorPos(ProgramState, View, BufferPos(0, 1));
    }
    
    for(int i = 0; i < 26; i++)
    {
        if(KeyShouldExecute(ProgramState->LetterKeys[i]))
        {
            ProgramState->ShouldChangeIdealCursorCol = true;
            char CharToAdd;
            if(IsAnyShiftKeyDown)
                CharToAdd = 'A' + i;
            else
                CharToAdd = 'a' + i;
            
            Buffer->Lines[View->CursorPos.l].InsertChar(View->CursorPos.c, CharToAdd);
            View->CursorPos.c++;
            View->IdealCursorCol = ColAt(ProgramState, View, View->CursorPos);
        }
    }
    for(int i = 0; i < 10; i++)
    {
        if(KeyShouldExecute(ProgramState->NumberKeys[i]))
        {
            ProgramState->ShouldChangeIdealCursorCol = true;
            char CharToAdd;
            if(IsAnyShiftKeyDown)
            {
                if(i == 1)
                    CharToAdd = '!';
                else if(i == 2)
                    CharToAdd = '@';
                else if(i == 3)
                    CharToAdd = '#';
                else if(i == 4)
                    CharToAdd = '$';
                else if(i == 5)
                    CharToAdd = '%';
                else if(i == 6)
                    CharToAdd = '^';
                else if(i == 7)
                    CharToAdd = '&';
                else if(i == 8)
                    CharToAdd = '*';
                else if(i == 9)
                    CharToAdd = '(';
                else
                    CharToAdd = ')';
                
            }
            else
            {
                CharToAdd = '0' + i;
            }
            
            Buffer->Lines[View->CursorPos.l].InsertChar(View->CursorPos.c, CharToAdd);
            View->CursorPos.c++;
            View->IdealCursorCol = ColAt(ProgramState, View, View->CursorPos);
        }
    }
    for(int i = 0; i < 11; i++)
    {
        if(KeyShouldExecute(ProgramState->SymbolKeys[i]))
        {
            ProgramState->ShouldChangeIdealCursorCol = true;
            
            char CharToAdd = ' ';
            if(!IsAnyShiftKeyDown)
            {
                if(i == 0)
                    CharToAdd = '`';
                else if(i == 1)
                    CharToAdd = '-';
                else if(i == 2)
                    CharToAdd = '=';
                else if(i == 3)
                    CharToAdd = '[';
                else if(i == 4)
                    CharToAdd = ']';
                else if(i == 5)
                    CharToAdd = '\\';
                else if(i == 6)
                    CharToAdd = ';';
                else if(i == 7)
                    CharToAdd = '\'';
                else if(i == 8)
                    CharToAdd = '/';
                else if(i == 9)
                    CharToAdd = ',';
                else if(i == 10)
                    CharToAdd = '.';
            }
            else
            {
                if(i == 0)
                    CharToAdd = '~';
                else if(i == 1)
                    CharToAdd = '_';
                else if(i == 2)
                    CharToAdd = '+';
                else if(i == 3)
                    CharToAdd = '{';
                else if(i == 4)
                    CharToAdd = '}';
                else if(i == 5)
                    CharToAdd = '|';
                else if(i == 6)
                    CharToAdd = ':';
                else if(i == 7)
                    CharToAdd = '"';
                else if(i == 8)
                    CharToAdd = '?';
                else if(i == 9)
                    CharToAdd = '<';
                else if(i == 10)
                    CharToAdd = '>';
            }
            
            Buffer->Lines[View->CursorPos.l].InsertChar(View->CursorPos.c, CharToAdd);
            View->CursorPos.c++;
            View->IdealCursorCol = ColAt(ProgramState, View, View->CursorPos);
        }
    }
    
    for(int i = 0; i < 7; i++)
    {
        if(KeyShouldExecute(ProgramState->SpecialKeys[i]))
        {
            
            char CharToAdd = ' ';
            if(i == 0)
            {// Space
                Buffer->Lines[View->CursorPos.l].InsertChar(View->CursorPos.c, ' ');
                View->CursorPos.c++;
                View->IdealCursorCol = ColAt(ProgramState, View, View->CursorPos);
                ProgramState->ShouldChangeIdealCursorCol = true;
            }
            else if(i == 1)
            { // Backspace
                if(View->CursorPos.c > 0)
                {
                    Buffer->Lines[View->CursorPos.l].RemoveChar(View->CursorPos.c-1);
                    View->CursorPos.c--;
                    Clamp(View->CursorPos.c, 0, LineLength(View, View->CursorPos.l));
                }
                ProgramState->ShouldChangeIdealCursorCol = true;
            }
            else if(i == 2)
            { // Delete
                Buffer->Lines[View->CursorPos.l].RemoveChar(View->CursorPos.c);
                ProgramState->ShouldChangeIdealCursorCol = true;
            }
            else if(i == 3)
            { // Tab
                // TODO: handling tab char
                for(int a = 0; a < 4; a++)
                {
                    Buffer->Lines[View->CursorPos.l].InsertChar(View->CursorPos.c, ' ');
                    View->CursorPos.c++;
                    //View->IdealCursorCol = ColAt(ProgramState, View, View->CursorPos);
                    ProgramState->ShouldChangeIdealCursorCol = true;
                }
            }
            else if(i == 4)
            { // Return
                InsertLine(Buffer, View->CursorPos.l+1, CopyString(Buffer->Lines[View->CursorPos.l]));
                if(Buffer->Lines[View->CursorPos.l].Length != 0)
                {
                    Buffer->Lines[View->CursorPos.l].Slice(0, View->CursorPos.c);
                    Buffer->Lines[View->CursorPos.l+1].Slice(View->CursorPos.c, Buffer->Lines[View->CursorPos.l+1].Length);
                }
                
                View->CursorPos.l++;
                View->CursorPos.c = 0;
                
                ProgramState->UserMovedCursor = true;
                ProgramState->ShouldChangeIdealCursorCol = true;
            }
            else if(i == 5)
            { // Caps lock
            }
            else if(i == 6)
            { // Escape
            }
            
        }
    }
}

void
HandleInput_EntryBar(program_state *ProgramState)
{
    view *View = &ProgramState->Views[ProgramState->SelectedViewIndex];
    lister *Lister = &View->Lister;
    string *EntryString = &View->Lister.Input;
    
    if(KeyShouldExecute(ProgramState->Escape_Key))
    {
        CloseLister(ProgramState, View);
        return;
    }
    
    if(KeyShouldExecute(ProgramState->Return_Key) ||
       KeyShouldExecute(ProgramState->Tab_Key))
    {
        Lister->ShouldExecute = true;
    }
    if(KeyShouldExecute(ProgramState->UpKey))
    {
        if(Lister->SelectedIndex > 0)
            Lister->SelectedIndex--;
    }
    if(KeyShouldExecute(ProgramState->DownKey))
    {
        if(Lister->SelectedIndex < Lister->MatchingEntries.Count - 1)
            Lister->SelectedIndex++;
    }
    
    if(KeyShouldExecute(ProgramState->Backspace_Key))
    {
        if(Lister->Input.Length > 0)
            Lister->Input.Length--;
    }
    
    for(int i = 0; i < 26; i++)
    {
        if(KeyShouldExecute(ProgramState->LetterKeys[i]))
        {
            char CharToAdd;
            if(IsAnyShiftKeyDown)
                CharToAdd = 'A' + i;
            else
                CharToAdd = 'a' + i;
            
            EntryString->InsertChar(EntryString->Length, CharToAdd);
        }
    }
    for(int i = 0; i < 10; i++)
    {
        if(KeyShouldExecute(ProgramState->NumberKeys[i]))
        {
            char CharToAdd;
            if(IsAnyShiftKeyDown)
            {
                if(i == 1)
                    CharToAdd = '!';
                else if(i == 2)
                    CharToAdd = '@';
                else if(i == 3)
                    CharToAdd = '#';
                else if(i == 4)
                    CharToAdd = '$';
                else if(i == 5)
                    CharToAdd = '%';
                else if(i == 6)
                    CharToAdd = '^';
                else if(i == 7)
                    CharToAdd = '&';
                else if(i == 8)
                    CharToAdd = '*';
                else if(i == 9)
                    CharToAdd = '(';
                else
                    CharToAdd = ')';
                
            }
            else
            {
                CharToAdd = '0' + i;
            }
            
            EntryString->InsertChar(EntryString->Length, CharToAdd);
        }
    }
    for(int i = 0; i < 11; i++)
    {
        if(KeyShouldExecute(ProgramState->SymbolKeys[i]))
        {
            
            char CharToAdd = ' ';
            if(!IsAnyShiftKeyDown)
            {
                if(i == 0)
                    CharToAdd = '`';
                else if(i == 1)
                    CharToAdd = '-';
                else if(i == 2)
                    CharToAdd = '=';
                else if(i == 3)
                    CharToAdd = '[';
                else if(i == 4)
                    CharToAdd = ']';
                else if(i == 5)
                    CharToAdd = '\\';
                else if(i == 6)
                    CharToAdd = ';';
                else if(i == 7)
                    CharToAdd = '\'';
                else if(i == 8)
                    CharToAdd = '/';
                else if(i == 9)
                    CharToAdd = ',';
                else if(i == 10)
                    CharToAdd = '.';
            }
            else
            {
                if(i == 0)
                    CharToAdd = '~';
                else if(i == 1)
                    CharToAdd = '_';
                else if(i == 2)
                    CharToAdd = '+';
                else if(i == 3)
                    CharToAdd = '{';
                else if(i == 4)
                    CharToAdd = '}';
                else if(i == 5)
                    CharToAdd = '|';
                else if(i == 6)
                    CharToAdd = ':';
                else if(i == 7)
                    CharToAdd = '"';
                else if(i == 8)
                    CharToAdd = '?';
                else if(i == 9)
                    CharToAdd = '<';
                else if(i == 10)
                    CharToAdd = '>';
            }
            
            EntryString->InsertChar(EntryString->Length, CharToAdd);
        }
    }
    
}

void
HandleInput(program_state *ProgramState)
{
    ProgramState->UserMovedCursor = false;
    if(ProgramState->ShowSuperDebugMenu)
    {
        HandleInput_SuperDebugMenu(ProgramState);
    }
    else if(ProgramState->Views[ProgramState->SelectedViewIndex].ListerIsOpen)
    {
        HandleInput_EntryBar(ProgramState);
    }
    else if(ProgramState->InputMode == InputMode_Nav)
    {
        HandleInput_Nav(ProgramState);
    }
    else if(ProgramState->InputMode == InputMode_Insert)
    {
        HandleInput_Insert(ProgramState);
    }
    
    view *View = &ProgramState->Views[ProgramState->SelectedViewIndex];
    
    int ScrollAmount = ProgramState->FontSize * 3;
    if(GetMouseWheelMoveV().y < 0)
        View->TargetY += ScrollAmount;
    else if(GetMouseWheelMoveV().y > 0)
        View->TargetY -= ScrollAmount;
    
    if(ProgramState->ShouldChangeIdealCursorCol)
    {
        ProgramState->ShouldChangeIdealCursorCol = false;
        View->IdealCursorCol = View->CursorPos.c;
    }
    else
    {
        View->CursorPos.c = View->IdealCursorCol;
        Clamp(View->CursorPos.c, 0, LineLength(View, View->CursorPos.l));
    }
    
    
    if(IsAnyControlKeyDown)
    {
        ProgramState->FontSize += GetMouseWheelMove();
        if(ProgramState->FontSize < 6) ProgramState->FontSize = 6;
        if(ProgramState->FontSize > 100) ProgramState->FontSize = 100;
    }
}



#endif //FESTIVAL_EDITING_H
