/* date = January 1st 2024 7:42 pm */

#ifndef FESTIVAL_UNDO_H
#define FESTIVAL_UNDO_H

struct action
{
    b32 Delete;
    // NOTE: end can be before start, the only difference is that end is where the cursor is. inc, ex
    b32 DeleteSingleLine;
    buffer_pos DeleteStart;
    buffer_pos DeleteEnd;
    string_list DeleteContent;
    
    
    b32 Add;
    buffer_pos AddPos;
    string_list AddContent;
    b32 AddLineBelow;
};

DefineList(action, Action);

#endif //FESTIVAL_UNDO_H
