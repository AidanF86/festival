color
RGBA(int r, int g, int b, int a) {
    color Result;
    Result.r = (u8)r;
    Result.g = (u8)g;
    Result.b = (u8)b;
    Result.a = (u8)a;
    return Result;
}

color
RGB(int r, int g, int b) {
    return RGBA(r, g, b, 255);
}

v2 GetCharDim(program_state *ProgramState, int Size)
{
    return V2(MeasureTextEx(ProgramState->FontMain, "_", Size, 0));
}
v2 GetCharDim(program_state *ProgramState)
{
    return GetCharDim(ProgramState, ProgramState->FontSize);
}

line_data
LineData() {
    line_data Result = {0};
    Result.CharRects = RectList();
    return Result;
};

int 
LineLength(buffer *Buffer, int l)
{
    if(l < 0 || l >= Buffer->Lines.Count)
        return 0;
    return Buffer->Lines.Data[l].Length;
}
int 
LineLength(view *View, int l)
{
    return LineLength(View->Buffer, l);
}
int
LineCount(buffer *Buffer)
{
    return Buffer->Lines.Count;
}
int
LineCount(view *View)
{
    if(!View->Buffer)
        return 0;
    return View->Buffer->Lines.Count;
}

rect
CharRectAt(view *View, int l, int c)
{
    if(c == LineLength(View, l))
        return View->LineDataList[l].EndLineRect;
    return View->LineDataList[l].CharRects[c];
}

rect CharRectAt(view *View, buffer_pos Pos) { return CharRectAt(View, Pos.l, Pos.c); }

rect
LineRect(view *View, int l)
{
    if(l < 0 || l >= LineCount(View))
        return {0};
    return View->LineDataList.Data[l].LineRect;
}

v2
CharToScreenSpace(view *View, v2 CharRect)
{
    v2 Result = CharRect;
    Result.x += View->TextRect.x;
    Result.y += View->TextRect.y;
    Result.y -= View->Y;
    return Result;
}
v2
ScreenToCharSpace(view *View, v2 ScreenRect)
{
    v2 Result = ScreenRect;
    Result.y += View->Y;
    Result.y -= View->TextRect.y;
    Result.x -= View->TextRect.x;
    return Result;
}
rect
CharToScreenSpace(view *View, rect CharRect)
{
    rect Result = CharRect;
    v2 V = CharToScreenSpace(View, V2(CharRect));
    Result.x = V.x;
    Result.y = V.y;
    return Result;
}
rect
ScreenToCharSpace(view *View, rect ScreenRect)
{
    rect Result = ScreenRect;
    v2 V = ScreenToCharSpace(View, V2(ScreenRect));
    Result.x = V.x;
    Result.y = V.y;
    return Result;
}

rect
ScreenRectAt(view *View, int l, int c)
{
    return CharToScreenSpace(View, CharRectAt(View, l, c));
}

rect
ScreenRectAt(view *View, buffer_pos Pos)
{
    return ScreenRectAt(View, Pos.l, Pos.c);
}

char
CharAt(buffer *Buffer, int l, int c)
{
    if(l < 0 || l >= Buffer->Lines.Count ||
       c < 0 || c >= LineLength(Buffer, l))
        return 0;
    return Buffer->Lines[l].Data[c];
}
char
CharAt(view *View, int l, int c)
{
    if(!View->Buffer)
        return 0;
    return CharAt(View->Buffer, l, c);
}
char
CharAt(buffer *Buffer, buffer_pos Pos)
{
    return CharAt(Buffer, Pos.l, Pos.c);
}
char
CharAt(view *View, buffer_pos Pos)
{
    return CharAt(View, Pos.l, Pos.c);
}

void
InsertLine(buffer *Buffer, int l, string S)
{
    ListInsert(&Buffer->Lines, l, S);
}

line_data
LineDataAt(view *View, int l)
{
    if(l < 0 || l >= LineCount(View))
    {
        printf("GetLineData: Out of bounds!\n");
        return {0};
    }
    return View->LineDataList[l];
}

int
YToLine(view *View, int Y)
{
    int l;
    int PrevLineY = LineDataAt(View, 0).LineRect.y;
    for(l = 0; l < LineCount(View); l++)
    {
        line_data LineData = LineDataAt(View, l);
        int LineY = LineData.LineRect.y;
        
        if(LineY > Y)
        {
            if(l > 0)
                l--;
            break;
        }
    }
    
    if(l >= LineCount(View))
        return LineCount(View)-1;
    
    return l;
}

