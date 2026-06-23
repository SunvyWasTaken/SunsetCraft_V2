//
// Created by sunvy on 23/06/2026.
//

#include "Item.h"


bool ItemStack::Empty() const
{
    return id == Item::null && count == 0;
}
