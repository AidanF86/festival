/* date = October 5th 2023 3:42 pm */

#ifndef FESTIVAL_LISTS_H
#define FESTIVAL_LISTS_H



rect_list RectList()
{
    rect_list Result;
    Result.Count = 0;
    Result.ArraySize = 20;
    Result.IsAllocated = true;
    Result.Data = (rect *)malloc(20 * sizeof(rect));
    return Result;
}

rect_list RectList(int Size)
{
    rect_list Result;
    Result.Count = 0;
    Result.ArraySize = Size;
    Result.IsAllocated = true;
    Result.Data = (rect *)malloc(Size * sizeof(rect));
    return Result;
}

int ListRemoveAt(rect_list *List, int Index)
{
    if(Index >= 0 && Index < List->Count)
    {
        for(int i = Index; i < List->Count; i++)
        {
            List->Data[i] = List->Data[i+1];
        }
        List->Count--;
        return 1;
    }
    return 0;
}

/*======= line_rect_data List =======*/
line_rect_data_list LineRectDataList()
{
    line_rect_data_list Result;
    Result.Count = 0;
    Result.ArraySize = 20;
    Result.IsAllocated = true;
    Result.Data = (line_rect_data *)malloc(20 * sizeof(line_rect_data));
    return Result;
}

line_rect_data_list LineRectDataList(int Size)
{
    line_rect_data_list Result;
    Result.Count = 0;
    Result.ArraySize = Size;
    Result.IsAllocated = true;
    Result.Data = (line_rect_data *)malloc(Size * sizeof(line_rect_data));
    return Result;
}

int ListRemoveAt(line_rect_data_list *List, int Index)
{
    if(Index >= 0 && Index < List->Count)
    {
        for(int i = Index; i < List->Count; i++)
        {
            List->Data[i] = List->Data[i+1];
        }
        List->Count--;
        return 1;
    }
    return 0;
}



#endif //FESTIVAL_LISTS_H