int
ColAt(program_state *ProgramState, view *View, buffer_pos P)
{
    int Col = 0;
    int PrevY = CharRectAt(View, BufferPos(P.l, 0)).y;
    
    for(int c = 1; c < LineLength(View, P.l) && c <= P.c; c++)
    {
        Col++;
        if(CharRectAt(View, BufferPos(P.l, c)).y > PrevY)
        {
            Col = ProgramState->SubLineOffset;
        }
        PrevY = CharRectAt(View, BufferPos(P.l, c)).y;
    }
    
    return Col;
}


buffer_pos
ClosestBufferPos(view *View, v2 P)
{ // P is in char space
    int l = YToLine(View, P.y);
    
    buffer_pos ClosestBufferP = BufferPos(l, 0);
    rect ClosestRect = CharRectAt(View, ClosestBufferP);
    v2 ClosestP = V2(ClosestRect.x+ClosestRect.w/2, ClosestRect.y+ClosestRect.h/2);
    
    for(int c = 0; c <= View->LineDataList[l].CharRects.Count; c++)
    {
        rect TestRect = CharRectAt(View, l, c);
        v2 TestP = V2(TestRect.x+TestRect.w/2, TestRect.y+TestRect.h/2);
        
        v2 Diff = TestP - P;
        v2 CompareDiff = ClosestP - P;
        if(abs(Diff.y) < abs(CompareDiff.y) ||
           ( !(abs(Diff.y) > abs(CompareDiff.y)) && abs(Diff.x) < abs(CompareDiff.x) )
           )
        {
            ClosestP = TestP;
            ClosestBufferP = BufferPos(l, c);
        }
    }
    
    return ClosestBufferP;
}

buffer_pos
ClosestBufferPos(view *View, rect Rect)
{
    return ClosestBufferPos(View, V2(Rect.x+Rect.w/2, Rect.y+Rect.h/2));
}

void
AdjustView(program_state *ProgramState, view *View)
{
    v2 CharDim = GetCharDim(ProgramState);
    int CharWidth = CharDim.x;
    int CharHeight = CharDim.y;
    buffer_pos CursorPos = View->CursorPos;
    int Y = View->Y;
    int TargetY = View->TargetY;
    
    rect CursorRect = CharRectAt(View, CursorPos.l, CursorPos.c);
    b32 MovedCursorUpOrDown = false;
    
    if(ProgramState->UserMovedCursor)
    { // Adjust based on cursor
        if(CursorRect.y < Y)
        {
            TargetY = CursorRect.y;
        }
        else if(CursorRect.y > Y + View->TextRect.h - CharHeight)
        {
            TargetY = CursorRect.y - View->TextRect.h + CharHeight;
        }
    }
    else
    { // Adjust based on view
        if(CursorRect.y < TargetY)
        {
            // adjust cursor pos to new rect?
            View->CursorPos.l = YToLine(View, TargetY) + 3;
            MovedCursorUpOrDown = true;
        }
        else if(CursorRect.y > TargetY + View->TextRect.h - CharHeight)
        {
            View->CursorPos.l = YToLine(View, 
                                        TargetY + View->TextRect.h) - 4;
            MovedCursorUpOrDown = true;
        }
    }
    
    View->TargetY = TargetY;
    
    Clamp(View->TargetY, 0, LineDataAt(View, LineCount(View)-1).EndLineRect.y);
    Clamp(View->CursorPos.l, 0, LineCount(View)-1);
    Clamp(View->CursorPos.c, 0, LineLength(View, View->CursorPos.l));
    
    
    if(MovedCursorUpOrDown && ColAt(ProgramState, View, View->CursorPos) < View->IdealCursorCol)
    {
        int Diff = View->IdealCursorCol - ColAt(ProgramState, View, View->CursorPos);
        int DistToEnd = LineLength(View, View->CursorPos.l) - View->CursorPos.c;
        if(Diff > DistToEnd)
            Diff = DistToEnd;
        View->CursorPos.c += Diff;
    }
}

void
FillLineData(view *View, program_state *ProgramState)
{
    line_data_list *DataList = &View->LineDataList;
    
    int MarginLeft = ProgramState->MarginLeft;
    int CharsPerVirtualLine = ProgramState->CharsPerVirtualLine;
    int NumbersWidth = ProgramState->NumbersWidth;
    int SubLineOffset = ProgramState->SubLineOffset;
    v2 CharDim = GetCharDim(ProgramState);
    int CharWidth = CharDim.x;
    int CharHeight = CharDim.y;
    // TODO(cheryl): formalize char-exclusion-zone
    int WrapPoint = View->TextRect.w - CharWidth;
    
    // DeAllocation
    if(DataList->IsAllocated);
    {
        // Deallocate all lists
        for(int i = 0; i < DataList->Count; i++)
        {
            if(DataList->Data[i].CharRects.IsAllocated)
            {
                ListFree(&(DataList->Data[i].CharRects));
            }
            else
            {
                Print("Unallocated rect list???\n");
            }
            
        }
        ListFree(DataList);
    }
    // Allocation
    *DataList = LineDataList();
    
    
    int y = 0;
    
    for(int l = 0; l < LineCount(View); l++)
    {
        ListAdd(DataList, LineData());
        
        line_data *RectData = &(DataList->Data[l]);
        int x = 0;
        
        RectData->LineRect.x = x;
        RectData->LineRect.y = y;
        RectData->LineRect.w = View->TextRect.w;
        RectData->DisplayLines = 1;
        
        for(int c = 0; c < LineLength(View, l); c++)
        {
            // Rect is within the space of textrect
            // so when drawing, offset by textrect.x and textrect.y
            // as well as buffer viewpos
            if(x+CharWidth >= WrapPoint)
            {
                x = SubLineOffset*CharWidth;
                y += CharHeight;
                RectData->DisplayLines++;
            }
            
            // TODO: optimize!!
            GlyphInfo Info = {0};
            int GlyphIndex = 0;
            for(int i = 0; i < ProgramState->FontMain.glyphCount; i++)
            {
                if(ProgramState->FontMain.glyphs[i].value == CharAt(View, l, c))
                {
                    Info = ProgramState->FontMain.glyphs[i];
                    GlyphIndex = i;
                    break;
                }
            }
            
            ListAdd(&(RectData->CharRects), Rect(x, y, CharWidth, CharHeight));
            
            x += /*CharWidth + */Info.advanceX;
        }
        RectData->EndLineRect = Rect(x, y, CharWidth, CharHeight);
        
        y += CharHeight;
        
        RectData->LineRect.h = RectData->DisplayLines * CharHeight;
    }
}

void
FillKeyData(program_state *ProgramState)
{
    for(int i = 0; i < sizeof(ProgramState->KeyData) / sizeof(key_data); i++)
    {
        ProgramState->KeyData[i] = {0};
    }
    ProgramState->LeftKey.KeyCode = KEY_LEFT;
    ProgramState->RightKey.KeyCode = KEY_RIGHT;
    ProgramState->UpKey.KeyCode = KEY_UP;
    ProgramState->DownKey.KeyCode = KEY_DOWN;
    ProgramState->PageUp_Key.KeyCode = KEY_PAGE_UP;
    ProgramState->PageDown_Key.KeyCode = KEY_PAGE_DOWN;
    //ProgramState->Key.KeyCode = KEY_;
    ProgramState->AKey.KeyCode = KEY_A;
    ProgramState->BKey.KeyCode = KEY_B;
    ProgramState->CKey.KeyCode = KEY_C;
    ProgramState->DKey.KeyCode = KEY_D;
    ProgramState->EKey.KeyCode = KEY_E;
    ProgramState->FKey.KeyCode = KEY_F;
    ProgramState->GKey.KeyCode = KEY_G;
    ProgramState->HKey.KeyCode = KEY_H;
    ProgramState->IKey.KeyCode = KEY_I;
    ProgramState->JKey.KeyCode = KEY_J;
    ProgramState->KKey.KeyCode = KEY_K;
    ProgramState->LKey.KeyCode = KEY_L;
    ProgramState->MKey.KeyCode = KEY_M;
    ProgramState->NKey.KeyCode = KEY_N;
    ProgramState->OKey.KeyCode = KEY_O;
    ProgramState->PKey.KeyCode = KEY_P;
    ProgramState->QKey.KeyCode = KEY_Q;
    ProgramState->RKey.KeyCode = KEY_R;
    ProgramState->SKey.KeyCode = KEY_S;
    ProgramState->TKey.KeyCode = KEY_T;
    ProgramState->UKey.KeyCode = KEY_U;
    ProgramState->VKey.KeyCode = KEY_V;
    ProgramState->WKey.KeyCode = KEY_W;
    ProgramState->XKey.KeyCode = KEY_X;
    ProgramState->YKey.KeyCode = KEY_Y;
    ProgramState->ZKey.KeyCode = KEY_Z;
    //ProgramState->Key.KeyCode = KEY_;
    ProgramState->Number0Key.KeyCode = KEY_ZERO;
    ProgramState->Number1Key.KeyCode = KEY_ONE;
    ProgramState->Number2Key.KeyCode = KEY_TWO;
    ProgramState->Number3Key.KeyCode = KEY_THREE;
    ProgramState->Number4Key.KeyCode = KEY_FOUR;
    ProgramState->Number5Key.KeyCode = KEY_FIVE;
    ProgramState->Number6Key.KeyCode = KEY_SIX;
    ProgramState->Number7Key.KeyCode = KEY_SEVEN;
    ProgramState->Number8Key.KeyCode = KEY_EIGHT;
    ProgramState->Number9Key.KeyCode = KEY_NINE;
    //ProgramState->Key.KeyCode = KEY_;
    ProgramState->Grave_Key.KeyCode = KEY_GRAVE;
    ProgramState->Minus_Key.KeyCode = KEY_MINUS;
    ProgramState->Equals_Key.KeyCode = KEY_EQUAL;
    ProgramState->LeftBracket_Key.KeyCode = KEY_LEFT_BRACKET;
    ProgramState->RightBracket_Key.KeyCode = KEY_RIGHT_BRACKET;
    ProgramState->Backslash_Key.KeyCode = KEY_BACKSLASH;
    ProgramState->Semicolon_Key.KeyCode = KEY_SEMICOLON;
    ProgramState->Quote_Key.KeyCode = KEY_APOSTROPHE;
    ProgramState->Slash_Key.KeyCode = KEY_SLASH;
    ProgramState->Comma_Key.KeyCode = KEY_COMMA;
    ProgramState->Period_Key.KeyCode = KEY_PERIOD;
    //ProgramState->Key.KeyCode = KEY_;
    ProgramState->Space_Key.KeyCode = KEY_SPACE;
    ProgramState->Backspace_Key.KeyCode = KEY_BACKSPACE;
    ProgramState->Delete_Key.KeyCode = KEY_DELETE;
    ProgramState->Tab_Key.KeyCode = KEY_TAB;
    ProgramState->Return_Key.KeyCode = KEY_ENTER;
    ProgramState->CapsLock_Key.KeyCode = KEY_CAPS_LOCK;
    ProgramState->Escape_Key.KeyCode = KEY_ESCAPE;
    //ProgramState->Key.KeyCode = KEY_;
}

void
UpdateKeyInput(program_state *ProgramState)
{
    for(int i = 0; i < sizeof(ProgramState->KeyData) / sizeof(key_data); i++)
    {
        key_data *Key = &ProgramState->KeyData[i];
        
        if(IsKeyDown(Key->KeyCode))
        {
            if(Key->PressTime == 0)
                Key->JustPressed = true;
            else
                Key->JustPressed = false;
            
            Key->PressTime += GetFrameTime();
        }
        else
        {
            Key->PressTime = 0;
            Key->TimeTillNextRepeat = 0;
        }
        
        if(Key->JustPressed || Key->PressTime >= ProgramState->KeyFirstRepeatTime)
        {
            if(Key->TimeTillNextRepeat <= 0)
            {
                Key->TimeTillNextRepeat = ProgramState->KeyRepeatSpeed;
            }
            else
            {
                Key->TimeTillNextRepeat -= GetFrameTime();
            }
        }
    }
}

void
LoadFont(program_state *ProgramState, int Size)
{
    // TODO: unload font
    //UnloadFontData(GlyphInfo *glyphs, int glyphCount);
    //UnloadFont(Font font);
    
    u32 FontFileSize = 0;
    u8 *FontFileData = LoadFileData("LiberationMono-Regular.ttf", &FontFileSize);
    //u8 *FontFileData = LoadFileData("Georgia.ttf", &FontFileSize);
    //u8 *FontFileData = LoadFileData("HelveticaNeue-Regular.otf", &FontFileSize);
    
    Font *FontMain = &ProgramState->FontMain;
    *FontMain = {0};
    FontMain->baseSize = Size;
    FontMain->glyphCount = 95;
    FontMain->glyphs = LoadFontData(FontFileData, FontFileSize, FontMain->baseSize, 0, 95, FONT_DEFAULT);
    Image Atlas = GenImageFontAtlas(FontMain->glyphs, &FontMain->recs, 95, FontMain->baseSize, 4, 0);
    FontMain->texture = LoadTextureFromImage(Atlas);
    UnloadImage(Atlas);
    
    UnloadFileData(FontFileData);
}

view
View(program_state *ProgramState, buffer *Buffer, int ParentId, view_spawn_location SpawnLocation, f32 Area)
{
    view View = {0};
    View.CursorPos.l = 0;
    View.CursorPos.c = 0;
    View.Buffer = Buffer;
    View.ParentId = ParentId;
    View.LineDataList = {0};
    
    if(ParentId == -1)
    {
        // TODO(cheryl): check if there are any views in existence (there shouldn't be)
        View.Id = 0;
        View.Area = 1;
        View.SpawnLocation = Location_Below;
        View.BirthOrdinal = 0;
    }
    else
    {
        // TODO(cheryl): test :3
        
        // Get a unique Id
        int Id = 0;
        for(Id; Id <= ProgramState->Views.Count; Id++)
        {
            b32 IsIdTaken = false;
            for(int a = 0; a < ProgramState->Views.Count; a++)
            {
                if(ProgramState->Views[a].Id == Id)
                    IsIdTaken = true;
            }
            if(!IsIdTaken)
                break;
        }
        
        View.Id = Id;
        View.SpawnLocation = SpawnLocation;
        View.Area = 0.5f; // Default
        
        int SiblingCount = 0;
        for(int i = 0; i < ProgramState->Views.Count; i++)
        {
            if(ProgramState->Views[i].ParentId == ParentId)
                SiblingCount++;
        }
        
        View.BirthOrdinal = SiblingCount;
    }
    
    return View;
}

view View(program_state *ProgramState, buffer *Buffer, int ParentId, view_spawn_location SpawnLocation)
{
    return View(ProgramState, Buffer, ParentId, SpawnLocation, 0.5f);
}

void
ComputeTextRect(program_state *ProgramState, view *View)
{
    v2 CharDim = GetCharDim(ProgramState);
    int CharWidth = CharDim.x;
    int CharHeight = CharDim.y;
    
    View->TextRect.x = View->Rect.x + ProgramState->NumbersWidth*CharWidth + ProgramState->MarginLeft;
    View->TextRect.y = View->Rect.y + CharHeight;
    View->TextRect.w = View->Rect.w - (View->TextRect.x - View->Rect.x);
    View->TextRect.h = View->Rect.h - CharHeight;
}

void
RemoveView(program_state *ProgramState, int Index)
{
    // TODO: decrement birth ordinal of children?
    
    view_list *Views = &ProgramState->Views;
    if(Views->Count <= 1)
    {
        // TODO: set view to no buffer?
        return;
    }
    
    int RemovedViewId = Views->Data[Index].Id;
    int RemovedViewParentId = Views->Data[Index].ParentId;
    view_spawn_location RemovedViewSpawnLocation = Views->Data[Index].SpawnLocation;
    
    ListRemoveAt(Views, Index);
    
    // find a suitable heir
    view *Heir = NULL;
    int HeirIndex = 0;
    int ChildCount = 0;
    int SmallestBirthOrdinal = 256;
    for(int i = 0; i < Views->Count; i++)
    {
        view *View = &Views->Data[i];
        if(View->ParentId == RemovedViewId)
        {
            ChildCount++;
            if(View->BirthOrdinal < SmallestBirthOrdinal)
            {
                Heir = View;
                HeirIndex = i;
                SmallestBirthOrdinal = View->BirthOrdinal;
            }
        }
    }
    
    if(Heir != NULL && ChildCount > 0)
    {
        Print("Has Heir");
        Heir->Id = RemovedViewId;
        Heir->ParentId = RemovedViewParentId;
        Heir->SpawnLocation = RemovedViewSpawnLocation;
        
        if(ProgramState->SelectedViewIndex == Index || ProgramState->SelectedViewIndex >= Views->Count)
        {
            // set to heir
            ProgramState->SelectedViewIndex = HeirIndex;
        }
    }
    if(ProgramState->SelectedViewIndex >= Views->Count)
    {
        // set to parent
        int ParentIndex = 0;
        for(int i = 0; i < Views->Count; i++)
        {
            view *View = &Views->Data[i];
            if(View->Id == RemovedViewParentId)
                ParentIndex = i;
        }
        ProgramState->SelectedViewIndex = ParentIndex;
    }
}
